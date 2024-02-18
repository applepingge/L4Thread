#
# Copyright 2017 Hyperkernel Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import collections
import copy
import types

import unittest
import sys
import importlib

import pdb
import z3
import libirpy
import libirpy.util as util
import ir_verify

_tests = [
    "case",
]

### from base.py
_order = 0
Solver = libirpy.solver.Solver
INTERACTIVE = False

class Accessor(object):
    def __init__(self, field, instance, imap):
        self._field = field
        self._instance = instance
        self._imap = imap

    def __repr__(self):
        return "Partial application of {!r}{!r}".format(self._instance._fields[self._field], tuple(self._instance._path))

    def __call__(self, *args):
        instance = self._instance
        for i in args:
            instance = instance[i]
        return self._imap(instance)

    def __getattr__(self, attr):
        val = getattr(self._imap, attr)
        if isinstance(val, types.MethodType):
            return lambda *args, **kwargs: val(self._instance, *args, **kwargs)
        else:
            return val

    def __getitem__(self, arg):
        return Accessor(self._field, self._instance[arg], self._imap)

    def __setitem__(self, arg, value):
        return setattr(self._instance[arg], self._field, value)

    def __iadd__(self, value):
        return self._imap.__iadd__(self._instance, value)

    def __sub__(self, value):
        return self._imap.__isub__(self._instance, value)


class AbstractMap(object):
    def __init__(self, *types):
        global _order
        self._order = _order
        _order += 1

        self._name = None
        self._arity = len(types) - 1
        self._types = types

    def get_arity(self):
        return self._arity

    def get_types(self):
        return self._types

    def invariants(self):
        return []

    def initial(self, instance):
        pass

    def _init(self, obj, name, prefix=None):
        if prefix:
            self._prefix = prefix + '_'
        else:
            self._prefix = ''

        self._name = name
        obj._fields[self._name] = self.new()

    def __get__(self, instance, owner=None):
        f = instance._fields[self._name]

        if len(instance._path) < self.get_arity():
            return Accessor(self._name, instance, self)
        elif self._arity == len(instance._path):
            return self.get(f, instance._path)
        else:
            raise TypeError('Too many arguments to function: expected arguments: {!r}'.format(self._types))

    def __call__(self, instance):
        return self.__get__(instance)

    def __set__(self, instance, value):
        assert len(instance._path) <= self.get_arity()
        instance._fields[self._name] = self.set(instance._fields[self._name], instance._path, value)

    def __iadd__(self, instance, value):
        return self.iadd(instance._fields[self._name], instance._path, value)

    def __isub__(self, instance, value):
        return self.isub(instance._fields[self._name], instance._path, value)

    def new(self):
        raise NotImplementedError()

    def get(self):
        raise NotImplementedError()

    def iadd(self):
        raise NotImplementedError()

    def isub(self):
        raise NotImplementedError()


class StructMeta(type):
    def __new__(cls, name, parents, dct):
        meta = collections.OrderedDict()
        for k, v in sorted(dct.items(), key=lambda v: getattr(v[1], '_order', float('inf'))):
            if hasattr(v, '_init'):
                meta[k] = v
            if getattr(v, '__metaclass__', None) == cls:
                meta[k] = v.__class__
                del dct[k]
        dct['_meta'] = meta
        return super(StructMeta, cls).__new__(cls, name, parents, dct)


class Struct(object):
    __frozen = False
    __metaclass__ = StructMeta

    def __init__(self):
        global _order
        self._order = _order
        _order += 1

    def copy(self):
        return copy.deepcopy(self)

    def _init(self, parent, name):
        self._fields = {}
        self._structs = collections.OrderedDict()
        self._path = []

        for k, v in self._meta.items():
            if isinstance(v, type):
                self._structs[k] = v()
                self._structs[k]._init(self, k)
            else:
                v._init(self, k, prefix=name)

        for k, v in self._structs.items():
            v = copy.copy(v)
            self._structs[k] = v
            v._init(self, k)
        self.__frozen = True

    def __getattribute__(self, arg):
        if arg.startswith('_'):
            return super(Struct, self).__getattribute__(arg)

        if arg not in self._meta:
            return super(Struct, self).__getattribute__(arg)

        if arg in self._structs:
            return self._structs[arg]
        else:
            return super(Struct, self).__getattribute__(arg)

    def __setattr__(self, attribute, value):
        if self.__frozen and not attribute in self.__dict__ and not attribute in self._meta:
            raise AttributeError("No such attribute: '{}'".format(attribute))
        return super(Struct, self).__setattr__(attribute, value)

    def __getitem__(self, value):
        # shallow copy of self
        other = copy.copy(self)
        # deep copy of the path
        other._path = copy.deepcopy(self._path)
        other._path.append(value)
        return other

    def invariants(self, conj=None):
        if conj is None:
            conj = []
        for k, v in self._meta.items():
            if k in self._structs:
                v = self._structs[k]
            for c in v.invariants():
                conj.append(c)
        return conj

    def initial(self):
        kp = self.copy()
        for k, v in kp._meta.items():
            if k in kp._structs:
                kp._structs[k] = kp._structs[k].initial()
            else:
                v.initial(kp)
        return kp

    def ite(self, other, cond):
        assert self.__class__ == other.__class__
        new = copy.copy(self)
        new._fields = util.If(cond, self._fields, other._fields)
        new._structs = util.If(cond, self._structs, other._structs)
        return new


class BaseStruct(Struct):
    def __init__(self):
        self._init(None, None)


class Map(AbstractMap):
    def new(self):
        if self.get_arity() == 0:
            types = self.get_types()
            assert len(types) == 1
            typ = types[0]
            if typ == z3.BoolSort():
                value = util.FreshBool(self._prefix + self._name)
            else:
                value = util.FreshBitVec(self._prefix + self._name, typ)
        else:
            value = util.FreshFunction(self._prefix + self._name, *self.get_types())
        return value

    def get(self, fn, args):
        if args:
            return fn(*args)
        else:
            return fn

    def set(self, old, args, value):
        # "raw" assignment of a lambda without any accessor arguments
        if isinstance(value, types.FunctionType):
            assert not args
            return lambda *args: value(*args, oldfn=old)
        elif self.get_arity() == 0:
            # assignment to a value
            return value
        else:
            # assignment with value with partial arguments
            return util.partial_update(old, args, value)
#from base.py done

uint_t = z3.BitVecSort(32)
uint64_t = z3.BitVecSort(64)

class IntArray(Struct):
    val = Map(uint64_t, uint_t)

class GlobalState(BaseStruct):
    #z = Map(uint_t)
    a = IntArray()

class IRPyTestMeta(type):
    def __new__(cls, name, parents, dct):
        cls._add_funcs(name, parents, dct)
        return super(IRPyTestMeta, cls).__new__(cls, name, parents, dct)

    @classmethod
    def _add_funcs(cls, name, parents, dct):
        for test in dct.get('tests', []):
	    #print(dct)
            dct['test_{}'.format(test)] = lambda self, test=test: self._test(test)
	    #print(dct)


class IRPyTestBase(unittest.TestCase):
    __metaclass__ = IRPyTestMeta

    def _prove(self, cond, pre=None, return_model=False, minimize=True):
        if minimize:
            self.solver.push()
        self.solver.add(z3.Not(cond))

        res = self.solver.check()
        if res != z3.unsat:
            if not minimize and not return_model:
                self.assertEquals(res, z3.unsat)

            model = self.solver.model()
            if return_model:
                return model

            print "Could not prove, trying to find a minimal ce"
            assert res != z3.unknown
            if z3.is_and(cond):
                self.solver.pop()
                # Condition is a conjunction of some child clauses.
                # See if we can find something smaller that is sat.

                if pre is not None:
                    ccond = sorted(
                        zip(cond.children(), pre.children()), key=lambda x: len(str(x[0])))
                else:
                    ccond = sorted(cond.children(), key=lambda x: len(str(x)))

                for i in ccond:
                    self.solver.push()
                    if isinstance(i, tuple):
                        self._prove(i[0], pre=i[1])
                    else:
                        self._prove(i)
                    self.solver.pop()

            print "Can not minimize condition further"
            if pre is not None:
                print "Precondition"
                print pre
                print "does not imply"
            print cond
            self.assertEquals(model, [])

def test_arg():
    target = util.FreshBitVec('target', uint_t)
    return (target,)

def test_spec1(old, target):
    cond = []
    return cond 

def test_spec2(old, target):
    cond = -1
    for i in range(0, 10):
        if (old.a[i].val == target):
            cond = i
            break
    return cond 

def impl_invariants(ctx):
    conj = []
    index = util.FreshBitVec('index', uint64_t)

    conj.append(z3.ForAll([index], z3.Implies(z3.ULT(index, 9),
        z3.ULE(util.global_field_element(ctx, '@a', None, index), util.global_field_element(ctx, '@a', None, index + util.i64(1))))))

    return z3.And(*conj)

def state_equiv(ctx, globalstate):
    conj = []
    index = util.FreshBitVec('index', uint64_t)

    #conj.append(globalstate.z == util.global_value(ctx, '@z'))
    conj.append(z3.ForAll([index], z3.Implies(z3.ULT(index, 10),
        util.global_field_element(ctx, '@a', None, index) == 
        globalstate.a[index].val)))

    conj.append(impl_invariants(ctx))

    return z3.And(*conj)

class IRPyTest(IRPyTestBase):
    def setUp(self):
        self.ctx = libirpy.newctx()
        self.state = GlobalState()

        self.solver = Solver()
        self.solver.set(AUTO_CONFIG=False)

    def _test_o(self, name):
        #module = __import__(name)
        model_filename='ir_verify.'+name
        module = importlib.import_module(model_filename)
        self.ctx.eval.declare_global_constant = self.ctx.eval.declare_global_variable
        libirpy.initctx(self.ctx, module)

        self._pre_state = state_equiv(self.ctx, self.state)
        self.ctx.add_assumption(impl_invariants(self.ctx))
        self.solver.add(self._pre_state)
        
        args = test_arg()
        res = self.ctx.call('@test', *args)

        #cond, newstate = test_spec(self.state, *args)
        #model = self._prove(z3.And(state_equiv(self.ctx, newstate),
        #                           cond == (res == util.i32(0))),
        #                    pre=z3.And(self._pre_state, z3.BoolVal(True)),
        #                    return_model=INTERACTIVE)
        # add a new check func
        cond = test_spec2(self.state, *args)
        #model = self._prove(z3.And(cond == res, impl_invariants(self.ctx)))
        model = self._prove(cond == res)
        
        if INTERACTIVE and model:
            from ipdb import set_trace
            set_trace()

    def _test(self, name):
        self._test_o(name)
    
    tests = _tests

if __name__ == '__main__':
    unittest.main()

