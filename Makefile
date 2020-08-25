# good makefile explanation
# -isystem import headers as system (will ignore any warnings)
#  -I. add . as directory, allows including headers in the good way
# auto-dependency are for headers only
# lib.a is equivalent to .so

EXEC = run
EXTPATH =/home/savar/currentstage/satsolver_glucose_4.1
CXXFLAGS = -I. -isystem$(EXTPATH) -std=c++17 -Werror -Wall -Wextra -O3 # -pedantic -g
LDFLAGS = -lglucose -lgecodesearch -lgecodeint -lgecodekernel -lgecodesupport -lgecodeminimodel


src = $(wildcard mem_schedule/*.cpp) \
			$(wildcard solvers/*/*.cpp) \
			$(wildcard *.cpp)
obj = $(src:.cpp=.o)
dep = $(obj:.o=.d)  # one dependency file for each source


$(EXEC): $(obj)
	$(CXX) -L$(EXTPATH) -o $@ $^ $(LDFLAGS)
	@echo
	@echo Build success

# generate dependencies
%.d: %.cpp
	$(CXX) -I$(EXTPATH) -I. $(FLAGS) $< -MM -MT $(@:.d=.o) > $@

-include $(dep)   # include all dep files in the makefile

.PHONY: clean
clean:
	rm -f $(obj) $(EXEC) 

.PHONY: cleanall
cleanall: clean
	rm -f $(dep)
