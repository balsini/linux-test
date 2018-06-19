#undef NDEBUG
#include <assert.h>
#include <errno.h>
#include <error.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>
#include <sys/time.h>

#include "sched_setattr_interface.h"
#include "utility.h"


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
#define THREAD_NUM 10
#define LOOP_CYCLES 1000

struct thread_params_t {
    int id;
    int result;
};

int tids[THREAD_NUM];

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


void *dl_thread(void *arg)
{
	struct sched_attr src = {
		.size = sizeof(struct sched_attr),

				.sched_policy = SCHED_DEADLINE,
				.sched_flags = 0,

				.sched_nice = 0,

        .sched_priority = 0,
	};
	int i;
  struct thread_params_t *tp = (struct thread_params_t *)arg;

  printf("DL: [%ld] checking\n", gettid());

  src.sched_period = 10 * 1000 * 1000,
  src.sched_deadline = src.sched_period;
  src.sched_runtime = 1 * 1000 * 1000 *
                      get_nprocs() *
                      0.9 /
                      THREAD_NUM;

  if (sched_setattr(0, &src, 0)) {
    log_critical("sched_setattr() returned. errno=%d", errno);
  }

  for (i = 0; i < LOOP_CYCLES; ++i) {
    sched_getattr(0, &src, sizeof(struct sched_attr), 0);
  }

	printf("DL: thread dies [%ld]\n", gettid());

  tp->result = 0;
  pthread_exit(&tp->result);
}


int main(int argc, char **argv)
{
  pthread_t thread[THREAD_NUM];
  struct thread_params_t thread_data[THREAD_NUM];
	int result_sum = 0;
	int i;

	printf("main thread [%ld]\n", gettid());

  for (i = 0; i < THREAD_NUM; ++i) {
    tids[i] = -1;
    if (pthread_create(&thread[i], NULL, dl_thread, &thread_data[i])) {
      log_critical("thread [%d] creation failed. errno=%d", i, errno);
    }
  }

  for (i = 0; i < THREAD_NUM; ++i) {
    if (pthread_join(thread[i], NULL)) {
      log_critical("thread [%d] join failed. errno=%d", i, errno);
    }

    printf("thread finished, returning [%d]\n", thread_data[i].result);
    result_sum |= thread_data[i].result;
	}

	printf("main dies [%ld]\n", gettid());

	return result_sum;
}
