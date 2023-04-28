#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <pthread.h>

int a = 0;

void* fonc(void* arg){ 
    int i;
    for(i=0;i<7;i++){
        a++;
        printf("Tache %d : %d\n",(int) arg, i);
        if(i == 4){
            pthread_detach(pthread_self());
        }
        usleep(1000000); //attendre 1 seconde 
    }
}
void* detachedFonc(void* arg){
    a++;
}

int main(void) {
    pthread_t tache1, tache2, tache3;

    pthread_attr_t attr; // déclaration de la variable contenant les propriétés de la tâche pthread_attr_init(&attr);
    //initialisation de attr aux valeurs par défaut. Obligatoire avant toute manipulation de attr 
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    // affectation de la propriété détachable à attr
    pthread_create(&tache3, &attr, detachedFonc, (void*) 3);
    printf("TACHE 3!!");
    //...
    //déclaration des deux tâches
    pthread_create(&tache1, NULL, fonc, (void*) 1); //création effective de la tâche tache1
    pthread_create(&tache2, NULL, fonc, (void*) 2);

    pthread_join(tache1, NULL);
    //la fonction principale main(void), doit attendre la fin de l'exécution de la tâche tache1
    pthread_join(tache3, NULL);
    pthread_join(tache2, NULL); 

    pthread_attr_destroy(&attr); // détruire attr pour libérer la mémoire allouée
    printf("A : %d\n", a);
    return 0; 

}