//
// Created by Sigpit on 2/5/2018.
//
#include "bathroom.h"
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

#define MAX_IN_BATHROOM 3

typedef enum __state {m, f, v} state;

typedef struct __bathroom {
    size_t num_in_bathroom;
    size_t total;
    size_t total_served;
    size_t waiting_female;
    size_t waiting_male;

    int isOpen;

    state state;
    pthread_mutex_t stateLock;
    pthread_mutex_t roomLock;
    pthread_cond_t menEnter;
    pthread_cond_t womenEnter;



} bathroom;

bathroom *theBathroom;

bathroom *allocBathroom() {
    bathroom *bath = (bathroom *) malloc(sizeof(bathroom));
    bath->num_in_bathroom = 0;
    bath->waiting_female = 0;
    bath->waiting_male = 0;
    bath->state = v;
    bath->isOpen = 1;
    pthread_mutex_init(&bath->stateLock, NULL);
    pthread_mutex_init(&bath->roomLock, NULL);
    pthread_cond_init(&bath->menEnter, NULL);
    pthread_cond_init(&bath->womenEnter, NULL);

    return bath;

}


void printToLog(char eventName[]) {

    printf("%s\n", eventName );
}


void user(gender g) {
    gender gen = g;
    pthread_mutex_lock(&theBathroom->stateLock);
    state s = theBathroom->state;
    if (gen == male && s == f || !theBathroom->isOpen) {
        theBathroom->waiting_male++;
        pthread_cond_wait(&theBathroom->menEnter, &theBathroom->stateLock);
        s = v;
    } else if (gen == female && s == m || !theBathroom->isOpen) {
        theBathroom->waiting_female++;
        pthread_cond_wait(&theBathroom->womenEnter, &theBathroom->stateLock);
        s = v;
    }

    if (theBathroom->num_in_bathroom > MAX_IN_BATHROOM)
        theBathroom->isOpen = 0;

    if (s == v) {
        if (gen == male) {
            theBathroom->state = m;
            theBathroom->num_in_bathroom++;
            theBathroom->waiting_male--;
            pthread_mutex_unlock(&theBathroom->stateLock);
            sleep((unsigned)rand() % 3);
            printf("Vacant Male Done\n");
            Leave();
        } else if (gen == female) {
            theBathroom->state = f;
            theBathroom->num_in_bathroom++;
            theBathroom->waiting_female--;
            pthread_mutex_unlock(&theBathroom->stateLock);
            sleep((unsigned)rand() % 3);
            printf("Vacant Female Done\n");
            Leave();
        }
    } else if (s == m) {
        theBathroom->num_in_bathroom++;
        theBathroom->waiting_male--;
        pthread_mutex_unlock(&theBathroom->stateLock);
        sleep((unsigned)rand() % 3);
        printf("Male only done\n");
        Leave();
    } else if (s == f) {
        theBathroom->num_in_bathroom++;
        theBathroom->waiting_female--;
        pthread_mutex_unlock(&theBathroom->stateLock);
        sleep((unsigned) rand() % 3);
        printf("Female only Done\n");
        Leave();
    }
}


void Enter(gender g) {
    user(g);

}

void Leave(void) {
    pthread_mutex_lock(&theBathroom->stateLock);
    theBathroom->num_in_bathroom--;
    theBathroom->total_served++;
    if (theBathroom->num_in_bathroom == 0) {
        printf("Vacant\n");
        theBathroom->isOpen = 1;


        if (theBathroom->waiting_male > theBathroom->waiting_female) {
            pthread_cond_broadcast(&theBathroom->menEnter);
        } else {
            pthread_cond_broadcast(&theBathroom->womenEnter);
        }
        theBathroom->state = v;
    }
    pthread_mutex_unlock(&theBathroom->stateLock);

}

void Initialize(void) {
    srand(time(NULL));
    theBathroom = allocBathroom();

}

void Finalize(void){

}

