// By Alexander Wurts

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include "bathroom.h"


#define WAIT_TIME 10

//pthread_t printlock;
pthread_mutex_t randLock;
char *MALE = "male";
char *FEMALE = "female";

typedef struct _individual_struct {
	gender gen;
	int mean_arrival_time;
	int mean_stay_time;
	int loop_count;
} individual_parameters;

double randN() {
	pthread_mutex_init(&randLock, NULL);
	double x, y, res;
	pthread_mutex_lock(&randLock);
	x = drand48();
	y = drand48();
//	printf("%f, %f\n", x, y);
	pthread_mutex_unlock(&randLock);
	res = sqrt(-2 * log(x)) * cos(2 * M_PI * y);

	return res;
}

int toUsec(struct timeval ts) {
	return ts.tv_sec * 1000000 +  ts.tv_usec;
}

float toSec(struct timeval ts) {
	return ts.tv_sec + ts.tv_usec / 1000000.0;
}

void *Individual(void *args) {
	individual_parameters *arg = (individual_parameters*)args;
    gender gen = arg->gen;

    struct timeval *total_time =  malloc(sizeof(struct timeval));
    struct timeval *start_time = malloc(sizeof(struct timeval));
    struct timeval *temp2 = malloc(sizeof(struct timeval));
    struct timeval *temp3 = malloc(sizeof(struct timeval));
    struct timeval *min_time = NULL;
    struct timeval *max_time = NULL;

    for (int i = 0; i < arg->loop_count; i++) {
    	gettimeofday(start_time, NULL);
		double rand = arg->mean_arrival_time + randN() * (arg->mean_arrival_time / 2);
		if (rand < 0) {
			rand = arg->mean_arrival_time;
		}
		usleep(rand);//(int)rand);

		Enter(gen);
		rand = arg->mean_stay_time +  randN() * (arg->mean_stay_time / 2);
		if (rand < 0) {
			rand = arg->mean_stay_time;
		}
		usleep(rand);//rand);
		Leave();


		gettimeofday(temp2, NULL);
		timersub(temp2,start_time, temp3);
		timeradd(temp3, total_time, temp2);
		*total_time = *temp2;
		if (min_time == NULL) {
			min_time = malloc(sizeof(struct timeval));
			*min_time = *temp3;
		} else if (toUsec(*temp3) < toUsec(*min_time)) {
			*min_time = *temp3;
		}

		if (max_time == NULL)  {
			 max_time = malloc(sizeof(struct timeval));
			 *max_time = *temp3;
		} else if (toUsec(*temp3) < toUsec(*max_time)) {
			*max_time = *temp3;
		}


    }
    char* gender = FEMALE;
    if (gen == male) {
    	gender = MALE;
    }

    total_time->tv_sec /= arg->loop_count;
    total_time->tv_usec /= arg->loop_count;

    printf("Thread %d finished. Gender: %s, Min Time: %f, Avg Time: %f, Max Time: %f\n",
		(int)syscall(__NR_gettid), gender, toSec(*min_time), toSec(*total_time), toSec(*max_time));
//    free(max_time);
//    free(min_time);
//    free(arg);
	return NULL;
}

int main() {
	srand48(time(NULL));
    printf("Hello, World!\n");
    individual_parameters base;
    base.mean_arrival_time = 2000;
    base.mean_stay_time = 20000;
    base.loop_count = 5;
    pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t)*100);
    int i = 0;
    Initialize();
    for (i = 0; i < 100; i++) {
        pthread_t user_c;
        individual_parameters *args = malloc(sizeof(individual_parameters));
        *args = base;
        args->gen =  drand48() > 0.5;

        pthread_create(&user_c, NULL, Individual, (void*)args);
        threads[i] = user_c;
    }
    for (; i >= 0; i-- ) {
        pthread_join(threads[i], NULL);
    }
    Finalize();

    free(threads);
    return 0;
}
