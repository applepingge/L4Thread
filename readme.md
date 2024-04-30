Verifying Env:

 	Ubuntu 18.04

  	LLVM 5.0/ Clang 5.0

   	Z3 4.5.1.0

    	Python 2.7.0

   	
verifying a function:	

	make case1
 
	make case2
 
	make ……
  
data flow：
	
 	ir_verify/test/ %.c ——> ir_verify/ case.ll ——> (compiler/irpy)——> ir_verify/case.py ——(test.py)——> verify

How to add a case:

	1. add c files in the directory of ir-verify/test
 
	2. add corresponding targets in file ir-verify/makefile
 
	3. add corresponding targets in the current makefile
