#include "sqm_curl.h"
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>


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

	double per = (double)(dlnow*100.0/dltotal);
    curl_easy_getinfo(handle, CURLINFO_TOTAL_TIME, &curtime);
    curl_easy_getinfo(handle, CURLINFO_SPEED_DOWNLOAD, &speed);
   // CURL_easy_getinfo(handle, )
    //_LOG("curtime %f,  speed %f",curtime, speed);

    if((curtime - pinfo->lasttime) >= MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL)
    {
        pinfo->lasttime = curtime;
        fprintf(stderr, "\n下载历时: %f, \n", curtime);

        fprintf(stderr,"已下载 %"CURL_FORMAT_CURL_OFF_T", 总大小 %"CURL_FORMAT_CURL_OFF_T\
                " , 速度 %.2fBytes/s, 进度 %g%%\n",dlnow, dltotal, speed, per);
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
                     pfunc_xferinfo xferinfo_callback, void* xinfer_data)
{
	CURLcode res;

    CURL *easy_handle = NULL;
    FILE *fp = NULL;
    char filepath[120] = {0};

    //curl easy 初始化
    easy_handle = curl_easy_init();

  //  getchar();
    printf("what the fuck! \n\n\n");

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
    _LOG("%s",filepath);
    fp = fopen(filepath, "ab+");
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
        strcat(userpwd, pwd);
        curl_easy_setopt(easy_handle, CURLOPT_USERPWD, userpwd);
        _LOG("password:  %s", userpwd);
    }

	//curl_easy_setopt(easy_handle, CURLOPT_RESUME_FROM_LARGE, use_resume?local_file_len:0);

	size_t filesize = 0;
	curl_easy_setopt(easy_handle, CURLOPT_HEADERFUNCTION, my_readheader_func);
	curl_easy_setopt(easy_handle, CURLOPT_HEADERDATA, &filesize);

	curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, my_write_func);

	curl_easy_setopt(easy_handle, CURLOPT_NOPROGRESS, 1L);

    progressstruct.curl = easy_handle;
    progressstruct.lasttime = 0;

    if(xferinfo_callback == NULL)
    {

        curl_easy_setopt(easy_handle, CURLOPT_XFERINFOFUNCTION, my_xferinfo_func);
        curl_easy_setopt(easy_handle, CURLOPT_XFERINFODATA, &progressstruct);
    }
    else
    {
        _LOG("progress struct");
        curl_easy_setopt(easy_handle, CURLOPT_XFERINFOFUNCTION, xferinfo_callback);
        if(xinfer_data != NULL)
            curl_easy_setopt(easy_handle, CURLOPT_XFERINFODATA, xinfer_data);

    }

	curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 1L);

	res = curl_easy_perform(easy_handle);

	if(res == CURLM_OK)
    {
        fprintf(stderr, "\n总用时: %f, \n", progressstruct.lasttime);
        fprintf(stderr,"已下载 %d, 总大小 %d , 平均速度 %.2fBytes/s, 进度 100%%\n",\
                filesize, filesize, filesize/progressstruct.lasttime);
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
