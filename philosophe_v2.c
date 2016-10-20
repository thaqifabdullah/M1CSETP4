#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#define N 5 // nombre de philosophes = nombre de baguettes 
#define PORTION 10

typedef struct philo{
	int indice_philo;
	int nbr_portion;
}philo_t;

typedef struct file_baguette{
	pthread_cond_t cond;
	philo_t *philo;
	struct file_baguette *next;
}file_baguette_t;

/**
Verifie si les deux baguettes sont disponible
Sinon, se met en attente dans la file d'attente des baguettes
Il sort de la file d'attente des deux baguettes quand ils sont disponibles
**/
void prendre_baguettes(philo_t *p);

/**
Signaler les deux philosophes qui son en attente dans les files d'attente des deux baguettes
**/
void poser_baguettes(int i);

/**
Reduire le nombre de portion
**/
int manger(int i);

/**
Le philosophe p entre dans la file d'attente de la baguette i
**/
void wait_in_fifo(int i, philo_t *p);

/**
Sortir de la file d'attente de la baguette i
**/
void sortir_fifo(int i);

/**
cherche le premièr philosophe qui en attente dans la file d'attente de la baguette i
**/
void signaler_fifo(int i);

/**
Le moniteur
**/
void *philosophe(void *arg);

int baguette_dispo[N];
pthread_mutex_t mutex;
file_baguette_t **fifo_baguettes; // Un tableau qui contient N pointeur sur file_baguette_t

void prendre_baguettes(philo_t *p){
	int i = p->indice_philo;
	pthread_mutex_lock(&mutex);
	while(!baguette_dispo[i] || !baguette_dispo[(i+1)%N]){
		if(!baguette_dispo[i]){
			wait_in_fifo(i,p);
		}
		else if(!baguette_dispo[(i+1)%N]){
			wait_in_fifo((i+1)%N, p);
		}
		else if(!baguette_dispo[i] && !baguette_dispo[(i+1)%N]){
			wait_in_fifo(i, p);
			wait_in_fifo((i+1)%N, p);
		}
	}
	baguette_dispo[i] = 0;
	baguette_dispo[(i+1)%N] = 0;
	sortir_fifo(i);
	sortir_fifo((i+1)%N);
	pthread_mutex_unlock(&mutex);
}

void poser_baguettes(int i){
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

void wait_in_fifo(int i, philo_t *p){ //attendre baguette numéro i
	if(fifo_baguettes[i] == NULL){ //personne attend le baguette i
		fifo_baguettes[i] = malloc(sizeof(file_baguette_t));
		file_baguette_t *tete = fifo_baguettes[i];
		if(pthread_cond_init(&(tete->cond), NULL) != 0){
			fprintf(stderr, "Cond init error\n");
			exit(5);
		}
		tete->philo = p;
		tete->next = NULL;
		pthread_cond_wait(&(tete->cond), &mutex);
	}else{ //il peut y avoir deux philosophe maximum qui est en attente
		int existe = 0;
		file_baguette_t *courant = fifo_baguettes[i];

		/**Verifie s'il est déjà dans la fifo**/
		while(courant->next != NULL){
			if(courant->philo == p)
				existe = 1;
			courant = courant->next;
		}

		/**S'il n'est pas dans le fifo, il peut entrer dans le fifo**/
		if(!existe){
			if(fifo_baguettes[i]->next == NULL){
				fifo_baguettes[i]->next = malloc(sizeof(file_baguette_t));
				file_baguette_t *next = fifo_baguettes[i]->next;
				if(pthread_cond_init(&(next->cond), NULL) != 0){
					fprintf(stderr, "Cond init error\n");
					exit(5);
				}
				next->philo = p;
				next->next = NULL;
				pthread_cond_wait(&(next->cond), &mutex);
			}
		}
		
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
		prendre_baguettes(p);
		printf("philospophe %d mange\n", p->indice_philo);
		p->nbr_portion = manger(p->nbr_portion);
		poser_baguettes(p->indice_philo);
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


