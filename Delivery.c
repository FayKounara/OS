#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>  
#include <time.h>
#include "constants.h" 

int available_phones=Ntel;
int available_cook = Ncook;
int available_oven = Noven;
int available_deliverer=Ndeliver;

pthread_mutex_t phone_mutex, mutexCook, mutexOven, mutexDeliverer; 
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
void* phone_operator(void *t) {
    int *tid = (int *)t;
    unsigned int seed = time(NULL) ^ *tid;
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
    int order_pizzas = Norderlow + rand_r(&seed) % (Norderhigh - Norderlow + 1);
    rc=pthread_mutex_unlock(&phone_mutex);
    if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(t);
    }
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
        failed_orders++;
        printf("Η παραγγελία με αριθμό %d απέτυχε.\n", *tid);
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
        successful_orders++;
        printf("Η παραγγελία με αριθμό %d καταχωρήθηκε.\n", *tid);
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
        printf("No Deliverer Available\n");
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
    int elapsed_time_prepare = (end_time_prepare.tv_sec - start_time.tv_sec) + 
                          (end_time_prepare.tv_nsec - start_time.tv_nsec) / 1e9;

    printf("Η παραγγελία με αριθμό %d ετοιμάστηκε σε %d λεπτά.\n", *tid, elapsed_time_prepare );

    int waiting=Tdellow + rand_r(&seed) % (Tdelhigh - Tdellow + 1);
    sleep(waiting);
    clock_gettime(CLOCK_REALTIME, &end_time_deliver);
    int elapsed_time_delivery = (end_time_deliver.tv_sec - start_time.tv_sec) + 
                      (end_time_deliver.tv_nsec - start_time.tv_nsec) / 1e9;
    printf("Η παραγγελία με αριθμό %d παραδόθηκε σε %d λεπτά.\n", *tid, elapsed_time_delivery );
    sum_total_time+=elapsed_time_delivery;
    if (max_time<elapsed_time_delivery) {
        max_time=elapsed_time_delivery;
    }

    int elapsed_time_cold = (end_time_deliver.tv_sec - start_time_cold.tv_sec) + 
                      (end_time_deliver.tv_nsec - start_time_cold.tv_nsec) / 1e9;
    sum_cold+=elapsed_time_cold;
    if (max_cold<elapsed_time_cold) {
        max_cold=elapsed_time_cold;
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
        printf("Παρακαλώ συμπληρώστε τα σωστά στοιχεία.");
        return 1;
  } 

  int Ncust = strtol(argv[1], NULL, 10);
  unsigned int seed = strtoul(argv[2], NULL, 10);

  pthread_t threads[Ncust]; 
  int threadIds[Ncust];
  int threadTime[Ncust]; 

  pthread_mutex_init(&mutexCook, NULL);
  pthread_cond_init(&condCook, NULL);
  
  pthread_mutex_init(&mutexOven, NULL);
  pthread_cond_init(&condOven, NULL); 
  
  pthread_mutex_init(&phone_mutex, NULL);
  pthread_cond_init(&phone_cond, NULL);
    
   for (int i = 0; i < Ncust; i++) {
        threadIds[i] = i + 1;
        if (i == 0) {
            threadTime[i] = 0;
        } else {
            threadTime[i] = threadTime[i - 1] + Torderlow + rand_r(&seed) % (Torderhigh - Torderlow + 1);
        }

        //printf("Main: creating thread %d\n", threadIds[i]);
	sleep(threadTime[i] - (i > 0 ? threadTime[i - 1] : 0));
        rc=pthread_create(&threads[i], NULL, phone_operator, &threadIds[i]);
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

printf("Όλες οι παραγγελίες έχουν ολοκληρωθεί.\n");
    printf("Συνολικά έσοδα: %d\n", profit);
    printf("Συνολικές πίτσες Margarita: %d\n", Sm);
    printf("Συνολικές πίτσες Peperoni: %d\n", Sp);
    printf("Συνολικές πίτσες Special: %d\n", Ss);
    printf("Πλήθος πετυχημένων παραγγελειών: %d\n", successful_orders);
    printf("Πλήθος αποτυχημένων παραγγελειών: %d\n", failed_orders); 

    if (successful_orders>0) {
        double average_time= sum_total_time/successful_orders;
        printf("Ο μέσος χρόνος εξυπηρέτησης των πελατών είναι: %.2f \n", average_time); 
        printf("Ο μέγιστος χρόνος εξυπηρέτησης των πελατών είναι: %d\n", max_time);

        double average_cold_time= sum_cold/successful_orders;
        printf("Ο μέσος χρόνος κρυώματος της παραγγελιας είναι: %.2f \n", average_cold_time); 
        printf("Ο μέγιστος χρόνος κρυώματος της παραγγελιας είναι: %d\n", max_cold);
    }
return 0; 
}  
