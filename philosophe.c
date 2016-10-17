#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#define N 5 // nombre de philosophes = nombre de baguettes 
#define PORTION 5
int baguette_dispo[N];
pthread_mutex_t mutex;
pthread_cond_t cond[N];

typedef struct philo{
	int indice_philo;
	int nbr_portion;
}philo_t;

void prendre_baguettes(int i){
	pthread_mutex_lock(&mutex);
	while(!baguette_dispo[i] || !baguette_dispo[(i+1)%N]){
		if(!baguette_dispo[i])
			pthread_cond_wait(&cond[i],&mutex);
		else if(!baguette_dispo[(i+1)%N])
			pthread_cond_wait(&cond[(i+1)%N],&mutex);
		else if(!baguette_dispo[i] && !baguette_dispo[(i+1)%N]){
			pthread_cond_wait(&cond[i],&mutex);
			pthread_cond_wait(&cond[(i+1)%N],&mutex);
		}
	}
	baguette_dispo[i] = 0;
	baguette_dispo[(i+1)%N] = 0;
	pthread_mutex_unlock(&mutex);
}

void reposer_baguettes(int i){
	pthread_mutex_lock(&mutex);
	baguette_dispo[i] = 1;
	baguette_dispo[(i+1)%N] = 1;
	pthread_cond_broadcast(&cond[i]);
	pthread_cond_broadcast(&cond[(i+1)%N]);
	pthread_mutex_unlock(&mutex);
}

int manger(int i){
	return i = i - 1;
}

void *philosophe(void *arg){
	philo_t *p = (philo_t *)arg;
	pthread_t tid;
	tid = pthread_self();
	while(p->nbr_portion){
  		srand ((int) tid) ;
		usleep (rand() / RAND_MAX * 1000000.);

		/** Le philospophe a fini de penser et envie de manger**/

		printf("Thread %x indice %d a faim\n", (unsigned int)tid, p->indice_philo);
		prendre_baguettes(p->indice_philo);
		p->nbr_portion = manger(p->nbr_portion);
		printf("Thread %x indice %d a mangé une portion. Il reste %d portion\n", (unsigned int)tid, p->indice_philo, p->nbr_portion);
		reposer_baguettes(p->indice_philo);
	}
	printf("Thread %x indice %d a FINI !\n", (unsigned int)tid, p->indice_philo);
	return (void *) tid;
}


int main(int argc, char const *argv[]){
	philo_t *philos;
	if(pthread_mutex_init(&mutex,NULL) != 0){
		fprintf(stderr, "Mutex init error\n");
		exit(1);
	}
	for(int i=0; i<N; i++){
		pthread_cond_init(&cond[i],NULL);			
	}
	// initialiser le tableau de baguette_dispo. Toutes les baguettes sont disponible
	for(int i=0; i<N; i++){
		baguette_dispo[i] = 1;
	}

	pthread_t *tids ;
	tids = malloc(sizeof(pthread_t)*N);
	if(tids == NULL){
		fprintf(stderr, "Malloc pthread_t error\n");
		exit(2);
	}

	philos = malloc(sizeof(philo_t)*N);
	if (philos == NULL){
		fprintf(stderr, "Malloc philo_t error\n");
		exit(3);
	}

	for(int i = 0 ;i <N ;i++){
		philos[i].indice_philo = i;
		philos[i].nbr_portion = PORTION;
		pthread_create(&tids[i],NULL,philosophe,&philos[i]);
	}

	for(int j = 0;j <N ; j++){
		pthread_join(tids[j],NULL);
	}

	free(tids);

	return 0;
}


