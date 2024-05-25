#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> // for sleep function 
#include <time.h>
#include "constants.h" // correct inclusion of constants.h 

int available_phones=Ntel; 
pthread_mutex_t phone_mutex; 
pthread_cond_t phone_cond;
int profit=0;
int *num_pizzas; 

void* phone_operator(void *threadId) {
    int *tid = (int *)threadId;
    int rc;
    unsigned int seed = time(NULL) ^ *tid;

    rc = pthread_mutex_lock(&phone_mutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    // Check if this customer is next in line
    while (available_phones == 0) {
        pthread_cond_wait(&phone_cond, &phone_mutex);
    }

    // At this point, this customer is next in line
    available_phones--;
    int pizzas = Norderlow + rand_r(&seed) % (Norderhigh - Norderlow + 1);
    num_pizzas[*(int *)threadId-1]=pizzas;

     rc = pthread_mutex_unlock(&phone_mutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }
    int arr[pizzas];
    for (int i = 0; i < pizzas; i++) {
        float rand_prob = (float)rand_r(&seed) / RAND_MAX;
        if (rand_prob < Pm) {
            arr[i]=1;
        } else if (rand_prob < Pm + Pp) {
            arr[i]=2;
        } else {
            arr[i]=3;
        }
    }


    // Simulate using the phone for 5 seconds
    int payment_time = Tpaymentlow + rand_r(&seed) % (Tpaymenthigh - Tpaymentlow + 1);
    sleep(payment_time);

    float payment_prob = (float)rand_r(&seed) / RAND_MAX;
    if (payment_prob < Pfail) {
        printf("Thread %d's payment failed. Order is cancelled.\n", *tid);
    } else {
        printf("Thread %d's payment succeeded. Order is processed.\n", *tid);
        
    }

    rc = pthread_mutex_lock(&phone_mutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    available_phones++;

    pthread_cond_signal(&phone_cond);

    rc = pthread_mutex_unlock(&phone_mutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }

    pthread_exit(threadId);
}

int main(int argc, char *argv[]) { 
  int Ncust = 10; 
  pthread_t threads[Ncust]; 
  int threadIds[Ncust];
  int threadTime[Ncust]; 
  num_pizzas = (int *)malloc(Ncust * sizeof(int));
  int rc; 
  unsigned int seed = time(NULL);

  // Initialize the mutex 
   rc = pthread_mutex_init(&phone_mutex, NULL);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }

    rc = pthread_cond_init(&phone_cond, NULL);
    if (rc != 0) {
        printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
        exit(-1);
    }

   // Create threads 
   for (int i = 0; i < Ncust; i++) {
        threadIds[i] = i + 1;
        if (i == 0) {
            threadTime[i] = 0;
        } else {
            threadTime[i] = threadTime[i - 1] + Torderlow + rand_r(&seed) % (Torderhigh - Torderlow + 1);
        }

        //printf("Main: creating thread %d\n", threadIds[i]);
        rc = pthread_create(&threads[i], NULL, phone_operator, &threadIds[i]);
        if (rc != 0) {
            printf("ERROR: return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
        sleep(threadTime[i] - (i > 0 ? threadTime[i - 1] : 0));
   }
   // Join threads 
   for (int i = 0; i < Ncust; i++) { 
      rc = pthread_join(threads[i], NULL); 
      if (rc != 0) { 
        printf("ERROR: return code from pthread_join() is %d\n", rc); 
        exit(-1); 
      } 
   } 
   // Destroy the mutex 
   rc = pthread_mutex_destroy(&phone_mutex); 
   if (rc != 0) { 
      printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc); 
      exit(-1); 
   } 

for (int i = 0; i < Ncust; i++) {
    //printf("Customer %d ordered %d pizzas.\n", i + 1, num_pizzas[i]);
}

free(num_pizzas);
printf("All threads have completed.\n"); 
return 0; 
}  
