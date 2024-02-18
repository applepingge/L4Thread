FROM		:= src/api/v4
TARGET		:= $(FROM)/l4.ll
TARGET_FOLDER	:= ../ir_verify
SRCS		= $(wildcard ./$(FROM)/*.cc)
LLS		= $(SRCS:.cc=.ll)

LINK		:= llvm-link-5.0
CLANG		:= clang++-5.0
INCLUDE		:= -I include/
INCLUDE		+= -include include/config.h
INCLUDE		+= -include include/types.h
IMACROS		:= -imacros config/config.h
IMACROS		+= -imacros include/macros.h
ARCH		:= --target=mips64
CONFIG		:= -g -mno-sse -mno-sse2 -nostdlib -O1 -S -emit-llvm


ccir: $(LLS)
	@$(LINK) -o $(TARGET) -S $^
	@mv $(TARGET) $(TARGET_FOLDER)

$(FROM)/%.ll: $(FROM)/%.cc
	@$(CLANG) $(ARCH) $(CONFIG) $(INCLUDE) $(IMACROS) $^ -o $@

.PHONY: clean
clean:
	@rm -rf $(TARGET_FOLDER)/*.ll
	@rm -rf $(FROM)/*.ll
