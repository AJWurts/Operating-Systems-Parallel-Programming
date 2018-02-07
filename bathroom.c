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
#define WAIT_TIME 4

typedef enum __state {m, f, v} state;

typedef struct __bathroom {
    size_t num_in_bathroom;

    // Statistics
    size_t total_served;
    size_t waiting_female; // Essentially the queue
    size_t waiting_male;

    // Timing Variables
    struct timeval total_time_vacant;
    struct timeval start_time_vacant;
    struct timeval total_time_occupied;
    struct timeval start_time_occupied;
    struct timeval temp;
    struct timeval temp2;

    state state;
    pthread_mutex_t stateLock;

    sem_t maleDoor;
    sem_t femaleDoor;





} bathroom;

bathroom *theBathroom;

bathroom *allocBathroom() {
    bathroom *bath = (bathroom *) malloc(sizeof(bathroom));
    bath->num_in_bathroom = 0;
    bath->waiting_female = 0;
    bath->total_served = 0;
    bath->waiting_male = 0;
    bath->state = v;

    timerclear(&bath->total_time_vacant);
    timerclear(&bath->start_time_vacant);
    timerclear(&bath->total_time_occupied);
    timerclear(&bath->start_time_occupied);
    timerclear(&bath->temp);
    timerclear(&bath->temp2);

    sem_init(&bath->maleDoor, 0, 1);
    sem_init(&bath->femaleDoor, 0, 1);

    pthread_mutex_init(&bath->stateLock, NULL);


    return bath;

}


void printToLog(char eventName[]) {

    printf("%s\n", eventName );
}

// Can only run within bathroom stateLock
void updateStats() {


}


void user(gender g) {
    gender gen = g;
    pthread_mutex_lock(&theBathroom->stateLock);
    state s = theBathroom->state;


    if (s == v) {

        gettimeofday(&theBathroom->start_time_occupied, NULL);
        timersub(&theBathroom->start_time_occupied, &theBathroom->start_time_vacant,
                 &theBathroom->temp);
        theBathroom->temp2 = theBathroom->total_time_vacant;
        timeradd(&theBathroom->temp2, &theBathroom->temp, &theBathroom->total_time_vacant);

        if (gen == male) {
            theBathroom->state = m;
            theBathroom->num_in_bathroom++;
            sem_wait(&theBathroom->femaleDoor); // lock female door
            pthread_mutex_unlock(&theBathroom->stateLock);

        } else if (gen == female) {
            theBathroom->state = f;
            theBathroom->num_in_bathroom++;
            sem_wait(&theBathroom->maleDoor);
            pthread_mutex_unlock(&theBathroom->stateLock);

        }
    } else if (gen == male) {
        theBathroom->waiting_male++;
        pthread_mutex_unlock(&theBathroom->stateLock);
        sem_wait(&theBathroom->maleDoor); // wait for male door to be open
        sem_post(&theBathroom->maleDoor); // signal other waiting men
        pthread_mutex_lock(&theBathroom->stateLock);
        theBathroom->state = m;
        theBathroom->waiting_male--;
        theBathroom->num_in_bathroom++;
        pthread_mutex_unlock(&theBathroom->stateLock);

    } else if (gen == female) {
        theBathroom->waiting_female++;
        pthread_mutex_unlock(&theBathroom->stateLock);
        sem_wait(&theBathroom->femaleDoor);
        sem_post(&theBathroom->femaleDoor);
        pthread_mutex_lock(&theBathroom->stateLock);
        theBathroom->state = f;
        theBathroom->waiting_female--;
        theBathroom->num_in_bathroom++;
        pthread_mutex_unlock(&theBathroom->stateLock);


    }

    sleep((unsigned) rand() % WAIT_TIME);
    printf("%d: %d done\n", gettid(), gen);
    Leave();
}




void Leave(void) {
    pthread_mutex_lock(&theBathroom->stateLock);
    theBathroom->num_in_bathroom--;
    theBathroom->total_served++;
    if (theBathroom->num_in_bathroom == 0) {

        gettimeofday(&theBathroom->start_time_vacant, NULL);
        timersub(&theBathroom->start_time_vacant, &theBathroom->start_time_occupied,
                 &theBathroom->temp);
        theBathroom->temp2 = theBathroom->total_time_occupied;
        timeradd(&theBathroom->temp2, &theBathroom->temp, &theBathroom->total_time_occupied);

        printf("Vacant\n");
        theBathroom->state = v;

        if (theBathroom->waiting_male > theBathroom->waiting_female) {
            sem_post(&theBathroom->maleDoor);
        } else {
            sem_post(&theBathroom->femaleDoor);
        }

    }
    pthread_mutex_unlock(&theBathroom->stateLock);

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
    printf("Average Queue Length: %d\n", 0);
    printf("Average Number of Concurrent Users: %d\n", 0);
}

