CC = gcc
CFLAGS = -Wall

test:
	./fm create_table 10 \
	search_directory /usr .c c_res \
	search_directory /usr .cpp cpp_res \
	search_directory /usr main main_res \
	remove_block 1 \
	search_directory /home file file_res \
	remove_block 2

static: file_managing_library.o
	$(CC) -o fm file_managing_library.o main.c

shared: file_managing_library.so
	$(CC) -o fm main.c file_managing_library.so

file_managing_library.so: file_managing_library.h
	$(CC) -shared -fPIC -o file_managing_library.so file_managing_library.c

file_managing_library.o: file_managing_library.h
	$(CC) -c file_managing_library.c