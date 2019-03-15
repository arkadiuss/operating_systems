//
// Created by arkadius on 15.03.19.
//

#ifndef INC_1_FUNCTIONS_COMPARISION_FILES_OPERATIONS_H
#define INC_1_FUNCTIONS_COMPARISION_FILES_OPERATIONS_H

int generate(char *file_name, int records_count, int record_size);
int sys_sort(char *file_name, int records_count, int record_size);
int lib_sort(char *file_name, int records_count, int record_size);
int sys_copy(char *file_name_src, char *file_name_dest, int records_count, int record_size);
int lib_copy(char *file_name_src, char *file_name_dest, int records_count, int record_size);
#endif //INC_1_FUNCTIONS_COMPARISION_FILES_OPERATIONS_H
