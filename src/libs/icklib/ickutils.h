#ifndef ICKLIB_H_UTILS
#define ICKLIB_H_UTILS
#define ICKSUCCESS   0
#define ICKERR      -1
#define ICKUE       -2
#include <stddef.h>
#include <math.h>

typedef enum {
  ICKOUT_LOG,
  ICKOUT_WARN,
  ICKOUT_ERROR,
  ICKOUT_SILENT,
} ickloglevel_t;

typedef struct {
  double x;
  double y;
} ickpoint_t;

/* loglevel filtered output */
size_t ickout
  (ickloglevel_t i, const char* str, ...) __attribute__((format(printf, 2, 3)));
size_t ickout_p
  (ickloglevel_t i, const char* str, ...) __attribute__((format(printf, 2, 3)));

extern ickloglevel_t ickloglevel;
static inline ickloglevel_t ickloglevel_get()
{ return ickloglevel; }
static inline void ickloglevel_set(ickloglevel_t i)
{ ickloglevel = i; }

/* implementation - ickpoint_t */
ickpoint_t ickpoint(double px, double py);
static inline double getdelta(const ickpoint_t *i)
{ return sqrt(i->x * i->x + i->y * i->y); }
static inline double getdist(const ickpoint_t *i, const ickpoint_t *j)
{ return sqrt(pow(j->x - i->x, 2) + pow(j->y - i->y, 2)); }
#endif
