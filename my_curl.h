#ifndef SQM_CURL_H_INCLUDED
#define SQM_CURL_H_INCLUDED
#include <pthread.h>
#include "_LOG.h"
#include "curl/curl.h"

#define TIMEOUT 5
#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL 0.25

/* this is how the CURLOPT_XFERINFOFUNCTION callback works */
typedef int (*pfunc_xferinfo)(void *p,
                    curl_off_t dltotal, curl_off_t dlnow,
                    curl_off_t ultotal, curl_off_t ulnow);

struct progressinfo {
  CURL *curl;
  double lasttime;
  pthread_mutex_t *plock;
//  pfunc_xferinfo xferinfo_callback; //not used
};


/** \brief ȫ�ֳ�ʼ�� ����CURL_GLOBAL_ALL
 *
 * \param void
 * \return int
 * �ɹ�CURL_OK
 */
int sqm_curl_init(void);

/** \brief ȫ�����
 *
 * \param void
 * \return void
 *
 */
void sqm_curl_clean(void);
int sqm_filedownload(const char *requestURL, const char *saveto, \
                     const char *user, const char *password, \
                     pfunc_xferinfo xferinfo_callback, void* xinfer_data, \
                     pthread_mutex_t* plock);

#endif // SQM_CURL_H_INCLUDED
