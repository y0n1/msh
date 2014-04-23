#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#define COUNT 100

// Our Cubby Hole representation
typedef struct cubbyhole_info {
	int item;
	sem_t *avail, *empty, *cs;
} CubbyHole;

// Our threads data
typedef struct thread_data {
	int id;
	int count; // Number of items to put/get
	CubbyHole *cubbyHole; // Shared cubbyhole
} ThreadInfo;

// Initializes a cubbyhole (allocates the semaphores in OS registry)
void cubbyhole_init(CubbyHole *cubbyHole) {
	cubbyHole->avail = sem_open("avail", O_CREAT, 0666, 0);
	if (cubbyHole->avail == SEM_FAILED) { perror("sem_open(avail) error"); exit(1); }
	cubbyHole->empty = sem_open("empty", O_CREAT, 0666, 1);
	if (cubbyHole->empty == SEM_FAILED) { perror("sem_open(empty) error"); exit(1); }
	cubbyHole->cs = sem_open("cs", O_CREAT, 0666, 1);
	if (cubbyHole->cs == SEM_FAILED) { perror("sem_open(cs) error"); exit(1); }
}

// Destroys a cubbyhole (frees the semaphores in OS registry)
void cubbyhole_destroy(CubbyHole *cubbyHole) {
	sem_close(cubbyHole->avail);
	sem_close(cubbyHole->empty);
	sem_close(cubbyHole->cs);
}

// Puts a value in the cubbyhole
void put(CubbyHole *cubbyHole, int value) {
	// First, wait for the cubbyhole to become empty (initially it is empty and this
        // semaphore is up, so first producer will not block here)
	sem_wait(cubbyHole->empty);

        // Then, protect the access to cubbyHole->item using a critical section around it
	sem_wait(cubbyHole->cs);

	// Put value
	cubbyHole->item = value;

        // Now exit the critical section by releasing the mutex
	sem_post(cubbyHole->cs);
 
        // Now signal that the cubbyhole has an item available. The next waiting consumer
        // will get this value from it
	sem_post(cubbyHole->avail);
}

// Gets item from cubbyhole
int get(CubbyHole *cubbyHole) {
	int res;

        // First, wait for the cubbyhole to become available
        sem_wait(cubbyHole->avail);

        // Then, protect the access to cubbyHole->item using a critical section around it
        sem_wait(cubbyHole->cs);


        // Get value
        res = cubbyHole->item;

        // Now exit the critical section by releasing the mutex
        sem_post(cubbyHole->cs);
 
        // Now signal that the cubbyhole is empty. The next waiting producer
        // will put a new value into it
        sem_post(cubbyHole->empty);

	return res; // Return the value that was in the cubbyhole
}

// Producer thread start point
void *run_producer(void *param) {
	int i;
	ThreadInfo *info = (ThreadInfo*)param;
	
	// Put some values
	for (i = 0; i < info->count; i++) {
		put(info->cubbyHole, i + 1);
	}
	return NULL;
}

// Consumer thread start point
void *run_consumer(void *param) {
	int i, sum;
	ThreadInfo *info = (ThreadInfo*)param;
	
	// Get the values
	sum = 0;
	for (i = 0; i < info->count; i++) {
		sum += get(info->cubbyHole);
	}

	// Print result
	printf("Sum = %d\n", sum);
	return NULL;
}

// Main start point
int main() {
	// Create thread descriptors, cubbyhole, info structs
	pthread_t producer, consumer;
	CubbyHole cubbyHole;
	ThreadInfo prod_info, cons_info;

	// Init cubbyhole
	cubbyhole_init(&cubbyHole);

	// Init thread info structs
	prod_info.id = 0;
	cons_info.id = 1;
	cons_info.count = prod_info.count = COUNT;
	cons_info.cubbyHole = prod_info.cubbyHole = &cubbyHole;

	// Start threads
	pthread_create(&producer, NULL, run_producer, (void*)(&prod_info));
	pthread_create(&consumer, NULL, run_consumer, (void*)(&cons_info));
	
	// Join threads
	pthread_join(producer, NULL);
	pthread_join(consumer, NULL);
	
	// Free cubbyhole allocated resources
	cubbyhole_destroy(&cubbyHole);

	return 0;
}
