#include <ickutils.h>
#include <ickclock.h>
#include <stdlib.h>
#include <time.h>

struct ickclock_t {
  ickclock_stat clock_stat;
  ickclock_mode clock_mode;
  thrd_t listen_thrd;
  thrd_t clock_thrd;
  mtx_t  mlock;
  time_t curr;
  time_t cend;
  time_t clen;
  time_t interval;
};

struct ickclock_listen_dt {
  ickclock_listener_func func;
  thrd_t            thrd_func;
  ////////////////////////////
  ickclock_t        *clock;
  void              *args;
};

ickclock_t* ickclock() {
  ickclock_t *clk = (ickclock_t *)malloc(sizeof(ickclock_t));

  if (!clk) { return NULL; }
  if (mtx_init(&(clk->mlock), mtx_plain) == thrd_error)
  { free(clk); return NULL; }

  ickout(ICKOUT_LOG, "clock[%p] - initialized\n", clk);
  clk->clock_stat = CLOCK_IDLE;
  clk->clock_mode = CLOCK_THRD;
  clk->curr = 0;
  clk->cend = 0;
  clk->clen = 0;
  clk->interval = 0;

  return clk;
}

// [[FIELD_START  *typical errors logger  *//
enum ickclock_errn {
  ERR_BUSYCLOCK,
  ERR_NULLCLOCK,
  ERR_MALFUNC,
};
static void internal_errmsg(ickclock_t *tar, enum ickclock_errn errtype) {
  switch (errtype) {
    default:
    { ickout(ICKOUT_WARN,  "clock[%p] -> unrecognized error type[%d]?\n", tar, errtype); return; }
    case ERR_BUSYCLOCK: 
    { ickout(ICKOUT_ERROR, "clock[%p] -> busy clock.\n", tar); return; }
    case ERR_NULLCLOCK:
    { ickout(ICKOUT_ERROR, "clock[%p] -> clock missing.\n", tar); return; }
    case ERR_MALFUNC:
    { ickout(ICKOUT_ERROR, "clock[%p] -> malfunctioned clock, needs rebuild new clock.\n", tar); return; }
  }
}
// [[FIELD_END  *typical errors logger  *//

static int internal_runtime(void *dt) {
  struct timespec ts = { 0 };
  int thrd_status = 0 ;
  ickclock_t *tar = (ickclock_t *)dt;

  mtx_t *k = &(tar->mlock);

  if (tar->clock_mode == CLOCK_TICK) {
    if (tar->interval == 0) {
      ts.tv_sec = 0;
      ts.tv_nsec = CLOCK_UPDATE_FQ;
    } else {
      ts.tv_sec  = tar->interval;
      ts.tv_nsec = 0;
    }

    while (tar->clock_stat != CLOCK_TERMINATE) {
      thrd_sleep(&ts, NULL);
      mtx_lock(k);
      tar->cend = time(0);
      tar->clen = difftime(tar->cend, tar->curr);
      mtx_unlock(k);
    }

    goto exit_success;
  }

  //* seq & thrd clock  *//
  ts.tv_sec = tar->clen;
  thrd_status = thrd_sleep(&ts, NULL);

  if (thrd_status == -1) { goto exit_early; }
  if (thrd_status !=  0) { goto exit_ue;    }

  goto exit_success;

  //* shared  *//
  exit_success:
    if (tar->clock_mode != CLOCK_TICK) 
    { ickclock_shut(tar); }
    return ICKSUCCESS;
  //* exceptions  *//
  exit_early:
    ickout(ICKOUT_WARN, "clock[%p] - exiting early.\n", dt);
    /* method will implement here later*/
    return ICKSUCCESS;
  exit_ue:
    ickout(ICKOUT_ERROR, "clock[%p] - unexpected issue?\n", dt);
    /* method will implement here later*/
    return ICKUE;
}

static int ickclock_listen_internal(void *dt) {
  struct ickclock_listen_dt *listener = (struct ickclock_listen_dt *)dt;
  ickclock_t *tar = listener->clock;

  while (ickclock_getstat(tar) != CLOCK_TERMINATE) {
    if (ickclock_getstat(tar) != CLOCK_TERMINATE)
    { listener->func(listener->args); }
    if (ickclock_getinterval(tar) != 0 ) {
      ickwait(ickclock_getinterval(tar));
    } else { 
      ickwaitn(CLOCK_UPDATE_FQ);
    }
  }
  free(listener);

  return ICKSUCCESS;
}

void ickclock_listen(ickclock_t *tar, void ( *func) (void *), void *args) {
  ickout(ICKOUT_LOG, "clock[%p] - start listening\n", tar);
  if (tar->clock_stat != CLOCK_JOB) {
    ickout(ICKOUT_ERROR, "clock[%p] - failed to listen to func before booted\n", tar);
    return;
  }

  struct ickclock_listen_dt *listener = (struct ickclock_listen_dt *) malloc(sizeof(struct ickclock_listen_dt));
  if (!listener) { return; }

  listener->clock = tar;
  listener->func = func;
  listener->args = args;

  if (thrd_create(&(tar->listen_thrd), ickclock_listen_internal, listener) != thrd_success)
  { free(listener); return; }
}

// [[FIELD_START  *user get-set  *//
void ickclock_setlen(ickclock_t *tar, time_t t) { 
  mtx_lock(&(tar->mlock));
  if (tar->clock_stat == CLOCK_JOB) { internal_errmsg(tar, ERR_BUSYCLOCK); goto setlen_finish; }
  tar->clen = t;
  setlen_finish: mtx_unlock(&(tar->mlock));
}
void ickclock_setmode(ickclock_t *tar, ickclock_mode md) {
  mtx_lock(&(tar->mlock));
  if (tar->clock_stat == CLOCK_JOB) { internal_errmsg(tar, ERR_BUSYCLOCK); goto setmode_finish; }
  tar->clock_mode = md;
  setmode_finish: mtx_unlock(&(tar->mlock));
}
void ickclock_setinterval(ickclock_t *tar, time_t t) {
  if (tar->clock_mode != CLOCK_TICK) 
  { return; }
  mtx_lock(&(tar->mlock));
  if (tar->clock_stat == CLOCK_JOB) { internal_errmsg(tar, ERR_BUSYCLOCK); goto setinterval_finish; }
  tar->interval = t;
  setinterval_finish: mtx_unlock(&(tar->mlock));
}

time_t ickclock_getlen(ickclock_t *tar) { 
  time_t rs;
  mtx_lock(&(tar->mlock)); 
  rs = tar->clen;
  mtx_unlock(&(tar->mlock)); 
  return rs; 
}
time_t ickclock_getcurr(ickclock_t *tar) {
  time_t rs;
  mtx_lock(&(tar->mlock));
  rs = tar->curr;
  mtx_unlock(&(tar->mlock));
  return rs;
}
time_t ickclock_getcend(ickclock_t *tar) {
  time_t rs;
  mtx_lock(&(tar->mlock));
  rs = tar->cend;
  mtx_unlock(&(tar->mlock));
  return rs;
}
time_t ickclock_getinterval(ickclock_t *tar) {
  time_t rs;
  mtx_lock(&(tar->mlock));
  rs = tar->interval;
  mtx_unlock(&(tar->mlock));
  return rs;
}

ickclock_mode ickclock_getmode(ickclock_t *tar) { 
  ickclock_mode rs;
  mtx_lock(&(tar->mlock)); 
  rs = tar->clock_mode;
  mtx_unlock(&(tar->mlock)); 
  return rs; 
}
ickclock_stat ickclock_getstat(ickclock_t *tar) { 
  ickclock_stat rs;
  mtx_lock(&(tar->mlock)); 
  rs = tar->clock_stat;
  mtx_unlock(&(tar->mlock));
  return rs;
}
// [[FIELD_END  *user get-set  *//

int ickclock_smash(ickclock_t *tar) {
  if (!tar) { internal_errmsg(tar, ERR_NULLCLOCK); return ICKERR; }
  ickout(ICKOUT_LOG, "clock[%p] - removing\n", tar);

  if (ickclock_getstat(tar) == CLOCK_JOB) {
    ickout(ICKOUT_WARN, "clock[%p] is being removed while running\n", tar);
    ickout(ICKOUT_LOG, "clock[%p] - removing -> force\n", tar);

    if (ickclock_shut(tar) == ICKSUCCESS) { goto clock_cleanup; }
    ickout(ICKOUT_ERROR, "clock[%p] - critical -> clock is not being stopped correctly!\n", tar);
    ickout(ICKOUT_ERROR, "clock[%p] - remove failure -> various unexpected issue may then occur\n", tar);

    mtx_lock(&(tar->mlock));
    tar->clock_stat = CLOCK_ERR;
    mtx_unlock(&(tar->mlock));

    return ICKERR;
  }

  clock_cleanup:
    ickout(ICKOUT_LOG, "clock[%p] - removing - completed\n", tar);
    mtx_destroy(&(tar->mlock));
    free(tar);

  return ICKSUCCESS;
}

int ickclock_boot(ickclock_t *tar) {
  mtx_t *k = &(tar->mlock);
  mtx_lock(k);

  if (tar->clock_mode != CLOCK_TICK && tar->clen == 0)
  { ickout(ICKOUT_ERROR, "clock[%p] -> clock length uninitialized.\n", tar); goto err_tUE; }
  if (tar->clock_stat == CLOCK_JOB) { internal_errmsg(tar, ERR_BUSYCLOCK);    goto err_tUE; }
  if (tar->clock_stat == CLOCK_ERR) { internal_errmsg(tar, ERR_MALFUNC);      goto err_tUE; }

  tar->clock_stat = CLOCK_JOB;
  tar->curr = time(0);


  int thrd_status, exit_status;
  thrd_status = thrd_create(&(tar->clock_thrd), internal_runtime, tar);

  if (thrd_status != thrd_success) { tar->clock_stat = CLOCK_ERR; }
  if (thrd_status == thrd_nomem) { goto err_tMemory; }
  if (thrd_status == thrd_error) { goto err_tUE; }

  if (tar->clock_mode == CLOCK_SEQ) {
    mtx_unlock(k);
    thrd_status = thrd_join(tar->clock_thrd, &exit_status);
    if (thrd_status == thrd_error) {
      thrd_detach(tar->clock_thrd); 
      return ICKUE;
    }
    return ICKSUCCESS;
  }
  mtx_unlock(k);
  return ICKSUCCESS;

  // exceptions
  err_tMemory:
    ickout(ICKOUT_ERROR, "clock[%p] - failed to start clock due to lack of memory.\n", tar);
    mtx_unlock(k);
    return ICKERR;
  err_tUE:
    mtx_unlock(k);
    return ICKUE;
}

int ickclock_shut(ickclock_t *tar) {
  mtx_t *k = &(tar->mlock);

  if (ickclock_getstat(tar) != CLOCK_JOB) {
    ickout(ICKOUT_WARN, "clock[%p] - cannot be terminated -> clock not running.\n", tar);
    return ICKUE;
  }
  ickout(ICKOUT_LOG, "clock[%p] - shutdown -> requested\n", tar);

  mtx_lock(k);
  tar->clock_stat = CLOCK_TERMINATE;
  mtx_unlock(k);

  int exit_code;

  if (tar->listen_thrd && thrd_join(tar->listen_thrd, &exit_code) != thrd_success)
  { return ICKUE; }

  if (tar->clock_mode == CLOCK_TICK)
  { goto ickclock_shut_finish; }

  if (tar->clock_thrd && thrd_join(tar->clock_thrd,  &exit_code) != thrd_success)
  { return ICKUE; }

ickclock_shut_finish:
  mtx_lock(k);
  tar->clock_stat = CLOCK_IDLE; 
  mtx_unlock(k);
  ickout(ICKOUT_LOG, "clock[%p] - shutdown -> completed\n", tar);
  return ICKSUCCESS;
}
