#include <stdio.h>
#include <sys-ops-commons.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>

typedef struct image {
    int w, h;
    unsigned char** pxls;
} image;

typedef struct filter {
    int c;
    float** pxls;
} filter;

image *src, *dst;
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

void init_out_image(){
    dst = malloc(sizeof(image));
    dst->h = src->h;
    dst->w = src->w;
    dst->pxls = calloc(sizeof(float*), (size_t) dst->h);
    for(int i = 0; i<dst->h; i++) {
        dst->pxls[i] = calloc(sizeof(float), (size_t) dst->w);
    }
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

typedef struct thread_data {
    int k, m, t;
} thread_data;

void count_dest(int x, int y){
    float sum = 0;
    for(int i = 0; i < flt->c; i++) {
        for(int j = 0; j < flt->c; j++) {
            int indi = max(0, min(src->h-1, x - (int) ceil(flt->c/2) + i));
            int indj = max(0, min(src->w-1, y - (int) ceil(flt->c/2) + j));
            sum += src->pxls[indi][indj] * flt->pxls[i][j];
        }
    }
    dst->pxls[x][y] = max(0, min(255.,round(sum)));
}

long long get_timestamp(){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return 1000000LL * tv.tv_sec + tv.tv_usec;
}

void* process_image(void* data) {
    //TODO: time measurement
    thread_data* td = (thread_data*) data;
    int k = td->k;
    int m = td->m;
    long long t1 = get_timestamp();
    if(td->t == 0)
        for(int j = (k-1)*src->w/m; j < k*src->w/m; j++) {
            for (int i = 0; i < src->h; i++)
                count_dest(i, j);
        }
    else if(td->t == 1)
        for(int j = k-1; j < src->w; j += m) {
            for (int i = 0; i < src->h; i++)
                count_dest(i, j);
        }
    long long t2 = get_timestamp();
    return (void*) (t2 - t1);
}

int main(int argc, char** argv) {
    validate_argc(argc, 5);
    read_arguments(argv);
    init_out_image();

    long long t1 = get_timestamp();
    pthread_t threads[threads_count];
    thread_data data[threads_count];
    for(int i = 0; i < threads_count; i++) {
        data[i].k = i + 1;
        data[i].m = threads_count;
        data[i].t = divide_method;
        pthread_create(&threads[i], NULL, process_image, &data[i]);
    }

    for(int i = 0; i < threads_count; i++) {
        long result;
        pthread_join(threads[i], (void*) &result);
        printf("Thread %d finished after %lld us\n", i + 1, (long long) result);
    }
    long long t2 = get_timestamp();
    printf("The whole process took %lld us\n", (t2 - t1));

    write_to_file(dst, out_file);
    free_image(src);
    free_image(dst);
    free_filter(flt);
    return 0;
}