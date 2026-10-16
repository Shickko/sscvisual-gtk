#include <curl/curl.h>
#include "icksession.h"
#include "ickutils.h"
#include "ickclock.h"

int main(int argc, char* argv[]) {
  ickloglevel_set(ICKOUT_LOG);

  /* CURL Initialization */
  CURLcode curl_status_code = curl_global_init(CURL_GLOBAL_SSL); // NON WIN-32
  if (curl_status_code == CURLE_OK) 
  {  goto winstart; }
  return curl_status_code;
winstart:
  return icksession_init(argc, argv);
}
