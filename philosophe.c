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

void prendre_baguettes(int i){
	pthread_mutex_lock(&mutex);
	while(!baguette_dispo[i] || !baguette_dispo[(i+1)%N]){
		if(!baguette_dispo[i])
			pthread_cond_wait(&cond[i],&mutex);
		else if(!baguette_dispo[(i+1)%N])
			pthread_cond_wait(&cond[(i+1)%N],&mutex);
		else{
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
	return i--;
}

void *philosophe(void *arg){
	int indice_philo = *(int *)arg;
	int nbr_portion = 5;
	pthread_t tid;
	tid = pthread_self();
	while(nbr_portion){
  		srand ((int) tid) ;
		usleep (rand() / RAND_MAX * 1000000.);
		printf("Thread %x a faim\n", (unsigned int)tid);
		prendre_baguettes(indice_philo);
		nbr_portion = manger(nbr_portion);
		printf("Thread %x a mangé une portion\n", (unsigned int)tid);
		reposer_baguettes(indice_philo);
	}
	return (void *) tid;
}


int main(int argc, char const *argv[])
{
	if(pthread_mutex_init(&mutex,NULL) != 0){
		fprintf(stderr, "Mutex init error\n");
		return -1;
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

	for(int i = 0 ;i <N ;i++){
		printf("create pthread\n");
		pthread_create(&tids[i],NULL,philosophe,&i);
	}

	for(int j = 0;j <N ; j++){
		pthread_join(tids[j],NULL);
	}

	free(tids);

	return 0;
}


