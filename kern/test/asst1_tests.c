
/*
 *  Thread fork and join test code.
 */

// Implemented by Thai Flowers

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <synch.h>
#include <test.h>

int
asst1_tests(int argc, char** argv)
{
	jointest1(argc, argv);
	jointest2(argc, argv);
	cv_u_test(argc, argv);
	return 0;
}
#define NTHREADS 6

static
void
/* delay threads in order via printing to force threads exiting after a join request has begun */ 
talkythread(void *junk, unsigned long num)
{
	(void)junk;

	unsigned int i;
	for (i=0; i<(num+1)*100000; i++){
		if ((i%10000) == 0)
			kprintf("Thread %d sleeping: %d \n", (int)num, i);
	}
	kprintf("Thread %d exiting \n", (int)num);
}

static
void
/* delay threads in order without printing to force threads exiting before a join request has begun */ 
quietthread(void *junk, unsigned long num)
{
	(void)junk;

	unsigned int i;
	for (i=0; i<(num+1)*100000; i++);
	kprintf("Thread %d exiting \n", (int)num);
}

int
jointest1(int nargs, char** args)
{
	(void)nargs;
	(void)args;

	char name[16];
	int i;
	int result;

	struct thread* threads[NTHREADS];

	kprintf("Starting join test...\n");
	kprintf("If this completes it works\n");
	for (i=0; i<NTHREADS; i++) {
		threads[i]=NULL;
		kprintf("Creating thread %d \n", i);
		snprintf(name, sizeof(name), "jointest1 %d", i);
		result = thread_fork_for_join(name, &threads[i],
				     talkythread,
				     NULL, i);
		if (result) {
			panic("jointest1: thread_fork failed %s)\n",
			      strerror(result));
		}
		if (threads[i] == NULL) {
			panic("threads not properly initialized\n");
		}
	}
	for (i=0; i<NTHREADS; i++) {
		kprintf("Joining thread %d : \n", i);
		thread_join(threads[i]);
		kprintf("Join %d Success! \n", i);
	}

	kprintf("\njoin test done.\n");

	return 0;
}

int
jointest2(int nargs, char** args)
{
	(void)nargs;
	(void)args;

	char name[16];
	int i;
	int result;

	struct thread* threads[NTHREADS];

	kprintf("Starting join test...\n");
	kprintf("If this completes it works\n");
	for (i=0; i<NTHREADS; i++) {
		threads[i]=NULL;
		kprintf("Creating thread %d \n", i);
		snprintf(name, sizeof(name), "jointest1 %d", i);
		result = thread_fork_for_join(name, &threads[i],
				     quietthread,
				     NULL, i);
		if (result) {
			panic("jointest2: thread_fork failed %s)\n",
			      strerror(result));
		}
		if (threads[i] == NULL) {
			panic("threads not properly initialized\n");
		}
	}
	for (i=0; i<NTHREADS; i++) {
		kprintf("Joining thread %d : \n", i);
		thread_join(threads[i]);
		kprintf("Join %d Success! \n", i);
	}

	kprintf("\njoin test done.\n");

	return 0;
}

// Implemented by Joseph Viyan
struct stock
{
        int max;
        int front;
        int tail;
        int* buffer;
        struct cv * empty;
        struct cv * full;
	struct lock * s_lock;
};

// Global variable for CV tests
struct stock * test_stock;
#define NUM_ITEMS 40

int stock_remove(struct stock* stock);
void stock_add(struct stock* stock, int item);

int
stock_remove(struct stock* stock)
{
	int item;

	lock_acquire(stock->s_lock);
        while(stock->front==stock->tail)
                cv_wait(stock->empty,stock->s_lock);
        item=stock->buffer[stock->front % stock->max];
        stock->front++;
        cv_signal(stock->full,stock->s_lock);

        lock_release(stock->s_lock);

	return item;
}

void
stock_add(struct stock* stock, int item)
{
	lock_acquire(stock->s_lock);
        while((stock->tail-stock->front) == stock->max)
                  cv_wait(stock->full,stock->s_lock);
        stock->buffer[stock->tail%stock->max]=item;
        stock->tail++;
        cv_signal(stock->empty,stock->s_lock);
        lock_release(stock->s_lock);
}

static
void
add_thread(void *junk, unsigned long num)
{
	(void)junk;

	unsigned int i;
	for (i=0; i<(num+1); i++) {
		stock_add(test_stock, i);
		kprintf("%d send\n", i);
	}
}

static
void
rem_thread(void *junk, unsigned long num)
{
	(void)junk;

	unsigned int out;
	do {
		out = stock_remove(test_stock);
		kprintf("%d recieved!\n", out);
	} while (out < num);
}

int cv_u_test(int nargs, char ** args)
{
	(void)nargs;
	(void)args;

	kprintf("\nStarting CV Unit tests...\n");

	test_stock = kmalloc(sizeof(struct stock));
	test_stock->max=20;
	test_stock->front=0;
	test_stock->tail=0;
	test_stock->buffer = kmalloc(test_stock->max*sizeof(int));
	test_stock->s_lock=lock_create("CVTlock");
	test_stock->empty=cv_create("Empty");
	test_stock->full=cv_create("Full");

	struct thread* consumer;

	int result;
	result = thread_fork("producer", NULL, add_thread, NULL, NUM_ITEMS);
	if (result) {
		panic("cv test: thread_fork failed %s)\n", strerror(result));
	}
	result = thread_fork_for_join("consumer", &consumer, rem_thread, NULL, NUM_ITEMS);
	if (result) {
		panic("cv test: thread_fork failed %s)\n", strerror(result));
	}

	thread_join(consumer);

	kfree(test_stock->buffer);
	lock_destroy(test_stock->s_lock);
	cv_destroy(test_stock->empty);
	cv_destroy(test_stock->full);

	kprintf("\nCV tests done\n");

	return 0;
}

