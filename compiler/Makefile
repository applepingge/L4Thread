IRPY_SRCS	:= $(wildcard *.cc)
IRPY_OBJS	:= $(IRPY_SRCS:.cc=.o)

CXX = g++

irpy: $(IRPY_OBJS)
	@$(CXX) $^ -L$(LIBDIR) $(LIBS) $(SYSLIBS) -g -O2 -o "$@"

%.o: %.cc
	@$(CXX) -o $@ -c $(CXXFLAGS) -Wall -g -O2 $<

.PHONY: clean
clean:
	@rm -rf *.o
	@rm -rf irpy
