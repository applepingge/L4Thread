验证kernel	make
	数据流：kernel/src/api/v4/*.cc ——> kernel/src/api/v4/ *.ll ——(link)——> ir_verify/l4.ll 
		 ——(compiler/irpy)——> ir_verify/case.py ——(test.py)——> 验证
验证某函数	make case1
		make case2
		make ……
	数据流：ir_verify/test/ %.c ——> ir_verify/ case.ll ——
		 ——(compiler/irpy)——> ir_verify/case.py ——(test.py)——> 验证
	如何增加case:
		在ir-verify/test中添加c文件
		在ir-verify/makefile中添加相应target
		在当前makefile中添加相应target
