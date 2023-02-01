CC=g++ -g -Wall -fno-builtin -std=c++17

# List of source files for your pager
PAGER_SOURCES=vm_pager.cpp memory_manager.cpp

# Generate the names of the pager's object files
PAGER_OBJS=${PAGER_SOURCES:.cpp=.o}

all: pager app test1.4 test2.4 test3.4 test4.4 test5.4 test6.4 test7.4 test8.4 test9.4 test10.4 test11.4 test12.4 test13.4 test22.4

# Compile the pager and tag this compilation
pager: ${PAGER_OBJS} libvm_pager.o
	./autotag.sh
	${CC} -o $@ $^

# Compile an application program
app: app.cpp libvm_app.o
	${CC} -o $@ $^ -ldl

# Test cases 
test%: test%.cpp libvm_app.o
	${CC} -o $@ $^ -ldl -pthread

# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} -c $<
%.o: %.cc
	${CC} -c $<

clean:
	rm -f ${PAGER_OBJS} pager app test1.4 test2.4 test3.4 test4.4 test5.4 test6.4 test7.4 test8.4 test9.4 test10.4 test11.4 test12.4 test22.4
