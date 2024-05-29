#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>  
#include <time.h>
#include "t8210017-t8210066-pizza.h" 

int available_phones=Ntel;
int available_cook = Ncook;
int available_oven = Noven;
int available_deliverer=Ndeliver;

pthread_mutex_t phone_mutex, mutexCook, mutexOven, mutexDeliverer, mutexStatistics, mutexPrint, mutexTest; 
pthread_cond_t phone_cond, condCook, condOven, condDeliverer;

int profit=0;
int Sm=0;
int Sp=0;
int Ss=0;
int successful_orders=0;
int failed_orders=0;
int sum_total_time=0;
int max_time=0; 
int sum_cold=0;
int max_cold=0;
int rc;
unsigned int seed;

void* operator(void *t) {
    int *tid = (int *)t;
    struct timespec start_time, start_time_cold, end_time_prepare, end_time_deliver;
    clock_gettime(CLOCK_REALTIME, &start_time);

    rc=pthread_mutex_lock(&phone_mutex);
    if (rc != 0) {	
		printf("ERROR: return code from pthrea`-d_mutex_lock() is %d\n", rc);
		pthread_exit(t);
	}

    while (available_phones == 0) {
        rc=pthread_cond_wait(&phone_cond, &phone_mutex);
        if (rc != 0) {	
		    printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
		    pthread_exit(t);
	    }
    }
    
    available_phones--;
    rc=pthread_mutex_unlock(&phone_mutex);
    if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(t);
    }
    int order_pizzas = Norderlow + rand_r(&seed) % (Norderhigh - Norderlow + 1);
    int arr[order_pizzas];
    for (int i = 0; i < order_pizzas; i++) {
        float rand_prob = (float)rand_r(&seed) / RAND_MAX;
        if (rand_prob < Pm) {
            arr[i]=1;
        } else if (rand_prob < Pm + Pp) {
            arr[i]=2;
        } else {
            arr[i]=3;
        }
    }

    int payment_time = Tpaymentlow + rand_r(&seed) % (Tpaymenthigh - Tpaymentlow + 1);
    sleep(payment_time);

    float payment_prob = (float)rand_r(&seed) / RAND_MAX;
    if (payment_prob < Pfail) {
        rc=pthread_mutex_lock(&mutexStatistics);
        if (rc != 0) {	
            printf("ERROR: return code from pthrea`-d_mutex_lock() is %d\n", rc);
            pthread_exit(t);
        }
        failed_orders++;
        rc=pthread_mutex_unlock(&mutexStatistics);
        if (rc != 0) {	
            printf("ERROR: return code from pthrea`-d_mutex_unlock() is %d\n", rc);
            pthread_exit(t);
        }

        rc=pthread_mutex_lock(&mutexPrint);
        if (rc != 0) {	
            printf("ERROR: return code from pthrea`-d_mutex_lock() is %d\n", rc);
            pthread_exit(t);
        }
        printf("Your order with id %d was cancelled.\n", *tid);

        rc=pthread_mutex_unlock(&mutexPrint);
        if (rc != 0) {	
            printf("ERROR: return code from pthrea`-d_mutex_unlock() is %d\n", rc);
            pthread_exit(t);
        }
        pthread_mutex_lock(&phone_mutex);
        available_phones++;
        rc=pthread_cond_signal(&phone_cond);
        if (rc != 0) {	
		printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
		pthread_exit(t);
	    }	
        rc=pthread_mutex_unlock(&phone_mutex);
        if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(t);
	    }

        return NULL;
    } else {
        rc=pthread_mutex_lock(&mutexStatistics);
        if (rc != 0) {	
            printf("ERROR: return code from pthrea`-d_mutex_lock() is %d\n", rc);
            pthread_exit(t);
        }
        successful_orders++;
        rc=pthread_mutex_unlock(&mutexStatistics);
        if (rc != 0) {	
            printf("ERROR: return code from pthrea`-d_mutex_unlock() is %d\n", rc);
            pthread_exit(t);
        }

        rc=pthread_mutex_lock(&mutexPrint);
        if (rc != 0) {	
            printf("ERROR: return code from pthrea`-d_mutex_lock() is %d\n", rc);
            pthread_exit(t);
        }
        printf("Your order with id %d was successful.\n", *tid);

        rc=pthread_mutex_unlock(&mutexPrint);
        if (rc != 0) {	
            printf("ERROR: return code from pthrea`-d_mutex_unlock() is %d\n", rc);
            pthread_exit(t);
        }
        rc=pthread_mutex_lock(&mutexStatistics);
        if (rc != 0) {	
            printf("ERROR: return code from pthrea`-d_mutex_lock() is %d\n", rc);
            pthread_exit(t);
        }
        for (int i = 0; i < order_pizzas; i++) {
            if (arr[i]==1) {
                profit+=Cm;
                Sm++;
            } else if (arr[i]==2) {
                profit+=Cp;
                Sp++;
            } else {
                profit+=Cs;
                Ss++;
            }
        }
        rc=pthread_mutex_unlock(&mutexStatistics);
        if (rc != 0) {	
            printf("ERROR: return code from pthrea`-d_mutex_unlock() is %d\n", rc);
            pthread_exit(t);
        }
    }

    rc=pthread_mutex_lock(&phone_mutex);
    if (rc != 0) {	
			printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
			pthread_exit(t);
    }	
    available_phones++;
    rc=pthread_cond_signal(&phone_cond);
    if (rc != 0) {	
	printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
	pthread_exit(t);
    }	
    rc=pthread_mutex_unlock(&phone_mutex);
    if (rc != 0) {	
	printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
	pthread_exit(t);
    }
//end with callers
//beginning of preparation
    rc=pthread_mutex_lock(&mutexCook);
    if (rc != 0) {	
	printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
	pthread_exit(t);
    }
    while (available_cook == 0) {
        rc=pthread_cond_wait(&condCook, &mutexCook);
        if (rc != 0) {	
		printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
		pthread_exit(t);
	    }
    }
    
    available_cook--;
    pthread_mutex_unlock(&mutexCook);
    if (rc != 0) {	
	printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
	pthread_exit(t);
    }
    sleep(Tprep * order_pizzas);

    rc=pthread_mutex_lock(&mutexOven);
    if (rc != 0) {	
	printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
	pthread_exit(t);
    }
    
    while (available_oven < order_pizzas) {
        //printf("Not enough ovens available\n");
        rc=pthread_cond_wait(&condOven, &mutexOven);
        if (rc != 0) {	
		printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
		pthread_exit(t);
	    }
    }
    available_oven -= order_pizzas;
    rc=pthread_mutex_unlock(&mutexOven);
    if (rc != 0) {	
			printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
			pthread_exit(t);
	}
    
    rc=pthread_mutex_lock(&mutexCook);
    if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(t);
	}

    available_cook++;
    rc=pthread_cond_signal(&condCook);
    if (rc != 0) {	
					printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
					pthread_exit(t);
	}
    rc=pthread_mutex_unlock(&mutexCook);
    if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(t);
	}

    sleep(Tbake);
    clock_gettime(CLOCK_REALTIME, &start_time_cold);

    //oven ready time for deliverers
    rc=pthread_mutex_lock(&mutexDeliverer);
    if (rc != 0) {	
			printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
			pthread_exit(t);
	}	
    while (available_deliverer == 0) {
        rc=pthread_cond_wait(&condDeliverer, &mutexDeliverer);
        if (rc != 0) {	
					printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
					pthread_exit(t);
		}
    }
    available_deliverer--;
    rc=pthread_mutex_unlock(&mutexDeliverer);
    if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(t);
	}
    rc=pthread_mutex_lock(&mutexOven);
    if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(t);
	}
    available_oven += order_pizzas;
    rc=pthread_cond_signal(&condOven);
    if (rc != 0) {	
					printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
					pthread_exit(t);
	}
    
    rc=pthread_mutex_unlock(&mutexOven);
    if (rc != 0) {	
			printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
			pthread_exit(t);
	}
    sleep(Tpack*order_pizzas);
    clock_gettime(CLOCK_REALTIME, &end_time_prepare);
    int elapsed_time_prepare = end_time_prepare.tv_sec - start_time.tv_sec ;
                          
    rc=pthread_mutex_lock(&mutexPrint);
    if (rc != 0) {	
        printf("ERROR: return code from pthrea`-d_mutex_lock() is %d\n", rc);
        pthread_exit(t);
    }
    printf("Your order with id %d was ready in %d minutes.\n", *tid, elapsed_time_prepare );
    rc=pthread_mutex_unlock(&mutexPrint);
    if (rc != 0) {	
        printf("ERROR: return code from pthrea`-d_mutex_unlock() is %d\n", rc);
        pthread_exit(t);
    }

    int waiting=Tdellow + rand_r(&seed) % (Tdelhigh - Tdellow + 1);
    sleep(waiting);
    clock_gettime(CLOCK_REALTIME, &end_time_deliver);
    int elapsed_time_delivery = end_time_deliver.tv_sec - start_time.tv_sec;

    rc=pthread_mutex_lock(&mutexPrint);
    if (rc != 0) {	
        printf("ERROR: return code from pthrea`-d_mutex_lock() is %d\n", rc);
        pthread_exit(t);
    }
    printf("The order with id %d was delivered in %d minutes.\n", *tid, elapsed_time_delivery );
    rc=pthread_mutex_unlock(&mutexPrint);
    if (rc != 0) {	
        printf("ERROR: return code from pthrea`-d_mutex_unlock() is %d\n", rc);
        pthread_exit(t);
    }
    rc=pthread_mutex_lock(&mutexStatistics);
    if (rc != 0) {	
        printf("ERROR: return code from pthrea`-d_mutex_lock() is %d\n", rc);
        pthread_exit(t);
    }
    sum_total_time+=elapsed_time_delivery;
    if (max_time<elapsed_time_delivery) {
        max_time=elapsed_time_delivery;
    }
    rc=pthread_mutex_unlock(&mutexStatistics);
    if (rc != 0) {	
        printf("ERROR: return code from pthrea`-d_mutex_unlock() is %d\n", rc);
        pthread_exit(t);
    }

    int elapsed_time_cold = (end_time_deliver.tv_sec - start_time_cold.tv_sec) + 
                      (end_time_deliver.tv_nsec - start_time_cold.tv_nsec) / 1e9;
        
    rc=pthread_mutex_lock(&mutexStatistics);
    if (rc != 0) {	
        printf("ERROR: return code from pthrea`-d_mutex_lock() is %d\n", rc);
        pthread_exit(t);
    }
    sum_cold+=elapsed_time_cold;
    if (max_cold<elapsed_time_cold) {
        max_cold=elapsed_time_cold;
    }
    rc=pthread_mutex_unlock(&mutexStatistics);
    if (rc != 0) {	
        printf("ERROR: return code from pthrea`-d_mutex_unlock() is %d\n", rc);
        pthread_exit(t);
    }
    sleep(waiting);
    rc=pthread_mutex_lock(&mutexDeliverer);
    if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(t);
	}
    available_deliverer +=1;
    rc=pthread_cond_signal(&condDeliverer);
    if (rc != 0) {	
					printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
					pthread_exit(t);
	}
    rc=pthread_mutex_unlock(&mutexDeliverer);
    if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(t);
	}

    return NULL;
}

int main(int argc, char *argv[]) {

  if (argc != 3) {
        printf("Please enter the correct values.\n");
        return 1;
  } 

  int Ncust = atoi(argv[1]);
  seed = atoi(argv[2]);

  pthread_t *threads; 
  threads = malloc(Ncust * sizeof(pthread_t));
  if (threads == NULL) {
    printf("NOT ENOUGH MEMORY!\n");
	  return -1;
  }
  int threadIds[Ncust]; 
  int threadTime;

  pthread_mutex_init(&mutexCook, NULL);
  pthread_cond_init(&condCook, NULL);
  
  pthread_mutex_init(&mutexOven, NULL);
  pthread_cond_init(&condOven, NULL); 
  
  pthread_mutex_init(&phone_mutex, NULL);
  pthread_cond_init(&phone_cond, NULL);
    
   for (int i = 0; i < Ncust; i++) {
        threadIds[i] = i + 1;
        if (i == 0) {
            threadTime = 0;
        } else {
            threadTime =  Torderlow + rand_r(&seed) % (Torderhigh - Torderlow + 1);
        }
        sleep(threadTime);
        rc=pthread_create(&threads[i], NULL, operator, &threadIds[i]);
        if (rc != 0) {
            printf("ERROR: return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
        
        
   }
   
   // Join threads 
   for (int i = 0; i < Ncust; i++) { 
      pthread_join(threads[i], NULL);
      if (rc != 0) {
			printf("ERROR: return code from pthread_join() is %d\n", rc);
			exit(-1);		
      }
   } 
   free(threads);
   
   rc=pthread_mutex_destroy(&phone_mutex);
   if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
   } 
   rc=pthread_cond_destroy(&phone_cond);
   if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
   }
   rc=pthread_mutex_destroy(&mutexCook);
   if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
   }
   rc=pthread_cond_destroy(&condCook);
   if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
   }
   rc=pthread_mutex_destroy(&mutexOven);
   if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
   }
   rc=pthread_cond_destroy(&condOven);
   if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
   }

printf("All the orders have been completed.\n");
    printf("Total Profit: %d\n", profit);
    printf("Total of Margarita orders: %d\n", Sm);
    printf("Total of Peperoni orders: %d\n", Sp);
    printf("Total of Special orders: %d\n", Ss);
    printf("Total of successful orders: %d\n", successful_orders);
    printf("Total of cancelled orders: %d\n", failed_orders); 

    if (successful_orders>0) {
        double average_time= sum_total_time/successful_orders;
        printf("The average service time was: %.0f \n", average_time); 
        printf("The maximum service time was: %d\n", max_time);

        double average_cold_time= sum_cold/successful_orders;
        printf("The average cold time was: %.0f \n", average_cold_time); 
        printf("The maximum cold time was: %d\n", max_cold);
    }
return 0; 

}  
