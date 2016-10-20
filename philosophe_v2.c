#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#define N 5 // nombre de philosophes = nombre de baguettes 
#define PORTION 5

void prendre_baguettes(int i);
void reposer_baguettes(int i);
int manger(int i);
void wait_in_fifo(int i);
void sortir_fifo(int i);
void signaler_fifo(int i);
void *philosophe(void *arg);

typedef struct philo{
	int indice_philo;
	int nbr_portion;
}philo_t;

typedef struct file_baguette{
	pthread_cond_t cond;
	struct file_baguette *next;
}file_baguette_t;

int baguette_dispo[N];
pthread_mutex_t mutex;
file_baguette_t **fifo_baguettes;

void prendre_baguettes(int i){
	pthread_mutex_lock(&mutex);
	while(!baguette_dispo[i] || !baguette_dispo[(i+1)%N]){
		if(!baguette_dispo[i]){
			wait_in_fifo(i);
		}
		else if(!baguette_dispo[(i+1)%N]){
			wait_in_fifo((i+1)%N);
		}
		else if(!baguette_dispo[i] && !baguette_dispo[(i+1)%N]){
			wait_in_fifo(i);
			wait_in_fifo((i+1)%N);
		}
	}
	baguette_dispo[i] = 0;
	baguette_dispo[(i+1)%N] = 0;
	sortir_fifo(i);
	sortir_fifo((i+1)%N);
	pthread_mutex_unlock(&mutex);
}

void reposer_baguettes(int i){
	pthread_mutex_lock(&mutex);
	baguette_dispo[i] = 1;
	baguette_dispo[(i+1)%N] = 1;
	signaler_fifo(i);
	signaler_fifo((i+1)%N);
	pthread_mutex_unlock(&mutex);
}

int manger(int i){
	return i = i - 1;
}

void wait_in_fifo(int i){ //attendre baguette numéro i
	if(fifo_baguettes[i] == NULL){ //personne attend le baguette i
		fifo_baguettes[i] = malloc(sizeof(file_baguette_t));
		file_baguette_t *tete = fifo_baguettes[i];
		if(pthread_cond_init(&(tete->cond), NULL) != 0){
			fprintf(stderr, "Cond init error\n");
			exit(5);
		}
		tete->next = NULL;
		pthread_cond_wait(&(tete->cond), &mutex);
	}else{ //un philosophe est en train d'attandre le baguette i
		fifo_baguettes[i]->next = malloc(sizeof(file_baguette_t));
		file_baguette_t *next = fifo_baguettes[i]->next;
		if(pthread_cond_init(&(next->cond), NULL) != 0){
			fprintf(stderr, "Cond init error\n");
			exit(5);
		}
		next->next = NULL;
		pthread_cond_wait(&(next->cond), &mutex);
	}
}

void sortir_fifo(int i){
	if(fifo_baguettes[i] != NULL){
		if(fifo_baguettes[i]->next != NULL){
			fifo_baguettes[i] = fifo_baguettes[i]->next;
		}else{
			fifo_baguettes[i] = NULL;
		}
	}
}

void signaler_fifo(int i){
	if(fifo_baguettes[i] != NULL){
		pthread_cond_signal(&(fifo_baguettes[i]->cond));
	}
}

void *philosophe(void *arg){
	philo_t *p = (philo_t *)arg;
	pthread_t tid;
	tid = pthread_self();
	while(p->nbr_portion){
  		srand ((int) tid) ;
		usleep (rand() / RAND_MAX * 1000000.);

		/** Le philospophe a fini de penser et envie de manger**/
		prendre_baguettes(p->indice_philo);
		printf("philosophe %d mange une portion\n", p->indice_philo);
		p->nbr_portion = manger(p->nbr_portion);
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
	fifo_baguettes = malloc(sizeof(file_baguette_t *)*N);
	if(fifo_baguettes == NULL){
		fprintf(stderr, "fifo_baguettes error\n");
		exit(2);
	}
	// initialiser le tableau de baguette_dispo. Toutes les baguettes sont disponible
	for(int i=0; i<N; i++){
		baguette_dispo[i] = 1;
	}

	pthread_t *tids ;
	tids = malloc(sizeof(pthread_t)*N);
	if(tids == NULL){
		fprintf(stderr, "Malloc pthread_t error\n");
		exit(3);
	}

	philos = malloc(sizeof(philo_t)*N);
	if (philos == NULL){
		fprintf(stderr, "Malloc philo_t error\n");
		exit(4);
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
	free(fifo_baguettes);

	return 0;
}


