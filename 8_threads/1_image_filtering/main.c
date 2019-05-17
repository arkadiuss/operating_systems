#include <stdio.h>
#include <sys-ops-commons.h>
#include <string.h>
#include <stdlib.h>

typedef struct image {
    int w, h;
    unsigned char** pxls;
} image;

typedef struct filter {
    int c;
    float** pxls;
} filter;

image *src;
filter *flt;
int threads_count, divide_method;
char *out_file;

image* read_image(const char* file_name) {
    FILE* file;
    if((file = fopen(file_name, "r")) == NULL) {
        show_error_and_exit("Unable to open requested file", 1);
    }
    char format[2];
    fscanf(file, "%s", format);
    if(strcmp(format, "P2") != 0){
        show_error_and_exit("File isn't in pgm format", 1);
    }
    image* new_image = malloc(sizeof(image));
    fscanf(file, "%d", &new_image->w);
    fscanf(file, "%d", &new_image->h);
    int max_depth;
    fscanf(file, "%d", &max_depth);

    new_image->pxls = malloc(new_image->h*sizeof(char*));
    for(int i = 0; i<new_image->h; i++) {
        new_image->pxls[i] = malloc(new_image->w* sizeof(char));
        for(int j = 0; j<new_image->w; j++) {
            int val;
            fscanf(file, "%d", &val);
            new_image->pxls[i][j] =  (unsigned char) val;
        }
    }

    if(fclose(file) != 0){
        fprintf(stderr, "Unable to close file");
    }
    return new_image;
}

void write_to_file(image* image, const char *out_file) {
    FILE* file;
    if((file = fopen(out_file, "w")) == NULL) {
        show_error_and_exit("Unable to open requested file", 1);
    }
    fprintf(file, "P2\n%d %d\n255\n", image->w, image->h);
    for(int i = 0; i<image->h; i++) {
        for(int j = 0; j<image->w; j++) {
            fprintf(file, "%d ", image->pxls[i][j]);
        }
        fprintf(file, "\n");
    }
    if(fclose(file) != 0){
        fprintf(stderr, "Unable to close file");
    }
}

filter* read_filter(const char* file_name) {
    FILE* file;
    if((file = fopen(file_name, "r")) == NULL) {
        show_error_and_exit("Unable to open requested file", 1);
    }
    filter* new_flt = malloc(sizeof(filter));
    fscanf(file, "%d", &new_flt->c);
    new_flt->pxls = malloc(new_flt->c*sizeof(float*));
    for(int i = 0; i<new_flt->c; i++) {
        new_flt->pxls[i] = malloc(new_flt->c* sizeof(float));
        for(int j = 0; j<new_flt->c; j++) {
            float val;
            fscanf(file, "%f", &val);
            new_flt->pxls[i][j] = val;
        }
    }

    if(fclose(file) != 0){
        fprintf(stderr, "Unable to close file");
    }
    return new_flt;
}

void free_image(image* image){
    for(int i = 0; i<image->h; i++) {
        free(image->pxls[i]);
    }
    free(image->pxls);
    free(image);
}

void free_filter(filter* filter){
    for(int i = 0; i<filter->c; i++) {
        free(filter->pxls[i]);
    }
    free(filter->pxls);
    free(filter);
}

void read_arguments(char** argv){
    threads_count = as_integer(argv[1]);
    char *divide_method_str = argv[2];
    if(strcmp(divide_method_str, "block") == 0)
        divide_method = 0;
    else if(strcmp(divide_method_str, "interleaved") == 0)
        divide_method = 1;
    else
        show_error_and_exit("Second arguments should be block or interleaved", 1);
    src = read_image(argv[3]);
    flt = read_filter(argv[4]);
    out_file = argv[5];
}

int main(int argc, char** argv) {
    validate_argc(argc, 5);
    read_arguments(argv);
    write_to_file(src, out_file);
    free_image(src);
    free_filter(flt);
    return 0;
}