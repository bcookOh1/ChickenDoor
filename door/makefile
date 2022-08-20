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

# compiler flags, all warnings and use (c++2a) c++17 libraries, and make dependency files
# the "CPPFLAGS" macro is automatically included in compile step (very confusing)
# note: this version of boost interprocess works with c++17 but not c++2a.
# -DBOOST_BIND_GLOBAL_PLACEHOLDERS
CPPFLAGS = -Wall -std=c++2a -MMD -fpermissive -DBOOST_BIND_GLOBAL_PLACEHOLDERS

LFLAGS = -L/usr/lib/arm-linux-gnueabihf -lsqlite3 -lwiringPi -lpthread -lstdc++fs -lboost_system $\
         -lboost_date_time -lboost_coroutine -lboost_context -lrt
# removed -lboost_filesystem, use std::filesystem linked with -lstdc++fs 

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
	rm -f *.o $(TARGET) *.d
