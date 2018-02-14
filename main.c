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


// Generate random number on normal curve
// Lock around number generation because drand48 is not thread safe
double randN(int mean) {
	pthread_mutex_init(&randLock, NULL);
	double x, y, res;
	pthread_mutex_lock(&randLock);
	x = drand48();
	y = drand48();
	pthread_mutex_unlock(&randLock);
	res = sqrt(-2 * log(x)) * cos(2 * M_PI * y);

	return (double)mean + res * ((double)mean / 2.0);
}

// Changes timeval struct to microseconds
long toUsec(struct timeval ts) {
	return ts.tv_sec * 1000000 +  ts.tv_usec;
}

// Changes timeval to seconds
float toSec(struct timeval ts) {
	return (float)ts.tv_sec + ts.tv_usec / 1000000.0;
}

void *Individual(void *args) {
	double arrivalTime, stayTime;
	individual_parameters *arg = (individual_parameters*)args;
    gender gen = arg->gen;


	// Allocate Timing Structs
    struct timeval *total_time =  malloc(sizeof(struct timeval));
    struct timeval *start_time = malloc(sizeof(struct timeval));
    struct timeval *temp2 = malloc(sizeof(struct timeval));
    struct timeval *temp3 = malloc(sizeof(struct timeval));
    struct timeval *min_time = NULL;
    struct timeval *max_time = NULL;

	// User goes through bathroom loop_count times
    for (int i = 0; i < arg->loop_count; i++) {	

		// Generates random arrival and wait time. If less than zero sets to 0
		// This does not maintain the normal distribution, but does not change it dramatically
		arrivalTime = randN(arg->mean_arrival_time);
		if (arrivalTime < 0) {
			arrivalTime = 0;
		}
	
		stayTime = randN(arg->mean_stay_time);
		if (stayTime < 0) {
			stayTime = 0;
		}

		// Timer start for user in bathroom
    	

		usleep(arrivalTime); // waits until user arrives at bathroom
		gettimeofday(start_time, NULL);	
		Enter(gen); // User can be here for an undefined amount of time waiting to enter
		gettimeofday(temp2, NULL); // End timer 
		usleep(stayTime); // User waits to leave bathroom

		Leave();


		

		// Add time in bathroom to total time for average calculation later
		timersub(temp2, start_time, temp3);
		timeradd(temp3, total_time, temp2);
		*total_time = *temp2;
		

		// Updates min_time timeval struct
		if (min_time == NULL) {
			min_time = malloc(sizeof(struct timeval));
			*min_time = *temp3;
		} else if (toUsec(*temp3) < toUsec(*min_time)) {
			
			*min_time = *temp3;
		} 
	

		// Updates max_time timeval struct
		if (max_time == NULL)  {
			 max_time = malloc(sizeof(struct timeval));
			 *max_time = *temp3;
		} else if (toUsec(*temp3) > toUsec(*max_time)) {
			
			*max_time = *temp3;
		} 


    }
	
	// Sets gender string
    char* gender = FEMALE;
    if (gen == male) {
    	gender = MALE;
    }

	// Calculates total time average
    total_time->tv_sec /= arg->loop_count;
    total_time->tv_usec /= arg->loop_count;

	// Prints Thread Statistics

    printf("Thread %d finished.\n\tGender: %s, \n\tLoop Count: %ds, \n\tMin Time: %.8fs, \n\tAvg Time: %.8fs, \n\tMax Time: %.8fs\n",
		(int)syscall(__NR_gettid), gender, arg->loop_count, toSec(*min_time), toSec(*total_time), toSec(*max_time));


    free(arg);

	return NULL;
}

int main(int argc, char* argv[]) {
	srand48(time(NULL));
	int num_users;
    individual_parameters base;
	pthread_attr_t *attr = (pthread_attr_t*)malloc(sizeof(pthread_attr_t));
	pthread_attr_setstacksize(attr, 50000);
	
	// Retrieves program arguments
	if (argc != 5) {
		printf("Invalid Number of Arguments.\n\tUsage: ./bathroomSim nUsers meanLoopCount meanArrival meanStay\n");
		printf("\tNote: meanArrival and meanStay are in microseconds\n");
		exit(1);
	} else {
		num_users = atoi(argv[1]);
		base.loop_count = atoi(argv[2]);
		base.mean_arrival_time = atoi(argv[3]);
		base.mean_stay_time = atoi(argv[4]);
	}

	if (base.loop_count < 1) {
		printf("Loop count must be greater than 0\n");
		exit(1);
	}
   	
	// Allocates memory for user threads
    pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t) * num_users);

    
	// Iniitalize Bathroom and create user threads
    Initialize();
	int i = 0;
    for (i = 0; i < num_users; i++) {
        pthread_t user_c;
		// Make new parameter struct to each user so that the data is not overwritten by the next loop
        individual_parameters *args = malloc(sizeof(individual_parameters)); 
        *args = base;
        args->gen =  drand48() > 0.5; // Randomly generated gender
		args->loop_count = (int)randN(base.loop_count);
		if (args->loop_count < 1) {
			args->loop_count = base.loop_count;
		}



        pthread_create(&user_c, attr, Individual, (void*)args); // Create new user thread
        threads[i] = user_c;
    }
	// Wait for threads to finish before finalizing
	 while (i > 0) {	
    	pthread_join(threads[--i], NULL);
    }
    Finalize();

	// Free threads and exit
    free(threads);
    return 0;
}
