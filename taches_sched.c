#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
void *fonc(void *arg)
{
    int i;
    for (i = 0; i < 7; i++)
    {
        printf("Tache %d : %d \n", (int)arg, i);
        usleep(1000000);
    }
}

int main(void)
{
    pthread_t tache1, tache2;
    pthread_attr_t attr;
    struct sched_param param;

    pthread_attr_init(&attr);

    // Tache main
    param.sched_priority = 12;
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

    // Tache 1
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = 10;
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&tache1, &attr, fonc, (void *)1);

    // Tache 2
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = 7;
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&tache1, &attr, fonc, (void *)2);

    pthread_attr_destroy(&attr);

    pthread_join(tache1, NULL);
    pthread_join(tache2, NULL);
    return 0;
}