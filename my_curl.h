#ifndef MY_CURL_H_INCLUDED
#define MY_CURL_H_INCLUDED
#include "_LOG.h"
#include "curl/curl.h"

#define TIMEOUT 5
#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL 0.1

struct progressinfo {
  CURL *curl;
  double lasttime;
  curl_off_t contentlength;
};
// progress callback
typedef int (*pfunc_progress)(void *clientp,
                     double dltotal,
                     double dlnow,
                     double ultotal,
                     double ulnow);
// set header callback
typedef int (*pfunc_header)(struct curl_slist **headers);
//
typedef int (*pfunc_option)(CURL *curlhandle, char *url, void *data);
// read or write callback
typedef size_t (*pfunc_iorw)(void *ptr, size_t size, size_t nmemb, void *stream);

extern int im_curl_init(CURL **curlhandle);
extern int im_curl_clean(CURL *curlhandle);
extern int im_curl_perform(CURL *curlhandle);
extern int im_curl_option(CURL *curlhandle, char *url,
								void *data, pfunc_option im_option);
extern int im_construct_header(struct curl_slist **headers,
										pfunc_header im_header_func);
extern int im_customer_header_back(CURL *curlhandle, char *url, void *data);
extern int my_curl_download_back(CURL *curlhandle, char *url, void *data);


#endif // MY_CURL_H_INCLUDED
