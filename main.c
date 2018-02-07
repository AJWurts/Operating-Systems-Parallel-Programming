// By Alexander Wurts

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "bathroom.h"

#define WAIT_TIME 10

pthread_t printlock;

typedef struct _individual_struct {
	gender gen;
	int mean_arrival_time;
	int mean_stay_time;
	int loop_count;
} individual_parameters;

float randN() {
	pthread_mutex_t lock;
	pthread_mutex_init(&lock, NULL);
	float x, y, res;
	pthread_mutex_lock(&lock);
	x = drand48();
	y = drand48();
	res = sqrt(-2 * log(x)) * cos(2 * M_PI * y);
	pthread_mutex_unlock(&lock);
	return 1 + res;
}


void *Individual(void *args) {
	individual_parameters *arg = (individual_parameters*)args;
    gender gen = arg->gen;
    float rand = randN() * arg->mean_arrival_time;
    printf("Rand1: %f", rand);
    usleep((int)rand);

    Enter(gen);
    rand = randN() * arg->mean_stay_time;
    usleep(rand);
    Leave();
    printf("%d done\n", gen);
    return NULL;
}

int main() {
	srand48(time(NULL));
    printf("Hello, World!\n");
    individual_parameters base;
    base.mean_arrival_time = 10;
    base.mean_stay_time = 20;
    base.loop_count = 5;
    pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t)*100);
    int i = 0;
    gender g[] = {male, female, male, female, male, male, male, female, female, female, male, male};
    Initialize();
//    gender g[] = {male, male, male, male, male, male};
    for (i = 0; i < 1; i++) {
        pthread_t user_c;
        individual_parameters args = base;
        args.gen = g[i];
        pthread_create(&user_c, NULL, Individual, (void*)&args);
        threads[i] = user_c;
    }
    for (; i >= 0; i-- ) {
        pthread_join(threads[i], NULL);
    }
    Finalize();
    return 0;
}
