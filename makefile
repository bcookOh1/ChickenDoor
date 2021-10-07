# Raspberry pi chicken door project
# ref: https://www.gnu.org/software/make/manual/html_node/index.html#Top
# ref: http://nuclear.mutantstargoat.com/articles/make/
# ref: https://stackoverflow.com/questions/8025766/makefile-auto-dependency-generation
# ref: http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
# update: 9/19/2021
# author: Bennett Cook
# desc: build from all *.cpp files in cwd, the "-MMD" option tells gcc to make a 
#       makefile fragment to compile *.cpp -> *.o with all dependencies. 
#       So if StateMachine.hpp changes main.o is rebuilt. (?? for project headers only)

# compiler for c++
CPP = g++

src = $(wildcard *.cpp)
obj = $(src:.cpp=.o)
dep = $(obj:.o=.d)  # one dependency file for each source

# compiler flags, all warnings and use c++2a libraries, and make dependency files
# the "CPPFLAGS" macro is automatically included in compile step (very confusing)
CPPFLAGS = -Wall -std=c++2a -MMD

LFLAGS = -L/usr/lib/arm-linux-gnueabihf -lwiringPi -lpthread -lboost_system -lboost_filesystem

# the executable to build
TARGET = coop

all: $(TARGET)  # first target so run by default if no command line args

debug: CPPFLAGS += -g
debug: $(TARGET)

# macros in recipe:  $@ -> target, $^ -> obj 
$(TARGET): $(obj)
	$(CPP) -o $@ $^ $(LFLAGS)

-include $(dep)   # include all dep files in the makefile

.PHONY: clean
clean:
	rm -f $(obj) $(TARGET) $(dep)
