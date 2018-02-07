#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "bathroom.h"


void *callback(void *g) {
    gender gen = *(gender*)g;
    Enter(gen);

}

int main() {
    printf("Hello, World!\n");
    pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t)*100);
    int index = 0, i = 0;
    gender g[] = {male, female, male, female, male, male, male, female, female, female, male, male};
    Initialize();
//    gender g[] = {male, male, male, male, male, male};
    for (i = 0; i < 12; i++) {
        pthread_t user_c;
        pthread_create(&user_c, NULL, callback, &g[i]);
        threads[i] = user_c;
    }
    for (; i >= 0; i-- ) {
        pthread_join(threads[i], NULL);
    }
    Finalize();
    return 0;
}