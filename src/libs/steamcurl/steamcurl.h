#ifndef STEAMCURL_H
#define STEAMCURL_H
#define STEAMCURL_CLI_OK  0
#define STEAMCURL_CLI_ERR 1
#define STEAMCURL_CLI_NIL 2
#include <stdint.h>
typedef struct steamcli_t steamcli_t;
typedef struct {
  int   steamstatus;
  char *steamavatar;
  char *steamname;
  char *steamid;
  char *lastlogoff;
  char *avatarfull;
} steamprofile_t;

steamcli_t *steamcli_create(const char *key, const char *uid);
int steamcli_close(steamcli_t *cli);

const char* steamcli_getauthid(steamcli_t *cli);

steamprofile_t steamcli_getplayer(steamcli_t *cli, char *uid);
void steamcli_putplayer(steamprofile_t *profile);

#endif
