#ifdef __APPLE__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

struct rk_sema
{
#ifdef __APPLE__
    dispatch_semaphore_t sem;
#else
    sem_t sem;
#endif
};

static inline void
rk_sema_init(struct rk_sema *s, uint32_t value)
{
#ifdef __APPLE__
    dispatch_semaphore_t *sem = &s->sem;

    *sem = dispatch_semaphore_create(value);
#else
    sem_init(&s->sem, 0, value);
#endif
}

static inline void
rk_sema_wait(struct rk_sema *s)
{

#ifdef __APPLE__
    dispatch_semaphore_wait(s->sem, DISPATCH_TIME_FOREVER);
#else
    int r;

    do
    {
        r = sem_wait(&s->sem);
    } while (r == -1 && errno == EINTR);
#endif
}

static inline void
rk_sema_post(struct rk_sema *s)
{

#ifdef __APPLE__
    dispatch_semaphore_signal(s->sem);
#else
    sem_post(&s->sem);
#endif
}

#define N_V 10
#define N_C 10

struct rk_sema s_voiture;
struct rk_sema s_camion;

// mutex sur pont (capacite, waiting_camions, waiting_voitures)
pthread_mutex_t mutex;

int capacite = 15;
int waiting_camions = N_C;
int waiting_voitures = N_V;

void *t_voiture(void *arg)
{
    int i = *(int *)arg;
    printf("VOITURE %d \n", i);
    // Try to get in
    pthread_mutex_lock(&mutex);
    while (capacite < 5)
    {
        pthread_mutex_unlock(&mutex);
        rk_sema_wait(&s_voiture);
        pthread_mutex_lock(&mutex);
    }
    capacite -= 5;
    waiting_voitures--;
    printf("Voiture %d ENTRE -- %d || WAITING::%d\n", i, capacite, waiting_voitures);

    pthread_mutex_unlock(&mutex);
    // passing
    usleep(500000);

    // passedd
    pthread_mutex_lock(&mutex);
    capacite += 5;
    printf("Voiture %d SORT -- %d\n", i, capacite);
    if (waiting_camions > 0)
    {
        rk_sema_post(&s_camion);
    }
    else
    {
        rk_sema_post(&s_voiture);
    }
    pthread_mutex_unlock(&mutex);

    free(arg);
}
void *t_camion(void *arg)
{
    int i = *(int *)arg;
    printf("CAMION %d \n", i);
    // Try to get in
    pthread_mutex_lock(&mutex);
    while (capacite < 15)
    {
        pthread_mutex_unlock(&mutex);
        rk_sema_wait(&s_camion);
        pthread_mutex_lock(&mutex);
    }
    capacite -= 15;
    waiting_camions--;
    printf("Camion %d ENTRE -- %d || WAITING::%d\n", i, capacite, waiting_camions);

    pthread_mutex_unlock(&mutex);
    // passing
    usleep(500000);

    // passedd
    pthread_mutex_lock(&mutex);
    capacite += 15;
    printf("Camion %d SORT -- %d\n", i, capacite);

    // wake up all possible vehicles
    if (waiting_camions > 0)
    {
        rk_sema_post(&s_camion);
    }
    else
    {
        rk_sema_post(&s_voiture);
        rk_sema_post(&s_voiture);
        rk_sema_post(&s_voiture);
    }
    pthread_mutex_unlock(&mutex);

    free(arg);
}

int main()
{

    pthread_t voitures[N_V];
    pthread_t camions[N_C];

    rk_sema_init(&s_voiture, 3);
    rk_sema_init(&s_camion, 1);
    pthread_mutex_init(&mutex, NULL);

    for (int i = 0; i < N_V; i++)
    {
        int *index = malloc(sizeof(int));
        *index = i;
        if (pthread_create(&voitures[i], NULL, t_voiture, (void *)index) != 0)
        {
            perror("Could not create VOITURE %d\n");
        }
    }
    for (int i = 0; i < N_C; i++)
    {
        int *index = malloc(sizeof(int));
        *index = i;
        if (pthread_create(&camions[i], NULL, t_camion, (void *)index) != 0)
        {
            perror("Could not create CAMION %d\n");
        }
    }
    for (int i = 0; i < N_V; i++)
    {
        if (pthread_join(voitures[i], NULL) != 0)
        {
            perror("Could not join VOITURE %d\n");
        }
    }
    for (int i = 0; i < N_C; i++)
    {
        if (pthread_join(camions[i], NULL) != 0)
        {
            perror("Could not create CAMION %d\n");
        }
    }
    return 0;
}
