CC = gcc
CFLAGS = -Wall
OFLAG ?= -O0
export OFLAG
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH

default: static test

test:
	./fm create_table 3 \
	search_directory /usr .c c_res \
	search_directory /usr .cpp cpp_res \
	search_directory /usr main main_res \
	remove_block 1 \
	search_directory /tmp file file_res \
	remove_block 2

define make-Otest
	$(eval export OFLAG=-O$(1))
	@echo TESTING O$(1) flag
	@make --no-print-directory clean
	@make --no-print-directory OFLAG=$(OFLAG) $(2)
	@make --no-print-directory test
endef

OFLAGS = 1 2 3

Otest-static:
	$(foreach i, $(OFLAGS), $(call make-Otest,$(i),static))

Otest-shared:
	$(foreach i, $(OFLAGS), $(call make-Otest,$(i),shared))

Otest-dynamic:
	$(foreach i, $(OFLAGS), $(call make-Otest,$(i),dynamic))

static: library_loader_static.o file_managing_library.o main.c
	$(CC) $(CFLAGS) $(OFLAG) -o fm library_loader_static.o file_managing_library.o main.c

shared: file_managing_library.so library_loader.so main.c
	$(CC) $(CFLAGS) $(OFLAG) -o fm main.c library_loader.so file_managing_library.so

dynamic: library_loader_dynamic.o file_managing_library.so main.c
	$(CC)) $(CFLAGS) $(OFLAG) -o fm main.c library_loader_dynamic.o -ldl

library_loader_static.o: library_loader.h library_loader_static.c
	$(CC) $(CFLAGS) $(OFLAG) -o library_loader_static.o -c library_loader_static.c

library_loader_dynamic.o: library_loader.h library_loader_dynamic.c
	$(CC) $(CFLAGS) $(OFLAG) -o library_loader_dynamic.o -c library_loader_dynamic.c

library_loader.so: library_loader.h library_loader_static.c
	$(CC) $(CFLAGS) $(OFLAG) -shared -fPIC -o library_loader.so library_loader_static.c

file_managing_library.o: file_managing_library.h file_managing_library.c
	$(CC) $(CFLAGS) $(OFLAG) -c file_managing_library.c

file_managing_library.so: file_managing_library.h file_managing_library.c
	$(CC) $(CFLAGS) $(OFLAG)-shared -fPIC -o file_managing_library.so file_managing_library.c

clean:
	rm -f *.o *.so fm
