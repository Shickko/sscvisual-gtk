#ifndef ICKLIB_H_SESSION_CONF
#define ICKLIB_H_SESSION_CONF
#define GLOB_ICKCONF_DIR  ("/home/Syuu/.config/shickky")
#define GLOB_ICKCONF_PATH ("/home/Syuu/.config/shickky/shickky.conf")
#include "ickstorage.h"

int  ickconf_create();
int  ickconf_read (ickdict_t *conf);
void ickconf_close(ickdict_t *conf);

void  ickconf_writev(ickdict_t *conf, const char* label, const char* dt);
char* ickconf_readv (ickdict_t *conf, const char* label);
#endif
