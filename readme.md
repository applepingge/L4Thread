   
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
