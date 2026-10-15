#define MEMTRACK_IMPLEMENTATION
#include "MemTrack/memtrack.h"
#include <pthread.h>

void malloc_failure(void *data){
    printf("\ndub\n");
}


void* thread_func(void *arg){
    return t_malloc(100);
}



int main(void){

    MemTrack_Init(malloc_failure, NULL, true, true);

    pthread_t thread;

    if(pthread_create(&thread, NULL, thread_func, NULL)){
        return 1;
    } 

    int *array = t_malloc(999999);

    if(!array)
        printf("array is null\n");


    t_free(array);

    if(!array){

        printf("is null after free\n");

    } else{

        printf("isn't null after free\n");

    }
   

    void *result = NULL;
    pthread_join(thread, &result);

    if(MemTrack_is_memory_leak())
        MemTrack_tracking_info();
    
    MemTrack_Quit();

    return 0;
}


