#include <stdio.h>
#include "file_managing_library.h"

const int TAB_SIZE = 10;

void print_files(char** files, int n){
    for(int i = 0; i < 1; i++) {
        if(files[i] != NULL)
            printf("%s", files[i]);
    }
}

int main(int argc, char** argv) {
    char** file_table = create_file_table(TAB_SIZE);
    s_file file = get_current_location();
    set_file_name(&file, "file1.txt");
    open_file(file_table, TAB_SIZE, &file);
    print_files(file_table, TAB_SIZE);
    return 0;
}