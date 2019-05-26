#include <stdio.h>
#include <sys-ops-commons.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <string.h>
#include <errno.h>

#define SEM_FLAGS 0666 | O_CREAT
#define QUEUE_FLAGS O_RDWR | O_CREAT

#define RIDE "/ride_sem4999"
#define CARRIAGE "/carriage_sem4999"

#define ENTER "/enter_queue4999"


typedef struct {
    int id;
} passenger_data;

typedef struct {
    int id, C, N;
} carriage_data;

int W;
sem_t *ride_sem, *carriage_sem;
mqd_t enter_queue;

pthread_cond_t* order_cond;
pthread_mutex_t* order_mutex;
int* order;

pthread_cond_t* psngr_cond;
pthread_mutex_t* psngr_mutex;
int* psngr;

void* passenger(void *data) {
    passenger_data* pdata = (passenger_data*) data;
    int id = pdata->id;
    char strid[10];
    sprintf(strid, "%d", id);
    while(1) {
        printf("Passenger %d is waiting for carriage\n", id);
        sem_wait(carriage_sem);
        printf("Passenger %d has entered to carriage\n", id);

        mq_send(enter_queue, strid, 10, 1);
        pthread_mutex_lock(&psngr_mutex[id]);
        while(!psngr[id])
            pthread_cond_wait(&psngr_cond[id], &psngr_mutex[id]);
        psngr[id] = 0;
        printf("Passenger %d has left carriage\n", id);
        pthread_mutex_unlock(&psngr_mutex[id]);
        sem_post(carriage_sem);
    }
    return NULL;
}

void* carriage(void *data) {
    carriage_data* cdata = (carriage_data*) data;
    int n = cdata->N;
    int id = cdata->id;
    int C = cdata->C;

    int cpsngs[C];
    for(int c=0;c<C;c++)
        cpsngs[c] = -1;

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
        for(int c=0;c<C;c++) {
            if(cpsngs[c] != -1){
                psngr[cpsngs[c]] = 1;
                pthread_cond_signal(&psngr_cond[cpsngs[c]]);
            } else {
                sem_post(carriage_sem);
            }
        }

        //entering
        for(int c=0;c<C;c++) {
            char pid[10];
            if(mq_receive(enter_queue, pid, 10, NULL) == -1){
                printf("Error while receiving: %s\n", strerror(errno));
            }
            cpsngs[c] = atoi(pid);
            printf("Carriage %d filled in %d/%d\n", id, c+1, C);
        }
        printf("Passenger %d has clicked the start button\n", cpsngs[C-1]);
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
    //TODO: leaving at the end
    return NULL;
}

void init(){
    if((ride_sem = sem_open(RIDE, SEM_FLAGS, 0666, 1)) == SEM_FAILED){
        show_error_and_exit("Unable to open semaphore", 1);
    }
    if((carriage_sem = sem_open(CARRIAGE, SEM_FLAGS, 0666, 0)) == SEM_FAILED){
        sem_unlink(RIDE);
        show_error_and_exit("Unable to open semaphore", 1);
    }

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 10;


    if((enter_queue = mq_open(ENTER, QUEUE_FLAGS, 0666, &attr)) == -1) {
        sem_unlink(RIDE);
        sem_unlink(CARRIAGE);
        show_error_and_exit("Unable to open queue", 1);
    }

}

void cleanup(){
    sem_close(ride_sem);
    sem_close(carriage_sem);
    sem_unlink(RIDE);
    sem_unlink(CARRIAGE);
    mq_close(enter_queue);
    mq_unlink(ENTER);
}

int main(int argc, char **argv) {
    validate_argc(argc, 4);
    int N, C, P;
    P = as_integer(argv[1]);
    W = as_integer(argv[2]);
    C = as_integer(argv[3]);
    N = as_integer(argv[4]);
    srand(time(0));
    init();

    // passengers
    pthread_t pthreads[P];
    passenger_data pdata[P];

    psngr_cond = malloc(sizeof(pthread_cond_t)*P);
    psngr_mutex = malloc(sizeof(pthread_mutex_t)*P);
    psngr = malloc(sizeof(int)*P);

    for(int p=0; p<P; p++){
        pthread_mutex_init(&psngr_mutex[p], NULL);
        pthread_cond_init(&psngr_cond[p], NULL);
    }

    for(int p=0; p<P; p++){
        pdata[p].id = p;
        pthread_create(&pthreads[p], NULL, passenger, &pdata[p]);
    }

    // carriages
    pthread_t cthreads[W];
    carriage_data cdata[W];

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

    cleanup();

    free(order_mutex);
    free(order_cond);
    free(order);

    return 0;
}