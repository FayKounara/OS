#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define Ncook 2
#define Noven 10
#define Tprep 1
#define Tbake 10

pthread_mutex_t mutexCook;
pthread_cond_t condCook;

pthread_mutex_t mutexOven;
pthread_cond_t condOven;

int available_cook = Ncook;
int available_oven = Noven;

void* cook(void* arg) {
    int order_pizzas = *(int*)arg;

    pthread_mutex_lock(&mutexCook);
    while (available_cook == 0) {
        printf("No Cook Available\n");
        pthread_cond_wait(&condCook, &mutexCook);
    }
    available_cook--;
    pthread_mutex_unlock(&mutexCook);

    printf("Taking the order for %d pizzas\n", order_pizzas);
    sleep(Tprep * order_pizzas);
    printf("Finished preparation for %d pizzas\n", order_pizzas);

    pthread_mutex_lock(&mutexOven);
    while (available_oven < order_pizzas) {
        printf("Not enough ovens available\n");
        pthread_cond_wait(&condOven, &mutexOven);
    }
    available_oven -= order_pizzas;
    pthread_mutex_unlock(&mutexOven);

    sleep(Tbake * order_pizzas);
    printf("Pizzas ready\n");

    pthread_mutex_lock(&mutexOven);
    available_oven += order_pizzas;
    pthread_cond_signal(&condOven);
    pthread_mutex_unlock(&mutexOven);

    pthread_mutex_lock(&mutexCook);
    available_cook++;
    pthread_cond_signal(&condCook);
    pthread_mutex_unlock(&mutexCook);

    return NULL;
}

int main() {
    pthread_t thread_cooks[3];
    int pizza_orders[3] = {5, 8, 11}; // Example pizza orders

    pthread_mutex_init(&mutexCook, NULL);
    pthread_cond_init(&condCook, NULL);
    pthread_mutex_init(&mutexOven, NULL);
    pthread_cond_init(&condOven, NULL);

    // Create cook threads
    for (int i = 0; i < 3; i++) {
        if (pthread_create(&thread_cooks[i], NULL, cook, &pizza_orders[i]) != 0) {
            perror("Failed to create cook thread");
        }
    }

    // Join cook threads
    for (int i = 0; i < 3; i++) {
        if (pthread_join(thread_cooks[i], NULL) != 0) {
            perror("Failed to join cook thread");
        }
    }

    pthread_mutex_destroy(&mutexCook);
    pthread_cond_destroy(&condCook);
    pthread_mutex_destroy(&mutexOven);
    pthread_cond_destroy(&condOven);

    return 0;
}



