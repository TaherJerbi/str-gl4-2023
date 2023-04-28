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

#define TO_CONVERT 6
#define TAILLE 8
struct rk_sema semaphores[TAILLE];
struct rk_sema s_reader;
pthread_mutex_t mutex;
int data[TAILLE];

void *reader(void *arg)
{
    rk_sema_wait(&s_reader);
    int n = *(int *)arg;
    printf("GOT NUMBER %d\n", n);
    pthread_mutex_lock(&mutex);
    data[0] = n;
    printf("ADDED NUMBER TO PIPELINE %d\n", n);
    pthread_mutex_unlock(&mutex);
    rk_sema_post(&semaphores[0]);
    printf("CALLED FIRST CONVERTER %d\n", n);
}
void *converter(void *arg)
{
    while (1)
    {

        int i = *(int *)arg;
        rk_sema_wait(&semaphores[i]);
        pthread_mutex_lock(&mutex);
        int n = data[i];
        pthread_mutex_unlock(&mutex);

        if (n != 0)
        {
            printf("%d", n % 2);
        }

        // if last
        if (n / 2 != 0)
        {
            if (i == TAILLE - 1)
            {
                perror("\nPipeline overflow\n");
            }
            else
            {
                pthread_mutex_lock(&mutex);
                data[i + 1] = n / 2;
                pthread_mutex_unlock(&mutex);
                rk_sema_post(&semaphores[i + 1]);
            }
        }
        else
        {
            printf("\n");
            rk_sema_post(&s_reader);
        }
    }
}
int main()
{
    pthread_t converters[TAILLE];
    pthread_t reader_thread1, reader_thread2;

    pthread_mutex_init(&mutex, NULL);
    for (int i = 0; i < TAILLE; i++)
    {
        rk_sema_init(&semaphores[i], 0);
    }
    rk_sema_init(&s_reader, 1);
    int *to_convert1 = malloc(sizeof(int));
    *to_convert1 = TO_CONVERT;
    int *to_convert2 = malloc(sizeof(int));
    *to_convert2 = TO_CONVERT * 10;

    pthread_create(&reader_thread1, NULL, reader, (void *)to_convert1);

    pthread_create(&reader_thread2, NULL, reader, (void *)to_convert2);

    for (int i = 0; i < TAILLE; i++)
    {
        int *index = malloc(sizeof(int));
        *index = i;
        pthread_create(&converters[i], NULL, converter, (void *)index);
    }
    for (int i = 0; i < TAILLE; i++)
    {
        int *index = malloc(sizeof(int));
        *index = i;
        pthread_join(converters[i], NULL);
    }
    pthread_join(reader_thread1, NULL);
    pthread_join(reader_thread2, NULL);

    return 0;
}
