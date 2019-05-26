#include <stdio.h>
#include <sys-ops-commons.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>

#define SEM_FLAGS 0666 | O_CREAT

#define RIDE "ride_sem79"
#define CARRIAGE "carriage_sem79"
#define CARRIAGE_FULL "carriagefull_sem79"


typedef struct {
    int id;
} passenger_data;

typedef struct {
    int id, C, N;
} carriage_data;

int W;
sem_t *ride_sem, *carriage_sem, *carriage_full_sem;

pthread_cond_t* order_cond;
pthread_mutex_t* order_mutex;
int* order;

pthread_cond_t* psngr_cond;
pthread_mutex_t* psngr_mutex;
int* psngr;

void* passenger(void *data) {
    passenger_data* pdata = (passenger_data*) data;
    while(1) {

    }
    return NULL;
}

void* carriage(void *data) {
    carriage_data* cdata = (carriage_data*) data;
    int n = cdata->N;
    int id = cdata->id;
    while(n--){
        //waiting for arrival
        printf("Carriage %d is waiting for arriving to platform\n", id);
        pthread_mutex_lock(&order_mutex[id]);
        while(!order[id])
            pthread_cond_wait(&order_cond[id], &order_mutex[id]);
        order[id] = 0;
        pthread_mutex_unlock(&order_mutex[id]);
        printf("Carriage %d is arriving to platform\n", id);

        //leaving

        //entering


        printf("Carriage %d is departuring from platform\n", id);
        //start riding
        int time = rand()%5;
        printf("Carriage %d is riding for %d seconds\n", id, time);
        sem_wait(ride_sem);
        //leaving
        order[(id+1)%W] = 1;
        pthread_cond_signal(&order_cond[(id+1)%W]);

        printf("Carriage %d is really riding\n", id);

        sleep(time);
        printf("Carriage %d has finished the ride\n", id);
        sem_post(ride_sem);
    }
    return NULL;
}

void init_sems(){
    if((ride_sem = sem_open(RIDE, SEM_FLAGS, 0666, 1)) == SEM_FAILED){
        show_error_and_exit("Unable to open semaphore", 1);
    }
    if((carriage_sem = sem_open(CARRIAGE, SEM_FLAGS)) == SEM_FAILED){
        show_error_and_exit("Unable to open semaphore", 1);
    }
    if((carriage_full_sem = sem_open(CARRIAGE_FULL, SEM_FLAGS)) == SEM_FAILED){
        show_error_and_exit("Unable to open semaphore", 1);
    }
}

void del_sems(){
    sem_close(ride_sem);
    sem_close(carriage_sem);
    sem_close(carriage_full_sem);
    sem_unlink(RIDE);
    sem_unlink(CARRIAGE_FULL);
    sem_unlink(CARRIAGE);
}

int main(int argc, char **argv) {
    validate_argc(argc, 4);
    int N, C, P;
    P = as_integer(argv[1]);
    W = as_integer(argv[2]);
    C = as_integer(argv[3]);
    N = as_integer(argv[4]);
    srand(time(0));
    init_sems();

    pthread_t pthreads[P];
    passenger_data pdata[P];
    pthread_t cthreads[W];
    carriage_data cdata[W];

//    for(int p=0; p<P; p++){
//        pdata[p].id = p;
//        pthread_create(&pthreads[p], NULL, passenger, &pdata[p]);
//    }

    order_cond = malloc(sizeof(pthread_cond_t)*W);
    order_mutex = malloc(sizeof(pthread_mutex_t)*W);
    order = malloc(sizeof(int)*W);

    for(int c=0; c<W; c++) {
        pthread_mutex_init(&order_mutex[c], NULL);
        pthread_cond_init(&order_cond[c], NULL);
    }

    for(int c=0; c<W; c++) {
        cdata[c].id = c;
        cdata[c].C = C;
        cdata[c].N = N;
        pthread_create(&cthreads[c], NULL, carriage, &cdata[c]);
    }

    order[0] = 1;
    pthread_cond_signal(&order_cond[0]);

    for(int c=0; c<W; c++) {
        pthread_join(cthreads[c], NULL);
    }

    del_sems();

    free(order_mutex);
    free(order_cond);
    free(order);

    return 0;
}