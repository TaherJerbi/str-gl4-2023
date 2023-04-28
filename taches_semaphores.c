#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <dispatch/dispatch.h>

dispatch_semaphore_t evt;

void *tache1(void *arg)
{
    int i = 0;
    while (i < 10)
    {
        printf("La tache %s s'execute \n", (char *)arg);
        usleep(1000000 / 2);

        i++;
    }
    for (int i = 0; i < 10; i++)
    {
        dispatch_semaphore_signal(evt);
    }
}
void *tache2(void *arg)
{
    int i = 0;
    while (i < 10)
    {
        dispatch_semaphore_wait(evt, DISPATCH_TIME_FOREVER);

        printf("La tache %s s'execute enfin\n", (char *)arg);

        i++;
    }
}

int main()
{
    pthread_t th1, th2;

    evt = dispatch_semaphore_create(0);

    pthread_create(&th1, NULL, tache1, "1");
    pthread_create(&th2, NULL, tache2, "2");

    pthread_join(th1, NULL);
    pthread_join(th2, NULL);

    return 0;
}