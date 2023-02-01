CC=g++ -g3 -Wall -std=c++17

# List of source files for your thread library
THREAD_SOURCES=sentinelq.cpp mycpu.cpp mythread.cpp mymutex.cpp mycv.cpp

# Generate the names of the thread library's object files
THREAD_OBJS=${THREAD_SOURCES:.cpp=.o}

all: libthread.o app

# Compile the thread library and tag this compilation
libthread.o: ${THREAD_OBJS}
	./autotag.sh
	ld -r -o $@ ${THREAD_OBJS}

# Compile an application program
app: main_hzin.cpp libthread.o libcpu.o
	${CC} -o $@ $^ -ldl -pthread

# Test cases 
test%: test_%.cpp libthread.o libcpu.o
	${CC} -o $@ $^ -ldl -pthread

context: main_context.cpp
	${CC} -o $@ $^

sentinelq: main_sentinelq.cpp sentinelq.cpp
	${CC} -o $@ $^

# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} -c $<
%.o: %.cc
	${CC} -c $<

clean:
	rm -f ${THREAD_OBJS} libthread.o app context sentinelq
