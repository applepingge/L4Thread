COMPILER	:= ../compiler/irpy
PYTHON		:= python2
CLANG		:= clang++-5.0
CONFIG		:= -g -mno-sse -mno-sse2 -nostdlib -O1 -S -emit-llvm
MAKE		:= make
MAKE_CLEAN	:= make clean

case1: test/case1.c 
	@cp $^ case.c
	@make case

case2: test/case2.c
	@cp $^ case.c
	@make case

case3: test/case3.c
	@cp $^ case.c
	@make case

case4: test/case4.c
	@cp $^ case.c
	@make case

case5: test/case5.cpp
	@cp $^ case.cpp
	@make case

case6: test/case6.cpp
	@cp $^ case.cpp
	@make case
#add case

case: ../libirpy/case.py
	@$(PYTHON) ../test.py

../libirpy/case.py: case.ll $(COMPILER)
	@$(COMPILER) case.ll > case.py

$(COMPILER): 
	@$(MAKE) -C ../compiler

case.ll: case.cpp
	@$(CLANG) $(CONFIG) $^ -o $@

.PHONY: clean
clean:
	@rm -rf *.ll
	@rm -rf *.cpp
	@rm -rf case.py
	@rm -rf case.pyc
