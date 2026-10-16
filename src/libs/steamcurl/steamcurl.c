#include <json-c/json.h>
#include <curl/curl.h>
#include <steamcurl.h>
#include <ickutils.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct steamcli_t {
  size_t rs_size;
  char *response;
  char *auth_key;
  char *auth_id;
  CURLM *curlm; 
};

static size_t steamapi_callback(char *strout, size_t sizeout, size_t nmemb, void *dt) {
  steamcli_t *client;
  size_t bytes;

  client = (steamcli_t *)dt;
  bytes = sizeout * nmemb;

  char *rs = realloc(client->response, client->rs_size + bytes + 1);
  if (rs == NULL) { return 0; }
  client->response = rs;

  memcpy(client->response + client-> rs_size, strout, bytes);
  client->rs_size += bytes;
  client->response[client->rs_size] = '\0';

  return bytes;
}

static int steamcli_fetch(steamcli_t *cli, const char* url) {
  //buffer reset
  if (cli->response) { 
    //cli->response[0] = '\0'; 
    free(cli->response);
  } 
  cli->response = NULL;
  cli->rs_size = 0;
  //curl main
  CURL *curl;
  CURLcode k;

  curl = curl_easy_init();
  if (!curl) { 
    ickout(ICKOUT_ERROR, "steamcli[%p] - curl failed to init\n", cli);

    return ICKERR;
  }

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,   steamapi_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA,       cli);
  curl_easy_setopt(curl, CURLOPT_URL,             url);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT,  2L);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT,         2L);
  k = curl_easy_perform(curl);

  if (k != CURLE_OK) {
    ickout(
        ICKOUT_ERROR, 
        "steamcli[%p] - curl failure - %s\n", 
        cli, curl_easy_strerror(k)
    );
    curl_easy_cleanup(curl);

    return ICKERR;
  }
  curl_easy_cleanup(curl);

  return ICKSUCCESS;
}

static int steamcli_write(steamcli_t *cli, const char* url, const char* dir) {
  if (steamcli_fetch(cli, url) != ICKSUCCESS) { return ICKERR; }
  if (cli->response == NULL)                  { return ICKERR; }
  if (cli->rs_size == 0)                      { return ICKERR; }
  FILE *file_readin = fopen(dir, "r");
  if (file_readin) {
    fclose(file_readin);
    return ICKSUCCESS;
  }

  FILE *file = fopen(dir, "wb");
  if (!file) { return ICKERR; }

  fwrite(cli->response, 1, cli->rs_size, file);
  fclose(file);

  return ICKSUCCESS;
}

steamcli_t *steamcli_create(const char *key, const char *uid) {
  puts("Attempting to create new client\n");
  if (key == NULL || uid == NULL)
  { return NULL; }
  steamcli_t *cli = malloc(sizeof(steamcli_t));
  cli->response = malloc(sizeof(char *));
  cli->curlm = curl_multi_init();
  cli->auth_key = key;
  cli->auth_id  = uid;

  /* attmpts to get self data to test api status */
  if (!(cli -> curlm)) { return NULL; }

  //steamprofilsteamcli_getplayer(cli, uid);

 ickout(ICKOUT_LOG, "steamcli[%p] startup - success\n", cli);
 return cli;
}

int steamcli_close(steamcli_t *cli) {
  if (cli == NULL || cli->response == NULL) { return ICKUE; }
  curl_multi_cleanup(cli->curlm);

  ickout(ICKOUT_LOG, "steamcli[%p] cleanup - success\n", cli);
  free(cli->response);
  free(cli);
  return ICKSUCCESS;
}

const char* steamcli_getauthid(steamcli_t *cli) {
  if (cli == NULL) { return NULL; }
  return cli->auth_id;
}

steamprofile_t steamcli_getplayer(steamcli_t *cli, char *uid) {
  /* struct main */
  char url[512];

  snprintf(url, sizeof(url),
    "https://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/?key=%s&steamids=%s",
    cli->auth_key,
    uid);

  /* curl fetcher */
  int curl_status = steamcli_fetch(cli, url);
  if (curl_status != ICKSUCCESS) {
    goto err_profile_handler;
  }

  /* json handler */
  struct json_object *json_response_root = json_tokener_parse(cli->response);
  struct json_object *json_response_temp = NULL;
  struct json_object *json_response_main = NULL;
  struct json_object *json_response_plys = NULL;
  struct json_object *json_response_ply  = NULL;

  steamprofile_t profile;
  
  if (json_response_root == NULL) 
  { goto err_profile_json_failure; }
  if (!json_object_object_get_ex(json_response_root, "response", &json_response_main))
  { goto err_profile_json_failure; }
  if (!json_object_object_get_ex(json_response_main, "players", &json_response_plys))
  { goto err_profile_json_failure; }
  if (json_object_get_type(json_response_plys) != json_type_array)
  { goto err_profile_json_failure; }
  if (json_object_array_length(json_response_plys) == 0)
  { goto err_profile_json_failure; }

  json_response_ply = json_object_array_get_idx(json_response_plys, 0);
  if (json_response_ply == NULL)
  { goto err_profile_json_failure; }

  /* [[ Public */
  if (json_object_object_get_ex(json_response_ply, "personastate", &json_response_temp)) { 
    profile.steamstatus = json_object_get_int(json_response_temp); 
  }
  if (json_object_object_get_ex(json_response_ply, "personaname", &json_response_temp)) { 
    const char* steam_usrname = json_object_get_string(json_response_temp);
    profile.steamname = strdup(steam_usrname ? steam_usrname : "?");
  } else {
    profile.steamname = strdup("?");
  }
  if (json_object_object_get_ex(json_response_ply, "steamid", &json_response_temp)) { 
    const char* steam_usrid = json_object_get_string(json_response_temp);
    profile.steamid = strdup(steam_usrid ? steam_usrid : "?");
  } else {
    profile.steamid = strdup("?");
  }
  if (json_object_object_get_ex(json_response_ply, "avatarfull", &json_response_temp)) { 
    profile.avatarfull = json_object_get_string(json_response_temp); 
    char fdir[512];
    sprintf(fdir, "%s.jpg", profile.steamid);

    steamcli_write(
        cli, 
        profile.avatarfull, 
        fdir
    );
  }
  /* [[ Private */
  if (json_object_object_get_ex(json_response_ply, "communityvisibilitystate", &json_response_temp) &&
      json_object_get_int(json_response_temp) != 3) { goto err_profile_handler_private; }

  json_object_put(json_response_root);
  return profile;

/* error handling */
err_profile_json_failure:
  if (json_response_root) { json_object_put(json_response_root); }
  goto err_profile_handler;
err_profile_handler:
  const char* errstr_corrupt = "[CORRUPTED DATA]";
  profile.steamstatus = -1;
  profile.steamname   = strdup(errstr_corrupt);
  profile.steamid     = strdup(errstr_corrupt);
  profile.lastlogoff  = strdup(errstr_corrupt);
  profile.avatarfull  = NULL;
  return profile;
err_profile_handler_private:
  if (json_response_root) { json_object_put(json_response_root); }
  const char* errstr_private = "[PRIVATE DATA]";
  return profile;
}

void steamcli_putplayer(steamprofile_t *profile) {
  if (!profile) { return; }
  //free(profile->steamavatar);
  free(profile->steamname);
  free(profile->steamid);
  //free(profile->lastlogoff);

  profile->steamavatar = NULL;
  profile->steamname   = NULL;
  profile->steamid     = NULL;
  profile->lastlogoff  = NULL;
  profile->avatarfull  = NULL;
}
