//
// Created by Sigpit on 2/5/2018.
//
#include "bathroom.h"
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/time.h>


#define MAX_IN_BATHROOM 3
#define WAIT_TIME 10

typedef enum __state {m, f, v} state;
typedef enum __isOpen  {open, closed} open_state;

typedef struct __bathroom {
    size_t num_in_bathroom;

    // Statistics
    size_t total_served;
    size_t waiting_female; // Essentially the queue
    size_t waiting_male;
    size_t queue_size_sum;
    size_t sum_in_bathroom;
    size_t num_data_points;

    // Timing Variables
    struct timeval total_time_vacant;
    struct timeval start_time_vacant;
    struct timeval total_time_occupied;
    struct timeval start_time_occupied;
    struct timeval temp;
    struct timeval temp2;

    state state;
    open_state state2;

    pthread_mutex_t stateLock;
    pthread_mutex_t femaleLock;
    pthread_mutex_t maleLock;
    pthread_mutex_t bothLockT;

    pthread_cond_t maleEnter;
    pthread_cond_t femaleEnter;

    sem_t maleDoor;
    sem_t femaleDoor;
    sem_t bothLock;





} bathroom;

bathroom *theBathroom;

bathroom *allocBathroom() {
    bathroom *bath = (bathroom *) malloc(sizeof(bathroom));
    bath->num_in_bathroom = 0;
    bath->waiting_female = 0;
    bath->total_served = 0;
    bath->waiting_male = 0;
    bath->queue_size_sum = 0;
    bath->sum_in_bathroom = 0;
    bath->num_data_points = 0;

    bath->state = v;
    bath->state2 = open;

    timerclear(&bath->total_time_vacant);
    timerclear(&bath->start_time_vacant);
    timerclear(&bath->total_time_occupied);
    timerclear(&bath->start_time_occupied);
    timerclear(&bath->temp);
    timerclear(&bath->temp2);

    sem_init(&bath->maleDoor, 0, 1);
    sem_init(&bath->femaleDoor, 0, 1);
    sem_init(&bath->bothLock, 0, 1);

    pthread_mutex_init(&bath->stateLock, NULL);
    pthread_mutex_init(&bath->maleLock, NULL);
    pthread_mutex_init(&bath->femaleLock, NULL);
    pthread_mutex_init(&bath->bothLockT, NULL);


    pthread_cond_init(&bath->maleEnter, NULL);
    pthread_cond_init(&bath->femaleEnter, NULL);




    return bath;

}


void printToLog(char eventName[]) {

    printf("%s\n", eventName );
}

// Can only run within bathroom stateLock
void updateStats() {
	theBathroom->num_data_points++;
	theBathroom->queue_size_sum += theBathroom->waiting_male + theBathroom->waiting_female;
	theBathroom->sum_in_bathroom += theBathroom->num_in_bathroom;
}


void user(gender g) {
    gender gen = g;
    pthread_mutex_lock(&theBathroom->stateLock);
    printf("\tGen: %d\n", gen);
    state s = theBathroom->state;
    gettimeofday(&theBathroom->start_time_occupied, NULL);
	timersub(&theBathroom->start_time_occupied, &theBathroom->start_time_vacant,
			 &theBathroom->temp);
	theBathroom->temp2 = theBathroom->total_time_vacant;
	timeradd(&theBathroom->temp2, &theBathroom->temp, &theBathroom->total_time_vacant);

    if (gen == male) {
    	if (s == v) {
    		theBathroom->state = m;

    	} else if (s == m) {
    	} else {
    		theBathroom->waiting_male++;
    		pthread_cond_wait(&theBathroom->maleEnter, &theBathroom->stateLock);
    		theBathroom->waiting_male--;
    	}
    } else if (gen == female) {
    	if (s == v) {
    		theBathroom->state = f;
    		pthread_cond_signal(&theBathroom->femaleEnter);
    	} else if (s == f) {
    		pthread_cond_signal(&theBathroom->femaleEnter);
    	} else {
    		theBathroom->waiting_female++;
    		pthread_cond_wait(&theBathroom->femaleEnter, &theBathroom->stateLock);
    		theBathroom->waiting_female--;
    	}
    }
    theBathroom->num_in_bathroom++;
    pthread_mutex_unlock(&theBathroom->stateLock);

//        gettimeofday(&theBathroom->start_time_occupied, NULL);
//        timersub(&theBathroom->start_time_occupied, &theBathroom->start_time_vacant,
//                 &theBathroom->temp);
//        theBathroom->temp2 = theBathroom->total_time_vacant;
//        timeradd(&theBathroom->temp2, &theBathroom->temp, &theBathroom->total_time_vacant);


    printf("\tExit Enter: MenW: %d, FemW: %d, Curr: %d, NumInBathroom: %d\n",
    		theBathroom->waiting_male,
			theBathroom->waiting_female,
			gen,
			theBathroom->num_in_bathroom);
}




void Leave(void) {
//	printf("before lock\n");
    pthread_mutex_lock(&theBathroom->stateLock);
    theBathroom->num_in_bathroom--;
    theBathroom->total_served++;
//    printf("In Leave\n");
    if (theBathroom->num_in_bathroom == 0) {

        gettimeofday(&theBathroom->start_time_vacant, NULL);
        timersub(&theBathroom->start_time_vacant, &theBathroom->start_time_occupied,
                 &theBathroom->temp);
        theBathroom->temp2 = theBathroom->total_time_occupied;
        timeradd(&theBathroom->temp2, &theBathroom->temp, &theBathroom->total_time_occupied);



        printf("Vacant\n");

        if (theBathroom->waiting_male == 0 && theBathroom->waiting_female == 0) {
        	 theBathroom->state = v;
    	} else if (theBathroom->waiting_male > theBathroom->waiting_female) {
        	theBathroom->state = m;
        	pthread_cond_signal(&theBathroom->maleEnter);
        } else {
        	theBathroom->state = f;
        	pthread_cond_signal(&theBathroom->femaleEnter);
        }
//        printf("After Broadcasts\n");
    }
    pthread_mutex_unlock(&theBathroom->stateLock);
//    printf("\tExit Leave: MenW: %d, FemW: %d, NumInBathroom: %d\n",
//        		theBathroom->waiting_male,
//    			theBathroom->waiting_female,
//    			theBathroom->num_in_bathroom);
}

void Enter(gender g) {

    user(g);

}

void Initialize(void) {

    theBathroom = allocBathroom();
    gettimeofday(&theBathroom->start_time_vacant, NULL);
}

void Finalize(void){
    printf("Statistics: \n");
    printf("Number of Users: %d\n", (unsigned)theBathroom->total_served);
    printf("Vacancy Time: %d\n", (int)theBathroom->total_time_vacant.tv_sec +
            (int)theBathroom->total_time_vacant.tv_usec / 1000000 );
    printf("Occupied Time: %d\n",  (int)theBathroom->total_time_occupied.tv_sec +
                                   (int)theBathroom->total_time_occupied.tv_usec / 1000000);
    printf("Average Queue Length: %d\n", theBathroom->sum_in_bathroom / theBathroom->num_data_points);
    printf("Average Number of Concurrent Users: %d\n", theBathroom->queue_size_sum / theBathroom->num_data_points);
}

