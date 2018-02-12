#include "mylibcurl.h"
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>


static void time2str(char *r, curl_off_t seconds)
{
  curl_off_t d, h, m, s;
  if(seconds <= 0) {
    strcpy(r, "--:--:--");
    return;
  }
  h = seconds / CURL_OFF_T_C(3600);
  if(h <= CURL_OFF_T_C(99)) {
    m = (seconds - (h*CURL_OFF_T_C(3600))) / CURL_OFF_T_C(60);
    s = (seconds - (h*CURL_OFF_T_C(3600))) - (m*CURL_OFF_T_C(60));
    snprintf(r, 9, "%2" CURL_FORMAT_CURL_OFF_T ":%02" CURL_FORMAT_CURL_OFF_T
             ":%02" CURL_FORMAT_CURL_OFF_T, h, m, s);
  }
  else {
    /* this equals to more than 99 hours, switch to a more suitable output
       format to fit within the limits. */
    d = seconds / CURL_OFF_T_C(86400);
    h = (seconds - (d*CURL_OFF_T_C(86400))) / CURL_OFF_T_C(3600);
    if(d <= CURL_OFF_T_C(999))
      snprintf(r, 9, "%3" CURL_FORMAT_CURL_OFF_T
               "d %02" CURL_FORMAT_CURL_OFF_T "h", d, h);
    else
      snprintf(r, 9, "%7" CURL_FORMAT_CURL_OFF_T "d", d);
  }
}

static char *max5data(curl_off_t bytes, char *max5)
{
#define ONE_KILOBYTE  CURL_OFF_T_C(1024)
#define ONE_MEGABYTE (CURL_OFF_T_C(1024) * ONE_KILOBYTE)
#define ONE_GIGABYTE (CURL_OFF_T_C(1024) * ONE_MEGABYTE)
#define ONE_TERABYTE (CURL_OFF_T_C(1024) * ONE_GIGABYTE)
#define ONE_PETABYTE (CURL_OFF_T_C(1024) * ONE_TERABYTE)

  if(bytes < CURL_OFF_T_C(100000))
    snprintf(max5, 6, "%5" CURL_FORMAT_CURL_OFF_T, bytes);

  else if(bytes < CURL_OFF_T_C(10000) * ONE_KILOBYTE)
    snprintf(max5, 6, "%4" CURL_FORMAT_CURL_OFF_T "k", bytes/ONE_KILOBYTE);

  else if(bytes < CURL_OFF_T_C(100) * ONE_MEGABYTE)
    /* 'XX.XM' is good as long as we're less than 100 megs */
    snprintf(max5, 6, "%2" CURL_FORMAT_CURL_OFF_T ".%0"
             CURL_FORMAT_CURL_OFF_T "M", bytes/ONE_MEGABYTE,
             (bytes%ONE_MEGABYTE) / (ONE_MEGABYTE/CURL_OFF_T_C(10)) );

#if (CURL_SIZEOF_CURL_OFF_T > 4)

  else if(bytes < CURL_OFF_T_C(10000) * ONE_MEGABYTE)
    /* 'XXXXM' is good until we're at 10000MB or above */
    snprintf(max5, 6, "%4" CURL_FORMAT_CURL_OFF_T "M", bytes/ONE_MEGABYTE);

  else if(bytes < CURL_OFF_T_C(100) * ONE_GIGABYTE)
    /* 10000 MB - 100 GB, we show it as XX.XG */
    snprintf(max5, 6, "%2" CURL_FORMAT_CURL_OFF_T ".%0"
             CURL_FORMAT_CURL_OFF_T "G", bytes/ONE_GIGABYTE,
             (bytes%ONE_GIGABYTE) / (ONE_GIGABYTE/CURL_OFF_T_C(10)) );

  else if(bytes < CURL_OFF_T_C(10000) * ONE_GIGABYTE)
    /* up to 10000GB, display without decimal: XXXXG */
    snprintf(max5, 6, "%4" CURL_FORMAT_CURL_OFF_T "G", bytes/ONE_GIGABYTE);

  else if(bytes < CURL_OFF_T_C(10000) * ONE_TERABYTE)
    /* up to 10000TB, display without decimal: XXXXT */
    snprintf(max5, 6, "%4" CURL_FORMAT_CURL_OFF_T "T", bytes/ONE_TERABYTE);

  else
    /* up to 10000PB, display without decimal: XXXXP */
    snprintf(max5, 6, "%4" CURL_FORMAT_CURL_OFF_T "P", bytes/ONE_PETABYTE);

    /* 16384 petabytes (16 exabytes) is the maximum a 64 bit unsigned number
       can hold, but our data type is signed so 8192PB will be the maximum. */

#else

  else
    snprintf(max5, 6, "%4" CURL_FORMAT_CURL_OFF_T "M", bytes/ONE_MEGABYTE);

#endif

  return max5;
}

static size_t my_write_func(void *ptr, size_t size, size_t nmemb, void *stream)
{
  return fwrite(ptr, size, nmemb, (FILE*)stream);
}

static size_t my_readheader_func(void *ptr, size_t size, size_t nmemb, void *stream)
{
	int ret = -1;
	long len = 0;

	ret = sscanf(ptr, "Content-Length: %ld\n", &len);
	if (ret)
	{
      *((long *) stream) = len;
	}
	return size * nmemb;
}

static int my_xferinfo_func(void *clientp,
                     curl_off_t  dltotal,
                     curl_off_t  dlnow,
                     curl_off_t  ultotal,
                     curl_off_t  ulnow)
{

    struct progressinfo *pinfo = (struct progressinfo *)(clientp);
    CURL* handle = pinfo->curl ;

    double curtime = 0;
    double speed =0;


    char max5[6][10];
   // char time_left[10];
   // char time_total[10];
    char time_spent[10];
    curl_off_t avrspeed=0;
    curl_off_t timespent=0;
    curl_off_t totalper=0;

    curl_easy_getinfo(handle, CURLINFO_TOTAL_TIME, &curtime);
    curl_easy_getinfo(handle, CURLINFO_SPEED_DOWNLOAD, &speed);
    timespent = (curl_off_t)curtime;

    if((curtime - pinfo->lasttime) >= MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL)
    {
    if(dltotal > CURL_OFF_T_C(10000))
        totalper = dlnow /(dltotal/CURL_OFF_T_C(100));
    else if(dltotal > CURL_OFF_T_C(0))
        totalper= (dlnow*100) / dltotal;

        avrspeed = (curl_off_t)((double)dlnow/curtime);
        time2str(time_spent, timespent);
        pinfo->lasttime = curtime;

        fprintf(stderr,
              "\nPID\tTotal \t Received/Totalsize\tSpeed/s\tTimeSpent\n");
                 
        fprintf(stderr,"%d \t%3"CURL_FORMAT_CURL_OFF_T"%%\t   %s /%s \t %s\t%s\n",
                //%.2fBytes/s, 进度 %g%%\n",
                pinfo->pid,
                totalper,
                max5data(dlnow, max5[0]), 
                max5data(dltotal, max5[2]),
               // max5data(avrspeed, max5[3]), 
               
                max5data((curl_off_t)speed, max5[5]), /* current speed */
                time_spent);    /* 8 letters */                /* time spent */
                 
    }

    return 0;
}

static int get_localsize(const char* localpath)
{
    struct stat file_info;

    if(stat(localpath, &file_info) == 0)
	{
		return  file_info.st_size;
	}

    return -1;
}

int sqm_curl_init(void)
{
	CURLcode res;

	res = curl_global_init(CURL_GLOBAL_ALL);

	if (res != CURLE_OK)
	{
		_LOG("curl_global_init fail:%s", curl_easy_strerror(res));
		return -1;
	}

	return res;
}

void sqm_curl_clean(void)
{
	curl_global_cleanup();
}

int sqm_filedownload(const char *requestURL, const char *saveto, \
                     const char *user, const char *pwd, \
                     pfunc_xferinfo xferinfo_callback, void* xinfer_data,\
                     pthread_mutex_t* plock, int pid)
{
	CURLcode res;

    CURL *easy_handle = NULL;
    FILE *fp = NULL;
    char filepath[120] = {0};
   // pid = pid;
    //curl easy 初始化
    easy_handle = curl_easy_init();


    if ( NULL == easy_handle )
    {
        _LOG("curl_easy_init error");
        return -1;
    }

    if(saveto == NULL)

    {
        strcpy(filepath,"file downloaded");
    }
    else
    {
            //_LOG("%s", saveto);
        strcpy(filepath, saveto);
    }
    //_LOG("%s",filepath);
    pthread_mutex_lock(plock);
    fp = fopen(filepath, "wb+");
    pthread_mutex_unlock(plock);
	if (fp == NULL)
	{
		_LOG("open file \"%s\" fail", filepath);
		fclose(fp);
		return -1;
	}

    struct progressinfo progressstruct;

    //断点续传
    int use_resume = 0;
	curl_off_t local_file_len = get_localsize(filepath);
	if(local_file_len > 0)
    {
        use_resume	= 1;
    }
   // _LOG("%ld",local_file_len);


	curl_easy_setopt(easy_handle, CURLOPT_URL, requestURL);
	curl_easy_setopt(easy_handle, CURLOPT_CONNECTTIMEOUT, TIMEOUT);

	if(user!=NULL && pwd!=NULL)
    {
        char userpwd[120];
        strcpy(userpwd, user);
		strcat(userpwd,":");
        strcat(userpwd, pwd);
        curl_easy_setopt(easy_handle, CURLOPT_USERPWD, userpwd);
    }

	//curl_easy_setopt(easy_handle, CURLOPT_RESUME_FROM_LARGE, use_resume?local_file_len:0);

	size_t filesize = 0;
	curl_easy_setopt(easy_handle, CURLOPT_HEADERFUNCTION, my_readheader_func);
	curl_easy_setopt(easy_handle, CURLOPT_HEADERDATA, &filesize);

	curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, my_write_func);

	curl_easy_setopt(easy_handle, CURLOPT_NOPROGRESS, 0L);

    progressstruct.curl = easy_handle;
    progressstruct.lasttime = 0;
    progressstruct.pid = pid;

    if(xferinfo_callback == NULL)
    {

        curl_easy_setopt(easy_handle, CURLOPT_XFERINFOFUNCTION, my_xferinfo_func);
        curl_easy_setopt(easy_handle, CURLOPT_XFERINFODATA, &progressstruct);
    }
    else
    {
        curl_easy_setopt(easy_handle, CURLOPT_XFERINFOFUNCTION, xferinfo_callback);
        if(xinfer_data != NULL)
            curl_easy_setopt(easy_handle, CURLOPT_XFERINFODATA, xinfer_data);

    }
    //ssh
    curl_easy_setopt(easy_handle, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);

	curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(easy_handle, CURLOPT_NOSIGNAL, 1L);

	res = curl_easy_perform(easy_handle);

	if(res == CURLM_OK)
    {
        //fprintf(stderr, "\n总用时: %f, \n", progressstruct.lasttime);
        //fprintf(stderr," %ld, 总大小 %ld , 平均速度 %.2fBytes/s, 进度 100%%\n",\
           //     filesize, filesize, filesize/progressstruct.lasttime);
        double curtime = 0;
        double speed =0;
        char str10[4][10];
        curl_easy_getinfo(easy_handle, CURLINFO_TOTAL_TIME, &curtime);
        curl_easy_getinfo(easy_handle, CURLINFO_SPEED_DOWNLOAD, &speed);
        time2str(str10[3],(curl_off_t)curtime);
        fprintf(stderr,
              "\nPID\tTotal \t Received/Totalsize\tSpeed/s\tTimeSpent");
              fprintf(stderr,"\tTask\n");
                 
        fprintf(stderr,"%d \t%3"CURL_FORMAT_CURL_OFF_T"%%\t   %s /%s \t %s\t%s",
                //%.2fBytes/s, 进度 %g%%\n",
                pid,
                100,
                max5data((curl_off_t)filesize, str10[0]), 
                max5data((curl_off_t)filesize, str10[1]),
                max5data((curl_off_t)speed, str10[2]), /* current speed */
                str10[3]);       /* time spent */
                fprintf(stderr,"\tComplete\n");
    }

    if (fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}

	curl_easy_cleanup(easy_handle);
    easy_handle = NULL;

	return res;
}
