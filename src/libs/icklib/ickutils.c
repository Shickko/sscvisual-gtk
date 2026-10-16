#include <ickutils.h>
#include <ickclock.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

ickloglevel_t ickloglevel;

ickpoint_t ickpoint(double px, double py) {
  ickpoint_t p;
  p.x = px;
  p.y = py;
  return p;
}

static void ickout_seth(ickloglevel_t i, char** prefix) {
  if (prefix == NULL) { return; }
  switch(i) {
    case ICKOUT_LOG:  { *prefix = "[ICKOUT] "  ; break; }
    case ICKOUT_WARN: { *prefix = "[ICKWAN] "  ; break; }
    case ICKOUT_ERROR:{ *prefix = "[ICKERR] "  ; break; }
    default:          { *prefix = ""           ; break; }
  }
}

size_t ickout(ickloglevel_t i, const char* str, ...) {
  if (ickloglevel_get() > i) { return 0; }

  size_t buffer_len;
  char* _hd;
  va_list v;

  ickout_seth(i, &_hd);

  buffer_len = 0;
  buffer_len += printf("%s", _hd);

  va_start(v, str);
  buffer_len += vprintf(str, v);
  va_end(v);

  return buffer_len;
}

size_t ickout_p(ickloglevel_t i, const char* str, ...) {
  if (ickloglevel_get() > i) { return 0; }

  size_t buffer_len;
  char* _buffer;
  char* _hd;
  va_list v;

  ickout_seth(i, &_hd);

  buffer_len = 0;

  for (int i = 0; i < strlen(_hd); i++) {
    ickwaitn(CLOCK_UPDATE_FQ);
    buffer_len += printf("%c", _hd[i]);
    fflush(stdout);
  }

  /* The following part set the maximum output string size to 1024 byte,
   * since I have no better idea to allocate an accurate 
   * data size for _buffer other than getting each arg in
   * va_list v and sums up their size, however, since the
   * arg type may not be just for string but also may contain
   * interger or float etc. I don't feel like writing a 20+ lines
   * switch to detect args type and assign size to the buffer */

  /* This may be bad but for the current state it works so
   * I don't feel like updating this in a while.
   * But do aware that this may cause unexpected issues for 
   * long strings. It is also possible to cut full string in chunks
   * for optimizations at this point. */

  _buffer = (char* )malloc(1024 * sizeof(char));

  va_start(v, str);
  vsprintf(_buffer, str, v);
  va_end(v);

  for (int i = 0; i < strlen(_buffer); i++) {
    ickwaitn(CLOCK_UPDATE_FQ);
    buffer_len += printf("%c", _buffer[i]);
    fflush(stdout);
  }

  free(_buffer);
  
  return buffer_len;
}
