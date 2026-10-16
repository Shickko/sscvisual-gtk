#include <dirconf/ickconf.h>
#include <ickstorage.h>
#include <ickutils.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

static const char* keyrange[] = {
  "usr_authkey",
  "usr_authid"
};

static int ickconf_checkdir(const char* dir) {
  struct stat info;
  if (stat(dir, &info) != 0) {
    ickout(ICKOUT_WARN, "ickconf - directory '%s' does not exist\n", dir);
    return 0;
  } else {
    return (info.st_mode & S_IFDIR) ? 1 : 0;
  }
}

int ickconf_create() {
  ickdict_t *conf;
  FILE*     fconf;
  
  if (ickconf_checkdir(GLOB_ICKCONF_DIR) == 0) { 
    mkdir(GLOB_ICKCONF_DIR, S_IRWXU); 
    if (ickconf_checkdir(GLOB_ICKCONF_DIR) == 0)
    { return ICKERR; } // Unable to create dir
  }

  fconf = fopen(GLOB_ICKCONF_PATH, "w");
  conf  = ickdict();

  if (!fconf) {
    ickdict_smash(conf);
    return ICKERR;
  }

  for (int i = 0; i < sizeof(keyrange) / sizeof(keyrange[0]); i++ ) {
    if (ickdict_attach(conf, keyrange[i], " ") != ICKSUCCESS)
    { return ICKERR; }
    fprintf(fconf, "%s %s\n", keyrange[i], (char*)ickdict_read(conf, keyrange[i]));
  }

  ickdict_smash(conf);
  fclose(fconf);

  return ICKSUCCESS;
}

int ickconf_read(ickdict_t *conf) {
  FILE*     fconf;

  fconf = fopen(GLOB_ICKCONF_PATH, "r");
  if (!fconf) { return ICKERR;}

  char line[1024];
  while (fgets(line, sizeof(line), fconf)) {
    char* dictkey = calloc(50, sizeof(char));
    char* dictvar = calloc(50, sizeof(char));
    int rs = sscanf(line, "%49s %49s", dictkey, dictvar);
    ickdict_attach(conf, dictkey, dictvar);
  }

 fclose(fconf);

  return ICKSUCCESS;
}

void ickconf_close(ickdict_t *conf) {
  /*
  for (int i = 0; i < sizeof(keyrange) / sizeof(keyrange[0]); i++) {
    char* tar = (char* )ickdict_detach(conf, keyrange[i]);
    ickout(ICKOUT_LOG, "Removing %p: %s\n", tar, tar);
    if (tar) { free(tar); }
    tar = NULL;
  } */
  ickdict_smash(conf);
}

void ickconf_writev(ickdict_t *conf, const char* label, const char* dt) {
  if (conf == NULL) { return; }
  FILE* fconf = fopen(GLOB_ICKCONF_PATH, "w");
  if (fconf) { fprintf(fconf, "%s %s\n", label, dt); }
  ickdict_attach(conf, label, dt);
}

char* ickconf_readv(ickdict_t *conf, const char* label) {
  if (conf == NULL) { return NULL; }
  char* rs = (char* )ickdict_read(conf, label);
  return rs;
}
