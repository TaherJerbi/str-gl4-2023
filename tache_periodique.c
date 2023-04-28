#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
pthread_cond_t cond;
pthread_mutex_t verrou;
struct timespec time;

void *tachePeriodique(void *periode)
{

    int i = 0;
    clock_gettime(CLOCK_REALTIME, &time);
    while (i < 10)
    {
        pthread_mutex_lock(&verrou);

        time.tv_sec = time.tv_sec + (int)periode;
        printf("La tache de periode %d s'execute periodiquement à l'instant %d secondes\n", (int)periode, (int)time.tv_sec);

        pthread_cond_timedwait(&cond, &verrou, &time);
        pthread_mutex_unlock(&verrou);
        i++;
    }
}

int main(void)
{
    pthread_t tache1, tache2;

    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&verrou, NULL);
    pthread_create(&tache1, NULL, tachePeriodique,
                   (void *)2);
    pthread_create(&tache1, NULL, tachePeriodique,
                   (void *)3);
    // la tache1 est périodique de periode 5s
    pthread_join(tache1, NULL);
    return 0;
}