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

static: library_loader.o file_managing_library.o
	$(CC) -o fm library_loader.o file_managing_library.o main.c

shared: file_managing_library.so library_loader.so
	$(CC) -o fm main.c file_managing_library.so library_loader.so

dynamic: library_loader_dynamic.o file_managing_library.so
	$(CC) -o fm main.c library_loader_dynamic.o -ldl

library_loader.so: library_loader.h
	$(CC) -shared -fPIC -o library_loader.so library_loader_static.c

file_managing_library.so: file_managing_library.h
	$(CC) -shared -fPIC -o file_managing_library.so file_managing_library.c

library_loader.o: library_loader.h
	$(CC) -o library_loader.o -c library_loader_static.c

library_loader_dynamic.o: library_loader.h
	$(CC) -o library_loader_dynamic.o -c library_loader_dynamic.c

file_managing_library.o: file_managing_library.h
	$(CC) -c file_managing_library.c