#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "sched_setattr_interface.h"


#define FAIR_THREAD_NUM 7
#define RT_THREAD_NUM 7
#define DL_THREAD_NUM 7
#define THREAD_NUM (FAIR_THREAD_NUM + RT_THREAD_NUM + DL_THREAD_NUM)

#define ITERATIONS 100000

pthread_barrier_t global_barrier_enter;
pthread_barrier_t global_barrier_leave;

long int thread_tid_fair[FAIR_THREAD_NUM];
long int thread_tid_rt[RT_THREAD_NUM];
long int thread_tid_dl[DL_THREAD_NUM];

void *dl_thread(void *result)
{
	unsigned int i, j;
	struct sched_attr check;
	struct sched_attr attr1 = {
		.size = sizeof(struct sched_attr),

		.sched_policy = SCHED_DEADLINE,
		.sched_flags = 0,

		.sched_nice = 0,

		.sched_priority = 0,

		.sched_runtime =   100 * 1000,
		.sched_period =   1000 * 1000,
		.sched_deadline = 1000 * 1000,
	};
	struct sched_attr attr2 = {
		.size = sizeof(struct sched_attr),

		.sched_policy = SCHED_DEADLINE,
		.sched_flags = 0,

		.sched_nice = 0,

		.sched_priority = 0,

		.sched_runtime =   200 * 1000,
		.sched_period =   2000 * 1000,
		.sched_deadline = 2000 * 1000,
	};

	*((long int *)result) = gettid();

	printf("DL: deadline thread created [%ld]\n", *((long int *)result));

	pthread_barrier_wait(&global_barrier_enter);

	printf("DL: deadline thread started [%ld]\n", *((long int *)result));

	for (i=0; i<ITERATIONS; ++i) {
		if (sched_setattr(0, &attr1, 0)) {
			perror("Unable to perform sched_setattr");
			exit -1;
		}
		sched_getattr(0, &check, sizeof(struct sched_attr), 0);
		assert(memcmp(&attr1, &check, sizeof(struct sched_attr)) == 0);
		if (sched_setattr(0, &attr2, 0)) {
			perror("Unable to perform sched_setattr");
			exit -1;
		}
		sched_getattr(0, &check, sizeof(struct sched_attr), 0);
		assert(memcmp(&attr2, &check, sizeof(struct sched_attr)) == 0);

		for (j=0; j<RT_THREAD_NUM; ++j)
			sched_getattr(thread_tid_rt[j], &check, sizeof(struct sched_attr), 0);
		for (j=0; j<FAIR_THREAD_NUM; ++j)
			sched_getattr(thread_tid_fair[j], &check, sizeof(struct sched_attr), 0);
	}

	printf("DL: thread dies [%ld]\n", *((long int *)result));

	pthread_barrier_wait(&global_barrier_leave);

	pthread_exit(result);
}


void *rt_thread(void *result)
{
	unsigned int i, j;
	struct sched_attr check;
	struct sched_attr attr1 = {
		.size = sizeof(struct sched_attr),

		.sched_policy = SCHED_FIFO,
		.sched_flags = 0,

		.sched_nice = 0,

		.sched_priority = 50,

		.sched_runtime = 0,
		.sched_period = 0,
		.sched_deadline = 0,
	};
	struct sched_attr attr2 = {
		.size = sizeof(struct sched_attr),

		.sched_policy = SCHED_FIFO,
		.sched_flags = 0,

		.sched_nice = 0,

		.sched_priority = 30,

		.sched_runtime = 0,
		.sched_period = 0,
		.sched_deadline = 0,
	};

	*((long int *)result) = gettid();

	printf("RT: deadline thread created [%ld]\n", *((long int *)result));

	pthread_barrier_wait(&global_barrier_enter);

	printf("RT: deadline thread started [%ld]\n", *((long int *)result));

	for (i=0; i<ITERATIONS; ++i) {
		if (sched_setattr(0, &attr1, 0)) {
			perror("Unable to perform sched_setattr");
			exit -1;
		}
		sched_getattr(0, &check, sizeof(struct sched_attr), 0);
		assert(memcmp(&attr1, &check, sizeof(struct sched_attr)) == 0);
		if (sched_setattr(0, &attr2, 0)) {
			perror("Unable to perform sched_setattr");
			exit -1;
		}
		sched_getattr(0, &check, sizeof(struct sched_attr), 0);
		assert(memcmp(&attr2, &check, sizeof(struct sched_attr)) == 0);

		for (j=0; j<DL_THREAD_NUM; ++j)
			sched_getattr(thread_tid_dl[j], &check, sizeof(struct sched_attr), 0);
		for (j=0; j<FAIR_THREAD_NUM; ++j)
			sched_getattr(thread_tid_fair[j], &check, sizeof(struct sched_attr), 0);
	}

	printf("RT: thread dies [%ld]\n", *((long int *)result));

	pthread_barrier_wait(&global_barrier_leave);

	pthread_exit(result);
}


void *fair_thread(void *result)
{
	unsigned int i, j;
	struct sched_attr check;
	struct sched_attr attr1 = {
		.size = sizeof(struct sched_attr),

		.sched_policy = SCHED_OTHER,
		.sched_flags = 0,

		.sched_nice = -5,

		.sched_priority = 0,

		.sched_runtime = 0,
		.sched_period = 0,
		.sched_deadline = 0,
	};
	struct sched_attr attr2 = {
		.size = sizeof(struct sched_attr),

		.sched_policy = SCHED_OTHER,
		.sched_flags = 0,

		.sched_nice = 5,

		.sched_priority = 0,

		.sched_runtime = 0,
		.sched_period = 0,
		.sched_deadline = 0,
	};

	*((long int *)result) = gettid();

	printf("FAIR: deadline thread created [%ld]\n", *((long int *)result));

	pthread_barrier_wait(&global_barrier_enter);

	printf("FAIR: deadline thread started [%ld]\n", *((long int *)result));

	for (i=0; i<ITERATIONS; ++i) {
		if (sched_setattr(0, &attr1, 0)) {
			perror("Unable to perform sched_setattr");
			exit -1;
		}
		sched_getattr(0, &check, sizeof(struct sched_attr), 0);
		assert(memcmp(&attr1, &check, sizeof(struct sched_attr)) == 0);
		if (sched_setattr(0, &attr2, 0)) {
			perror("Unable to perform sched_setattr");
			exit -1;
		}
		sched_getattr(0, &check, sizeof(struct sched_attr), 0);
		assert(memcmp(&attr2, &check, sizeof(struct sched_attr)) == 0);

		for (j=0; j<RT_THREAD_NUM; ++j)
			sched_getattr(thread_tid_rt[j], &check, sizeof(struct sched_attr), 0);
		for (j=0; j<DL_THREAD_NUM; ++j)
			sched_getattr(thread_tid_dl[j], &check, sizeof(struct sched_attr), 0);
	}

	printf("FAIR: thread dies [%ld]\n", *((long int *)result));

	pthread_barrier_wait(&global_barrier_leave);

	pthread_exit(result);
}


int main(int argc, char **argv)
{
	pthread_t thread[THREAD_NUM];
	unsigned int i;

	printf("main thread [%ld]\n", gettid());

	pthread_barrier_init(&global_barrier_enter,
			     NULL,
			     THREAD_NUM + 1);
	pthread_barrier_init(&global_barrier_leave,
			     NULL,
			     THREAD_NUM + 1);

	for (i=0;;++i) {
		if (i < RT_THREAD_NUM)
			pthread_create(&thread[i], NULL, rt_thread, &thread_tid_rt[i]);
		else if (i - RT_THREAD_NUM < DL_THREAD_NUM)
			pthread_create(&thread[i], NULL, dl_thread, &thread_tid_dl[i - RT_THREAD_NUM]);
		else if (i - RT_THREAD_NUM - DL_THREAD_NUM < FAIR_THREAD_NUM)
			pthread_create(&thread[i], NULL, fair_thread, &thread_tid_fair[i - RT_THREAD_NUM - DL_THREAD_NUM]);
		else
			break;
	}

	pthread_barrier_wait(&global_barrier_enter);
	pthread_barrier_wait(&global_barrier_leave);

	for (i=0; i<THREAD_NUM; ++i) {
		pthread_join(thread[i], NULL);
		printf("thread [%d] finished\n",
		       i);
	}

	printf("main dies [%ld]\n", gettid());

	pthread_barrier_destroy(&global_barrier_enter);
	pthread_barrier_destroy(&global_barrier_leave);

	return 0;
}
