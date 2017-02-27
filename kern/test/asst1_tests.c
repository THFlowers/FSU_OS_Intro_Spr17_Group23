
/*
 *  Thread fork and join test code.
 */

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
