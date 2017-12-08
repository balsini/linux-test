#undef NDEBUG
#include <assert.h>
#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "sched_setattr_interface.h"


/*
 * Tests to be performed:
 *  - SCHED_FAIR check:
 *    - sched_nice must be consistent
 *    - the sched_priority value must be always 0
 *  - SCHED_RT:
 *    - sched_priority must be consistent
 *  - sched_DL check:
 *    - sched_period, sched_deadline and sched_runtime must be consistent
 *    - the sched_priority value must be always 0
 */
#define TODO 1

#define PARAM_LOOP_SIZE 100000

#define log_to(where, msg, args...)			\
	do {									\
	fprintf(where, msg "\n", ##args);		\
	} while (0)

#define log_critical(msg, args...)					\
	do {									\
	log_to(stderr, "[CRIT-ERR] %s:%d: " msg, __FUNCTION__, __LINE__, ##args);	\
	exit(-1); \
	} while (0)

static inline void
check_common_sched_attr(const struct sched_attr *a,
			const struct sched_attr *b)
{
	if (a->size != b->size)
		log_critical("a->size (%d) != b->size (%d)", a->size, b->size);
	if (a->sched_policy != b->sched_policy)
		log_critical("a->sched_policy (%d) != b->sched_policy (%d)",
			     a->sched_policy, b->sched_policy);
	if (a->sched_flags != b->sched_flags)
		log_critical("a->sched_flags (%lld) != b->sched_flags (%lld)",
			     a->sched_flags, b->sched_flags);
}

/*
 * setattr_result is the value that sched_setattr should return
 */
static inline void
check_fair_sched_attr(const struct sched_attr *template, int setattr_result)
{
	struct sched_attr src = *template;
	struct sched_attr dst;
	int res;
	
	res = sched_setattr(0, &src, 0);
	if (res != setattr_result) {
		log_critical("sched_setattr() returned %d, but %d was expected",
			     res, setattr_result);
	}

	if (res != 0)
		return;
	
	sched_getattr(0, &dst, sizeof(dst), 0);

	check_common_sched_attr(&src, &dst);

	if (dst.sched_priority != 0)
		log_critical("sched_priority != 0");

	if (memcmp(&src, &dst, sizeof(dst))) {
		if (src.sched_nice != dst.sched_nice) {
			struct rlimit nice_limit;

			if (getrlimit(RLIMIT_NICE, &nice_limit) == -1)
				log_critical("getrlimit(RLIMIT_NICE, &nice_limit) == -1");

			if (src.sched_nice < -20 && dst.sched_nice < 20 - nice_limit.rlim_cur) {
				log_critical("src.sched_nice (%d) != "
					     "dst.sched_nice (%d)",
					     src.sched_nice,
					     dst.sched_nice);
			}
			if (src.sched_nice > 19 && dst.sched_nice > 19) {
				log_critical("src.sched_nice (%d) != "
					     "dst.sched_nice (%d)",
					     src.sched_nice,
					     dst.sched_nice);
			}
		} else {
			log_critical("memcmp failed");
		}
	}
}

void *fair_thread(void *result)
{
	struct sched_attr src = {
		.size = sizeof(struct sched_attr),

				.sched_policy = SCHED_OTHER,
				.sched_flags = 0,

				.sched_nice = 5,

				.sched_priority = 0,

				.sched_runtime = 0,
				.sched_period = 0,
				.sched_deadline = 0,
	};
	int i;

	printf("FAIR: checking FAIR [%ld]\n", gettid());
	
	for (i=0; i<PARAM_LOOP_SIZE; ++i) {
		for (src.sched_nice = -100;
		     src.sched_nice <= 100;
		     ++src.sched_nice) {
			check_fair_sched_attr(&src, 0);
		}
	}
	
	printf("FAIR: thread dies [%ld]\n", gettid());

	*((int *)result) = 0;
	pthread_exit(result);
}


void check_dl_sched_attr(const struct sched_attr *template, int setattr_result)
{
	struct sched_attr src = *template;
	struct sched_attr dst;
	int res;

	res = sched_setattr(0, &src, 0);
	if (res != setattr_result) {
		log_critical("sched_setattr() returned %d, but %d was expected "
			     "runtime: %lld, deadline: %lld, period: %lld",
			     res, setattr_result,
			     src.sched_runtime, src.sched_deadline, src.sched_period);
	}

	if (res != 0)
		return;

	sched_getattr(0, &dst, sizeof(dst), 0);

	check_common_sched_attr(&src, &dst);

	if (dst.sched_priority != 0)
		log_critical("sched_priority != 0");

	if (memcmp(&src, &dst, sizeof(dst))) {
		if (src.sched_runtime != dst.sched_runtime)
			log_critical("src.sched_runtime (%lld) != "
				     "dst.sched_runtime (%lld)",
				     src.sched_runtime,
				     dst.sched_runtime);
		else if (src.sched_period != dst.sched_period && dst.sched_period != src.sched_deadline)
			log_critical("src.sched_period (%lld) != "
				     "dst.sched_period (%lld)",
				     src.sched_period,
				     dst.sched_period);
		else if (src.sched_deadline != dst.sched_deadline)
			log_critical("src.sched_deadline (%lld) != "
				     "dst.sched_deadline (%lld)",
				     src.sched_deadline,
				     dst.sched_deadline);
	}
}


void *dl_thread(void *result)
{
	struct sched_attr src = {
		.size = sizeof(struct sched_attr),

				.sched_policy = SCHED_DEADLINE,
				.sched_flags = 0,

				.sched_nice = 0,

				.sched_priority = 0,

				.sched_runtime = 1 * 1000 * 1000,
				.sched_period = 30 * 1000 * 1000,
				.sched_deadline = 30 * 1000 * 1000,
	};
	int i;

	printf("DL: checking FAIR [%ld]\n", gettid());

	for (src.sched_runtime = 0;
	     src.sched_runtime <= 20000;
	     src.sched_runtime += 1000) {
		for (src.sched_period = 0;
		     src.sched_period <= 30000;
		     src.sched_period += 1000) {
			for (src.sched_deadline = 0;
			     src.sched_deadline <= 40000;
			     src.sched_deadline += 1000) {
				if ((src.sched_period != 0 && src.sched_period < src.sched_deadline) ||
						src.sched_deadline < src.sched_runtime ||
						src.sched_runtime < (1 << 10)) {
					check_dl_sched_attr(&src, -1);
				} else {
					check_dl_sched_attr(&src, 0);
				}
			}
		}
	}

	printf("DL: thread dies [%ld]\n", gettid());

	*((int *)result) = 0;
	pthread_exit(result);
}


void check_rt_sched_attr(const struct sched_attr *template, int setattr_result)
{
	struct sched_attr src = *template;
	struct sched_attr dst;
	int res;

	res = sched_setattr(0, &src, 0);
	if (res != setattr_result) {
		log_critical("sched_setattr() returned %d, but %d was expected "
			     "priority: %d",
			     res, setattr_result,
			     src.sched_priority);
	}

	if (res != 0)
		return;

	sched_getattr(0, &dst, sizeof(dst), 0);

	check_common_sched_attr(&src, &dst);

	if (src.sched_priority != dst.sched_priority)
		log_critical("src.sched_priority (%d) != "
			     "dst.sched_priority (%d)",
			     src.sched_priority,
			     dst.sched_priority);
}


void *rt_thread(void *result)
{
	struct sched_attr src = {
		.size = sizeof(struct sched_attr),

				.sched_policy = SCHED_FIFO,
				.sched_flags = 0,

				.sched_nice = 5,

				.sched_priority = 50,

				.sched_runtime = 0,
				.sched_period = 0,
				.sched_deadline = 0,
	};
	int i;

	printf("RT: checking FAIR [%ld]\n", gettid());

	for (i=0; i<PARAM_LOOP_SIZE; ++i) {
		for (src.sched_priority = 0;
		     src.sched_priority <= 150;
		     ++src.sched_priority) {
			if (src.sched_priority < 1 || src.sched_priority > 99)
				check_rt_sched_attr(&src, -1);
			else
				check_rt_sched_attr(&src, 0);
		}
	}

	printf("RT: thread dies [%ld]\n", gettid());

	*((int *)result) = 0;
	pthread_exit(result);
}


int main(int argc, char **argv)
{
	pthread_t thread[3];
	int thread_result[3];
	int result_sum = 0;
	int i;

	printf("main thread [%ld]\n", gettid());

	pthread_create(&thread[0], NULL, dl_thread, &thread_result[0]);
	pthread_create(&thread[1], NULL, rt_thread, &thread_result[1]);
	pthread_create(&thread[2], NULL, fair_thread, &thread_result[2]);

	for (i=0; i<3; ++i) {
		pthread_join(thread[i], NULL);
		printf("thread finished, returning [%d]\n", thread_result[i]);
		result_sum += thread_result[i];
	}

	printf("main dies [%ld]\n", gettid());

	return result_sum;
}
