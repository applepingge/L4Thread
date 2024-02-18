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
import libirpy.datatypes as dt
from libirpy.datatypes import BitcastPointer
import libirpy.itypes as itypes
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

def test_spec(old):
    new = old.copy()

    #for i in range(0, 5):
    #    idx = i
    #    for j in range(i + 1, 5):
    #        if new.a[j].val < new.a[idx].val:
    #            idx = j
    #        new.a[i].val, new.a[idx].val = new.a[idx].val, new.a[i].val
    return new

def impl_invariants(ctx):
    conj = []
    index = util.FreshBitVec('index', uint64_t)
    irpy = ctx['eval']

    #conj.append(z3.ForAll([index], z3.Implies(z3.ULT(index, 4),
    #        z3.ULE(util.global_field_element(ctx, '@a', None, index), util.global_field_element(ctx, '@a', None, index + util.i64(1))))))
    scheduler_tid = util.global_field_element(ctx, '@t', 'scheduler', 
        index).getelementptr(util.i64(0), util.i64(0)).read(ctx)
    scheduler_tcb_globalid = util.global_field_element(ctx, '@t', 'myself_global', 
        scheduler_tid).getelementptr(util.i64(0), util.i64(0)).read(ctx)
    # Inv_Threads_Sche_In_Threads
    conj.append(z3.ForAll([index], z3.Implies(z3.ULT(index, 256),
            z3.And(z3.ULT(scheduler_tid, 256), scheduler_tcb_globalid != 0))))

    current_tcb = ctx.globals['@current_tcb']
    idle_tcb = ctx.globals['@__idle_tcb']
    current_tcb_utcb = current_tcb.getelementptr(util.i64(0), util.i64(3), 
        type=itypes.parse_type(ctx,'%class.utcb_t**')).read(ctx)
    # Inv_Current_Thread_In_Active
    conj.append(z3.Or(
        current_tcb == idle_tcb, current_tcb_utcb != irpy.constant_pointer_null(ctx, itypes.parse_type(ctx,'%class.utcb_t*'))
    ))

    return z3.And(*conj)

def state_equiv(ctx, globalstate):
    conj = []
    index = util.FreshBitVec('index', uint64_t)

    #conj.append(globalstate.z == util.global_value(ctx, '@z'))

    #conj.append(impl_invariants(ctx))

    return z3.And(*conj)

def llvm_memset(ctx, ptr, val, size, align, isvolatile):
    #val = z3.Extract(7, 0, val)
    pdb.set_trace()
    size = size.as_long()
    
    # If we're passed a bitcasted pointer we just check if the write size is a
    # multiple of the underlying types write size, then we can just ignore the bitcast.
    if isinstance(ptr, BitcastPointer):
        ptr = ptr._ptr
    
    inner = ptr.type().deref()

    # We are memsetting an array whose inner type matches the size of the val
    assert inner.is_array()
    assert inner.deref().size() % val.size() == 0

    val = z3.Concat(*([val] * (inner.deref().size() / val.size())))

    if inner.deref().is_int():
        array_len = ptr.type().deref().length()

        dst_start = ptr.getelementptr(ctx, util.i64(0), util.i64(0))
        dst_end = ptr.getelementptr(
            ctx, util.i64(0), util.i64(array_len - 1))

        dst_start_path = dst_start.canonical_path()
        dst_end_path = dst_end.canonical_path()

        dst_tup, dst_start_args = ptr._ref.build_field_tuple_and_path(
            ctx, dst_start_path)
        _, dst_end_args = ptr._ref.build_field_tuple_and_path(
            ctx, dst_end_path)

        dstfn = ctx['references'][ptr._ref._name][dst_tup]

        def newf(*args):
            assert len(args) == len(dst_end_args)
            cond = []
            for a, b in zip(args[:-1], dst_start_args[:-1]):
                cond.append(a == b)

            cond.append(z3.UGE(args[-1], dst_start_args[-1]))
            cond.append(z3.ULE(args[-1], dst_end_args[-1]))

            cond = z3.And(*cond)

            return util.If(cond, val, dstfn(*args))

        ctx['references'][ptr._ref._name][dst_tup] = newf
        return ptr

    else:
        raise NotImplementedError(
            "Don't know how to memset {!r}".format(inner))

def test_arg():
    dest_tid = util.FreshBitVec('dest_tid', uint64_t)
    space_tid = util.FreshBitVec('space_tid', uint64_t)
    #space_tid = util.i64(0)
    scheduler_tid = util.FreshBitVec('scheduler_tid', uint64_t)
    pager_tid = util.FreshBitVec('pager_tid', uint64_t)
    utcb_location = util.FreshBitVec('utcb_location', uint64_t)
    return (dest_tid, space_tid, scheduler_tid, pager_tid, utcb_location)

def set_pre_cond(ctx, args):
    conj = []
    #solver = Solver()
    conj.append(z3.UGT(z3.Extract(5, 0, args[0]), 0))
    conj.append(z3.UGT(args[1], 0))
    conj.append(z3.UGE(z3.Extract(63, 32, args[0]), util.i32(32+32+3)))
    conj.append(z3.ULT(z3.Extract(63, 32, args[0]), util.i32(32+32+256)))
    ctx.add_assumption(z3.And(*conj))
    return

def set_pre_cond_delete_thread(ctx, args):
    conj = []
    # dest_tid is global
    conj.append(z3.UGT(z3.Extract(5, 0, args[0]), 0))
    # space_tid is nilthread
    conj.append(args[1] == 0)
    # scheduler_tid is global
    conj.append(z3.UGT(z3.Extract(5, 0, args[2]), 0))
    # pager_tid is global
    conj.append(z3.UGT(z3.Extract(5, 0, args[3]), 0))

    current_tcb = ctx.globals['@current_tcb']
    idle_tcb = ctx.globals['@__idle_tcb']
    conj.append(current_tcb != idle_tcb)
    current_space = current_tcb.getelementptr(ctx,util.i64(0),util.i64(25),
        type=itypes.parse_type(ctx,'%class.space_t**')).read(ctx)
    # current_space == sigma0_space || current_space == sigma1_space || current_space == roottask_space
    conj.append(is_privileged_space(current_space))
    current_tid = current_tcb.getelementptr(ctx,util.i64(0),util.i64(0),util.i64(0),util.i64(0),
        type=itypes.parse_type(ctx,'i64*')).read(ctx)
    # current_tid != dest_tid
    conj.append(z3.Extract(5, 0, current_tid) != z3.Extract(5, 0, args[0]))
    # dest_tid is user thread && dest_tid != sigma0, sigma1, rootserver
    conj.append(z3.UGE(z3.Extract(63, 32, args[0]), util.i32(32+32+3)))
    conj.append(z3.ULT(z3.Extract(63, 32, args[0]), util.i32(32+32+256)))
    ctx.add_assumption(z3.And*(conj))
    return

def post_cond_delete_tcb(ctx, args):
    conj = []

    dest_tcb = func_get_tcb_simple(ctx, z3.Concat(util.i32(0), z3.Extract(63, 32, args[0])))
    dest_global_id = dest_tcb.getelementptr(ctx,util.i64(0),util.i64(0),util.i64(0),util.i64(0),
        type=itypes.parse_type(ctx,'i64*')).read(ctx)
    # dest_tid is nilthread
    conj.append(dest_global_id == 0)

    dest_state = dest_tcb.getelementptr(ctx,util.i64(0),util.i64(4),util.i64(0),
        type=itypes.parse_type(ctx,'i64*')).read(ctx)
    # dest_tcb state is aborted
    conj.append(dest_state == 15)

    print("post condition for delete tcb done")
    return z3.And(*conj)

def set_pre_cond_modify_thread(ctx, args):
    conj = []
    irpy = ctx['eval']
    # dest_tid is global
    conj.append(z3.UGT(z3.Extract(5, 0, args[0]), 0))
    # space_tid is not nilthread and global
    conj.append(z3.And(z3.UGT(args[1], 0), z3.UGT(z3.Extract(5, 0, args[1]), 0)))
    # scheduler_tid is global
    conj.append(z3.UGT(z3.Extract(5, 0, args[2]), 0))
    # pager_tid is global
    conj.append(z3.UGT(z3.Extract(5, 0, args[3]), 0))

    current_tcb = ctx.globals['@current_tcb']
    idle_tcb = ctx.globals['@__idle_tcb']
    conj.append(current_tcb != idle_tcb)
    current_space = current_tcb.getelementptr(ctx,util.i64(0),util.i64(25),
        type=itypes.parse_type(ctx,'%class.space_t**')).read(ctx)
    # current_space == sigma0_space || current_space == sigma1_space || current_space == roottask_space
    conj.append(is_privileged_space(current_space))
    # dest_tid is user thread && dest_tid != sigma0, sigma1, rootserver
    conj.append(z3.UGE(z3.Extract(63, 32, args[0]), util.i32(32+32+3)))
    conj.append(z3.ULT(z3.Extract(63, 32, args[0]), util.i32(32+32+256)))

    dest_tcb = func_get_tcb_simple(ctx, z3.Concat(util.i32(0), z3.Extract(63, 32, args[0])))
    dest_space = dest_tcb.getelementptr(ctx,util.i64(0),util.i64(25),
        type=itypes.parse_type(ctx,'%class.space_t**')).read(ctx)
    # dest_tcb space is not null
    conj.append(dest_space != irpy.constant_pointer_null(ctx, itypes.parse_type(ctx,'%class.space_t*')))
    ctx.add_assumption(z3.And*(conj))
    return

def post_cond_modify_tcb(ctx, args):
    conj = []

    dest_tcb = func_get_tcb_simple(ctx, z3.Concat(util.i32(0), z3.Extract(63, 32, args[0])))
    scheduler_tid = dest_tcb.getelementptr(ctx, util.i64(0), util.i64(24),util.i64(0),util.i64(0),
        type=itypes.parse_type(ctx, 'i64*')).read(ctx)
    # set scheduler
    conj.append(z3.Implies(args[2] != 0,
        scheduler_tid == args[2]
    ))
    
    global_id = dest_tcb.getelementptr(ctx,util.i64(0),util.i64(0),util.i64(0),util.i64(0),
        type=itypes.parse_type(ctx,'i64*')).read(ctx)
    # set global id
    conj.append(global_id == args[0])

    print("post condition for modify tcb done")
    return z3.And(*conj)


def func__get_space_args(ctx,name):
    arg1 = dt.fresh_ptr(ctx, util.fresh_name(name),
        itypes.parse_type(ctx,'%class.space_t*'))
    return arg1

def func__get_tcb_args(ctx,name):
    arg1 = dt.fresh_ptr(ctx, util.fresh_name(name),
        itypes.parse_type(ctx,'%class.tcb_t*'))
    #util.FreshBitVec('x', 64)
    #arg1.getelementptr(ctx,util.i64(0),util.i64(0),util.i64(0)).write(ctx,util.FreshBitVec('x', 64))
    #arg1.getelementptr(ctx,util.i64(0),util.i64(0),util.i64(0)).read(ctx)
    return arg1

def func__get_utcb_args(ctx,name):
    arg1 = dt.fresh_ptr(ctx, util.fresh_name(name),
        itypes.parse_type(ctx,'%class.utcb_t*'))
    #util.FreshBitVec('x', 64)
    #arg1.getelementptr(ctx,util.i64(0),util.i64(0),util.i64(0)).write(ctx,util.FreshBitVec('x', 64))
    #arg1.getelementptr(ctx,util.i64(0),util.i64(0),util.i64(0)).read(ctx)
    return arg1

#32 irqs, 32 kernels, sigma0, sigma1, roottask
def func_get_tcb_simple(ctx, num):
    irpy = ctx['eval']
    return irpy.get_element_ptr(ctx,itypes.parse_type(ctx,'%class.tcb_t*'),
        irpy.global_value(ctx,itypes.parse_type(ctx,'[134217729 x %class.tcb_t]*'),'@t',),
        itypes.parse_type(ctx,'[134217729 x %class.tcb_t]*'),irpy.constant_int(ctx,0,64,),
        itypes.parse_type(ctx,'i64'),num,itypes.parse_type(ctx,'i64'),inbounds=True,nuw=True,)

def func_init_rootservers_simple(ctx):
    roottask_space = ctx.globals['@roottask_space']
    sigma0_space = ctx.globals['@sigma0_space']
    sigma1_space = ctx.globals['@sigma1_space']
    utcb_area = z3.BitVecVal((18 << 4) | ((1 << 32) << 10), 64)

    space_root = func__get_space_args(ctx, 'root')
    space_root.getelementptr(ctx, util.i64(0), util.i64(0), util.i64(0),util.i64(1022),
        type=itypes.parse_type(ctx, 'i64*')).write(ctx, util.i64(2))
    space_root.getelementptr(ctx, util.i64(0), util.i64(0), util.i64(0),util.i64(1021),
        type=itypes.parse_type(ctx, 'i64*')).write(ctx, utcb_area)
    roottask_space.write(ctx, space_root)
    space_sigma = func__get_space_args(ctx, 'sigma')
    space_sigma.getelementptr(ctx, util.i64(0), util.i64(0), util.i64(0),util.i64(1022),
        type=itypes.parse_type(ctx, 'i64*')).write(ctx, util.i64(2))
    space_sigma.getelementptr(ctx, util.i64(0), util.i64(0), util.i64(0),util.i64(1021),
        type=itypes.parse_type(ctx, 'i64*')).write(ctx, utcb_area)        
    sigma0_space.write(ctx, space_sigma)
    sigma1_space.write(ctx, space_sigma)

    sigma0_tcb = func_get_tcb_simple(ctx, util.i64(32+32))
    sigma0_tcb.getelementptr(ctx,util.i64(0),util.i64(25),
        type=itypes.parse_type(ctx,'%class.space_t**')).write(ctx, space_sigma)
    sigma1_tcb = func_get_tcb_simple(ctx, util.i64(32+32+1))
    sigma1_tcb.getelementptr(ctx,util.i64(0),util.i64(25),
        type=itypes.parse_type(ctx,'%class.space_t**')).write(ctx, space_sigma)
    roottask_tcb = func_get_tcb_simple(ctx, util.i64(32+32+2))
    roottask_tcb.getelementptr(ctx,util.i64(0),util.i64(25),
        type=itypes.parse_type(ctx,'%class.space_t**')).write(ctx, space_root)
    
    kip = ctx.globals['@kip']
    thread_info_val = util.simplify(((util.i64(64) << 12) + util.i64(32)) << 8)
    #pdb.set_trace()
    thread_info = kip.getelementptr(ctx,util.i64(0),util.i64(28), util.i64(0)).write(ctx, thread_info_val)
    #tup = (kip.getelementptr(ctx,util.i64(0),util.i64(28), type=itypes.parse_type(ctx,'%union.anon*')),)
    #res = ctx.call('@_ZN13thread_info_t15get_system_baseEv', *tup)
    
    return space_root

def func_is_global_test(ctx):
    x = util.FreshBitVec('x', 64)
    arg1 = dt.fresh_ptr(ctx, util.fresh_name('g'),
        itypes.parse_type(ctx,'%class.threadid_t*'))
    arg1.getelementptr(ctx, util.i64(0), util.i64(0), util.i64(0)).write(ctx, x)
    res = ctx.call('@_ZN10threadid_t9is_globalEv', arg1)
    #pdb.set_trace()
    return

def func_init_tcb_user(ctx):
    irpy = ctx['eval']
    # exists activated
    for i in range(3, 100):
        tcb = func_get_tcb_simple(ctx, util.i64(32+32+i))
        space = func__get_space_args(ctx, 'space')
        space.getelementptr(ctx, util.i64(0), util.i64(0), util.i64(0), util.i64(1022),
            type=itypes.parse_type(ctx, 'i64*')).write(ctx, util.i64(2))
        utcb_area = z3.BitVecVal((18 << 4) | ((1 << 32) << 10), 64)
        space.getelementptr(ctx, util.i64(0), util.i64(0), util.i64(0), util.i64(1021),
            type=itypes.parse_type(ctx, 'i64*')).write(ctx, utcb_area)
        tcb_space = tcb.getelementptr(ctx,util.i64(0),util.i64(25),type=itypes.parse_type(ctx,'%class.space_t**'))
        tcb_space.write(ctx, space)
        utcb = func__get_utcb_args(ctx,'utcb')
        tcb_utcb = tcb.getelementptr(ctx,util.i64(0),util.i64(3),type=itypes.parse_type(ctx,'%class.utcb_t**'))
        tcb_utcb.write(ctx, utcb)

    # exists not activated
    for i in range(100, 256):
        tcb = func_get_tcb_simple(ctx, util.i64(32+32+i))
        space = func__get_space_args(ctx, 'space')
        space.getelementptr(ctx, util.i64(0), util.i64(0), util.i64(0), util.i64(1022),
            type=itypes.parse_type(ctx, 'i64*')).write(ctx, util.i64(2))
        utcb_area = z3.BitVecVal((18 << 4) | ((1 << 32) << 10), 64)
        space.getelementptr(ctx, util.i64(0), util.i64(0), util.i64(0), util.i64(1021),
            type=itypes.parse_type(ctx, 'i64*')).write(ctx, utcb_area)
        tcb_space = tcb.getelementptr(ctx,util.i64(0),util.i64(25),type=itypes.parse_type(ctx,'%class.space_t**'))
        tcb_space.write(ctx, space)
        tcb_utcb = tcb.getelementptr(ctx,util.i64(0),util.i64(3),type=itypes.parse_type(ctx,'%class.utcb_t**'))
        tcb_utcb.write(ctx,irpy.constant_pointer_null(ctx, itypes.parse_type(ctx,'%class.utcb_t*')))

    # not exists not activated
    #for i in range(200, 256):
    #    tcb = func_get_tcb_simple(ctx, util.i64(32+32+i))
    #    tcb.getelementptr(ctx,util.i64(0),util.i64(25),
    #        type=itypes.parse_type(ctx,'%class.space_t**')).write(ctx,irpy.constant_pointer_null(ctx, itypes.parse_type(ctx,'%class.space_t*')))
    #    tcb.getelementptr(ctx,util.i64(0),util.i64(3),
    #        type=itypes.parse_type(ctx,'%class.utcb_t**')).write(ctx,irpy.constant_pointer_null(ctx, itypes.parse_type(ctx,'%class.utcb_t*')))
    return

def func_sys_thread_control_test(ctx):
    zero_i64 = z3.BitVecVal(0, 64)
    irpy = ctx['eval']
    zero_32 = irpy.constant_int(ctx,0,32,)
    one_32 = irpy.constant_int(ctx,1,32,)
    zero_64 = irpy.constant_int(ctx,0,64,)
    one_64 = irpy.constant_int(ctx,1,64,)
    tf_64 = irpy.constant_int(ctx,25,64,)
    #ctx['references'][p._ref._name][(x,)](0)
    conj = []
    pre = []

    space_root = func_init_rootservers_simple(ctx)

    #space = func__get_space_args(ctx,'space')
    tcb = func__get_tcb_args(ctx,'tcb')
    tcb_space = tcb.getelementptr(ctx,zero_64,tf_64,type=itypes.parse_type(ctx,'%class.space_t**'))
    tcb_space.write(ctx,space_root)
    utcb = func__get_utcb_args(ctx,'utcb')
    tcb_utcb = tcb.getelementptr(ctx,zero_64,irpy.constant_int(ctx,3,64,),type=itypes.parse_type(ctx,'%class.utcb_t**'))
    tcb_utcb.write(ctx,utcb)
    utcb_preempt_flag = utcb.getelementptr(ctx,zero_64,irpy.constant_int(ctx,5,64,),type=itypes.parse_type(ctx,'i8*'))
    utcb_preempt_flag.write(ctx,util.FreshBitVec('flag', 8))
    current_tcb = ctx.globals['@current_tcb']
    scheduler = ctx.globals['@scheduler']
    #pdb.set_trace()
    current_tcb.write(ctx,tcb)
    
    func_init_tcb_user(ctx)

    args = test_arg()
    set_pre_cond(ctx, args)
    #dest_space = func__get_space_args(ctx, 'dest_space')
    #dest_tcb = ctx.call('@_ZN7space_t7get_tcbE10threadid_t', dest_space, args[0])
    #pdb.set_trace()
    #ctx.call('@_ZN7space_t7add_tcbEP5tcb_t', dest_space, dest_tcb)
    #ctx.call('@_ZN7space_t7add_tcbEP5tcb_t', dest_space, dest_tcb)
    #ctx.call('@_ZN7space_t7add_tcbEP5tcb_t', dest_space, dest_tcb)
    #dest_tcb.getelementptr(ctx,zero_64,tf_64,type=itypes.parse_type(ctx,'%class.space_t**')).write(ctx, dest_space)
    #pdb.set_trace()
    res = ctx.call('@sys_thread_control', *args)
    #pdb.set_trace()
    
    if res == util.i64(0):
        print("ERROR CODE: ")
        print(utcb.getelementptr(ctx,zero_64,irpy.constant_int(ctx,10,64,),
            type=itypes.parse_type(ctx,'i64*')).read(ctx))
    #else:
        #print(res)
    #return post_cond_delete_tcb(ctx, args)
    return post_cond_modify_tcb(ctx, args)

def allocate(ctx,tcb):
    return

def delete_tcb(ctx, tcb):
    # myself_global myself_local
    tcb.getelementptr(ctx,util.i64(0),util.i64(0),util.i64(0), util.i64(0),
        type=itypes.parse_type(ctx,'i64*')).write(ctx, util.i64(0))
    tcb.getelementptr(ctx,util.i64(0),util.i64(1),util.i64(0), util.i64(0),
        type=itypes.parse_type(ctx,'i64*')).write(ctx, util.i64(0))
    
    # set_state
    tcb.getelementptr(ctx,util.i64(0),util.i64(4),util.i64(0),
        type=itypes.parse_type(ctx,'i64*')).write(ctx, util.i64((7 << 1) | 1))
    return

def remove_tcb(ctx, space, tcb):
    return z3.BoolVal(False)

def check_utcb_location(ctx, tcb, loc):
    #pdb.set_trace()
    tcb_space = tcb.getelementptr(ctx, util.i64(0),util.i64(25),
        type=itypes.parse_type(ctx,'%class.space_t**')).read(ctx)
    utcb_area = tcb_space.getelementptr(ctx, util.i64(0), util.i64(0), util.i64(0), util.i64(1021),
        type=itypes.parse_type(ctx, 'i64*')).read(ctx)
    
    #is_range_in_fpage
    utcb_area_size = (utcb_area >> 4) & 63
    utcb_area_base = utcb_area >> 10

    return z3.Or(z3.And(utcb_area_size == 1, utcb_area_base == 0), 
            z3.And(z3.ULE(utcb_area_base << 10, loc), 
                z3.UGE((utcb_area_base << 10) + (1 << utcb_area_size), loc + 1024)))

def activate(ctx, tcb, startup_func, pager):
    return z3.BoolVal(True)

def init(ctx, tcb, dest):
    # myself_global myself_local
    tcb.getelementptr(ctx,util.i64(0),util.i64(0),util.i64(0), util.i64(0),
        type=itypes.parse_type(ctx,'i64*')).write(ctx, dest)
    tcb.getelementptr(ctx,util.i64(0),util.i64(1),util.i64(0), util.i64(0),
        type=itypes.parse_type(ctx,'i64*')).write(ctx, util.i64(0))
    
    # set_state
    tcb.getelementptr(ctx,util.i64(0),util.i64(4),util.i64(0),
        type=itypes.parse_type(ctx,'i64*')).write(ctx, util.i64((7 << 1) | 1))

def allocate_space(ctx):
    return func__get_space_args(ctx, 'space')

def migrate_to_space(ctx, tcb, space):
    return z3.BoolVal(False)

def nilthread(ctx):
    value = dt.fresh_ptr(ctx, util.fresh_name('nilthread'),
        itypes.parse_type(ctx,'{ i64 }'))
    value.getelementptr(ctx, util.i64(0)).write(ctx, util.i64(0))
    return value

def llvm_lifetime_start(ctx, a, b):
    return

def llvm_lifetime_end(ctx, a, b):
    return

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

        self.ctx.globals['@llvm_memset_p0i8_i64'] = llvm_memset
        self.ctx.globals['@_ZN5tcb_t8allocateEv'] = allocate
        self.ctx.globals['@_ZN5tcb_t10delete_tcbEv'] = delete_tcb
        self.ctx.globals['@_ZN7space_t10remove_tcbEP5tcb_t'] = remove_tcb
        self.ctx.globals['@_ZN5tcb_t19check_utcb_locationEm'] = check_utcb_location
        self.ctx.globals['@_ZN5tcb_t8activateEPFvvE10threadid_t'] = activate
        self.ctx.globals['@_ZN5tcb_t4initE10threadid_t'] = init
        self.ctx.globals['@_Z14allocate_spacev'] = allocate_space
        self.ctx.globals['@_ZN5tcb_t16migrate_to_spaceEP7space_t'] = migrate_to_space
        self.ctx.globals['@_ZL14thread_startupv'] = self.ctx.eval.constant_pointer_null(self.ctx, itypes.parse_type(self.ctx,'void ()*'))
        self.ctx.globals['@_ZN10threadid_t9nilthreadEv'] = nilthread
        self.ctx.globals['@llvm_lifetime_start_p0i8'] = llvm_lifetime_start
        self.ctx.globals['@llvm_lifetime_end_p0i8'] = llvm_lifetime_end
        #self._pre_state = state_equiv(self.ctx, self.state)
        #self.ctx.add_assumption(impl_invariants(self.ctx))
        #self.solver.add(self._pre_state)
        
        #pdb.set_trace()
        post = func_sys_thread_control_test(self.ctx)
        self.solver.add(z3.And(*self.ctx.path_condition))
        #func_is_global_test(self.ctx)
        #pdb.set_trace()
        #newstate = test_spec(self.state)
        model = self._prove(post,
                            #pre=self._pre_state,
                            return_model=INTERACTIVE)
        # add a new check func
        
        if INTERACTIVE and model:
            from ipdb import set_trace
            set_trace()

    def _test(self, name):
        self._test_o(name)
    
    tests = _tests

if __name__ == '__main__':
    unittest.main()

