CC=g++ -g -Wall -std=c++17

# List of source files for your file server
FS_SOURCES= file_system_hzin.cpp file_system_yz.cpp create_file2.cpp delete_file2.cpp

# Generate the names of the file server's object files
FS_OBJS=${FS_SOURCES:.cpp=.o}

all: fs test1 test2 test3 test4 test5 test6 test6fs test7 test8 test9 test10 test11 test12 test13 test14 test15 test17 test18 test19 test20 test21 test22 app testt1 testt2

# Compile the file server and tag this compilation
fs: fs_server.cpp libfs_server.o file_system_hzin.cpp file_system_yz.cpp create_file2.cpp delete_file2.cpp
	./autotag.sh
	${CC} -o $@ $^ -pthread -ldl

app: ${FS_OBJS} app.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test1: ${FS_OBJS} test1.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test2: ${FS_OBJS} test2.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test2.2: ${FS_OBJS} test2.2.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test3: ${FS_OBJS} test3.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test4: ${FS_OBJS} test4.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test5: ${FS_OBJS} test5.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test6: ${FS_OBJS} test6.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test6fs: ${FS_OBJS} test6fs.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test7: ${FS_OBJS} test7.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test8: ${FS_OBJS} test8.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test9: ${FS_OBJS} test9.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test10: ${FS_OBJS} test10.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test11: ${FS_OBJS} test11.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test12: ${FS_OBJS} test12.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test13: ${FS_OBJS} test13.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test14: ${FS_OBJS} test14.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test15: ${FS_OBJS} test15.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test17: ${FS_OBJS} test17.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test18: ${FS_OBJS} test18.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test19: ${FS_OBJS} test19.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test20: ${FS_OBJS} test20.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test21: ${FS_OBJS} test21.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

test22: ${FS_OBJS} test22.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

testt1: ${FS_OBJS} test_temp1.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

testt2: ${FS_OBJS} test_temp2.cpp libfs_client.o libfs_server.o
	${CC} -o $@ $^ -pthread -ldl

# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} -c $<
%.o: %.cc
	${CC} -c $<

clean:
	rm -f ${FS_OBJS} fs test1 test2 test3 test4 test5 test6fs test7 test8 test9 test_disk test2.2 test11 test12 test13 test6 test14 test15 test16 test17 test18 test19 test20 test21 test22
