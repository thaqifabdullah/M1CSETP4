#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#define N 5 // nombre de philosophes = nombre de baguettes 
#define PORTION 10
int bag_dispo[N];
pthread_mutex_t mutex;
pthread_cond_t *cond;

typedef struct philo{
	int nbr_portion;
	int indice_philo;
}philo_t;

void *philosophe(void *arg){
	philo_t *p = (void *)arg;
	tid = pthread_self () ;
  	srand ((int) tid) ;
	usleep (rand() / RAND_MAX * 1000000.);
	pthread_mutex_lock(&mutex);
		while(bag_dispo[p->indice_philo] || bag_dispo[(p->indice_philo+1)%N]){
			bag_dispo[p->indice_philo] ? pthread_cond_wait(&cond[p->indice_philo]) :  pthread_cond_wait(&cond[(p->indice_philo+1)%N]);
		}
	bag_dispo[arg->indice_philo] = 1;
	bag_dispo[(arg->indice_philo+1)%N] = 1;
	pthread_mutex_unlock(&mutex);
	arg->nbr_portion--; //Philosophe mange une portion
	printf("Philosophe no %d a mangé une portion. Il reste %d portion\n", arg->indice_philo, arg->nbr_portion);
	pthread_mutex_lock(&mutex);
	bag_dispo[arg->indice_philo] = 0;
	bag_dispo[(arg->indice_philo+1)%N] = 0;
	pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&mutex);
	return (void *) tid;
}


int main(int argc, char const *argv[])
{
	pthread_mutex_init(&mutex,NULL);
	for(int i=0; i<N; i++){
		pthread_cond_init(&cond[i],NULL);			
	}
	// initialiser le tableau de bag_dispo. Toutes les baguettes sont disponible
	for(int i=0; i<N; i++){
		bag_dispo[i] = 0;
	}

	pthread_t *tids ;
	tids = malloc(sizeof(pthread_t)*N);

	philo_t *philos;
	philos = malloc(sizeof(struct philo)*N);

	for(int i = 0 ;i <N ;i++){
		philos->nbr_portion = PORTION;
		philos->indice_philo = i;
		pthread_create(&tids[i],NULL,philosophe,&philos[i]);
	}

	for(int j = 0;j <N ; j++){
		pthread_join(tids[j],NULL);
	}

	free(tids);
	free(philos);

	return 0;
}


