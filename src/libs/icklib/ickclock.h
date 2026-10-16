#ifndef ICKLIB_H_CLOCK
#define ICKLIB_H_CLOCK
#define CLOCK_UPDATE_FQ (50 * 1000000)

#include <threads.h>
#include <time.h>

typedef enum { CLOCK_IDLE, CLOCK_JOB, CLOCK_TERMINATE, CLOCK_ERR }  ickclock_stat;
typedef enum { CLOCK_TICK, CLOCK_THRD, CLOCK_SEQ }                  ickclock_mode;
typedef void (*ickclock_listener_func)(void *);
typedef struct ickclock_t ickclock_t;

#ifdef __cplusplus
static inline void ickwait(time_t sec) {
  struct timespec ts = { .tv_sec = sec, .tv_nsec = 0 };
  thrd_sleep(&ts, NULL);
}

static inline void ickwaitn(time_t nsec) {
  struct timespec ts = { .tv_sec = 0, .tv_nsec = nsec };
  thrd_sleep(&ts, NULL);
}
#else
static inline void ickwait(time_t sec)
{ thrd_sleep(&(struct timespec){.tv_sec = sec}, NULL); }

static inline void ickwaitn (time_t nsec)
{ thrd_sleep(&(struct timespec){.tv_sec = 0, .tv_nsec = nsec}, NULL); }
#endif

ickclock_t* ickclock();

void ickclock_listen(ickclock_t *tar, ickclock_listener_func func, void *args);
void ickclock_setlen(ickclock_t *tar, time_t t);
void ickclock_setmode(ickclock_t *tar, ickclock_mode md);
void ickclock_setinterval(ickclock_t *tar, time_t t);

time_t        ickclock_getlen(ickclock_t *tar);
time_t        ickclock_getcurr(ickclock_t *tar);
time_t        ickclock_getcend(ickclock_t *tar);
time_t        ickclock_getinterval(ickclock_t *tar);

ickclock_mode ickclock_getmode(ickclock_t *tar);
ickclock_stat ickclock_getstat(ickclock_t *tar);

int ickclock_smash(ickclock_t *tar);
int ickclock_boot(ickclock_t *tar);
int ickclock_shut(ickclock_t *tar);

#endif
