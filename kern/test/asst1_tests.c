
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
	cv_u_test(argc, argv);
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

struct cvt_stock
{
        int max;
        int front;
        int tail;
        int buf[30];
        struct cv * empty;
        struct cv * full;
	struct lock * cvt_lock;
};
struct cvt_stock * stock;
static void cvtake(void*,unsigned long);
static void cvadd(void*,unsigned long);

static void cvtake(void * junk,unsigned long cvt_item)
{

	(void)junk;
	cvt_item=0;
	lock_acquire(stock->cvt_lock);
        while(stock->front==stock->tail)
                cv_wait(stock->empty,stock->cvt_lock);
        cvt_item=stock->buf[stock->front%stock->max];
        stock->front++;
        cv_signal(stock->full,stock->cvt_lock);
        lock_release(stock->cvt_lock);
	(void)cvt_item;
}
static void cvadd(void * junk,unsigned long item)
{
	(void)junk;
	lock_acquire(stock->cvt_lock);
        while((stock->tail-stock->front)==stock->max)
                  cv_wait(stock->full,stock->cvt_lock);
        stock->buf[stock->tail%stock->max]=item;
        stock->tail++;
        cv_signal(stock->empty,stock->cvt_lock);
        lock_release(stock->cvt_lock);
}


int cv_u_test(int nargs, char ** args)
{
	(void)nargs;
	(void)args;
	kprintf("\nStarting CV Unit tests...\n");
	int i=0;
	int k=0;
	//char threadname[20];
	unsigned long cvt_item;
	unsigned long cvt_item1;
	stock->max=30;
	stock->front=0;
	stock->tail=0;
	stock->cvt_lock=lock_create("CVTlock");
	stock->empty=cv_create("Empty");
	stock->full=cv_create("Full");




	//strcpy(threadname, "thread   ");
	cvt_item=i+1;
	cvt_item1=i+2;
	threadname[7]=i;
	threadname[8]='T';
	thread_fork(threadname, NULL, cvtake,NULL,cvt_item);
	threadname[8]='A';
	thread_fork(threadname,NULL,cvadd,NULL,cvt_item1);

	k++;
	if(stock->front>stock->tail)
	{
		kprintf("\nError CV breach\n");
		return 0;
	}
	while(k< stock->max)
	{
		kprintf("\n");
		for(i=stock->front;i< stock->tail;i++)
			kprintf("%d ",stock->buf[i]);
	}

	kprintf("\nCV tests done\n");

	return 1;

}


