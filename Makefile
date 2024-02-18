export
LLVM_CONFIG  := llvm-config-5.0
LIBDIR		= $(shell "$(LLVM_CONFIG)" --libdir)
LIBS		= $(shell "$(LLVM_CONFIG)" --libs)
SYSLIBS		= $(shell "$(LLVM_CONFIG)" --system-libs)
CXXFLAGS	= $(shell "$(LLVM_CONFIG)" --cxxflags)

PYTHON		:= python2
LL			:= ir_verify/l4.ll
PY			:= ir_verify/case.py
COMPILER	:= ./compiler/irpy
TEST		:= test.py
MAKE		:= make
MAKE_CLEAN	:= make clean
TEST_PATH	:= ir_verify/test


start: $(PY)
	@$(PYTHON) $(TEST)

$(PY): $(COMPILER) $(LL)
	@$(COMPILER) $(LL) > $(PY)

$(COMPILER):
	@$(MAKE) -C compiler

$(LL):
	@$(MAKE) -C kernel

case1: $(TEST_PATH)/case1.c
	@$(MAKE) $@ -C ir_verify

case2: $(TEST_PATH)/case2.c
	@$(MAKE) $@ -C ir_verify

case3: $(TEST_PATH)/case3.c
	@$(MAKE) $@ -C ir_verify

#add case

.PHONY: clean
clean:
	@rm -rf $(PY)
	@rm -rf $(PYC)
	@cd kernel && $(MAKE_CLEAN)
	@cd ir_verify && $(MAKE_CLEAN)
