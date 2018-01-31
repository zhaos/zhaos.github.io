#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "my_curl.h"

// write to file
size_t my_write_func(void *ptr, size_t size, size_t nmemb, void *stream)
{
  return fwrite(ptr, size, nmemb, (FILE*)stream);
}
// read to file
size_t my_read_func(void *ptr, size_t size, size_t nmemb, void *stream)
{
  return fread(ptr, size, nmemb, (FILE*)stream);
}

int im_curl_init(CURL **curlhandle)
{
	CURLcode res;

	if (curlhandle == NULL)
	{
		_LOG("PARAM ERROR\n");
		return -1;
	}

	res = curl_global_init(CURL_GLOBAL_ALL);
	if (res != CURLE_OK)
	{
		_LOG("curl_global_init fail:%s", curl_easy_strerror(res));
		return -1;
	}

	*curlhandle = curl_easy_init();
	if (*curlhandle == NULL)
	{
		_LOG("curl_easy_init error\n");
		return -1;
	}

	return res;
}

int im_curl_clean(CURL *curlhandle)
{
	if (curlhandle == NULL)
	{
		_LOG("PARAM ERROR");
		return -1;
	}

	curl_easy_cleanup(curlhandle);
	curl_global_cleanup();
}

int im_curl_perform(CURL *curlhandle)
{
	CURLcode res;
	long retcode = 0;

	if (curlhandle == NULL)
	{
		_LOG("PARAM ERROR");
		return -1;
	}

	res = curl_easy_perform(curlhandle);
	if (res != CURLE_OK)
	{
		  _LOG("%s\n", curl_easy_strerror(res));
		  return -1;
	}

	res = curl_easy_getinfo(curlhandle, CURLINFO_RESPONSE_CODE , &retcode);
	if ((res == CURLE_OK)&& ((retcode == 200) || (206 == retcode)))//  206 resume download
	{
		_LOG("download OK retcode:%d", retcode);
	}
	else
	{
		_LOG("fail %s  retcode:%d\n", curl_easy_strerror(res), retcode);
		return -1;
	}

	return 0;

}

int im_curl_option(CURL *curlhandle, char *url, void *data, pfunc_option im_option)
{
	return im_option(curlhandle, url, data);
}


int im_construct_header(struct curl_slist **headers, pfunc_header im_header_func)
{
	return im_header_func(headers);
}


int im_curl_get_back(CURL *curlhandle, char *url, void *data)
{
	int ret = -1;

	if (!curlhandle || !url)
	{
		_LOG("Bad param!!!");
		return -1;
	}

	curl_easy_setopt(curlhandle, CURLOPT_URL, url);
	curl_easy_setopt(curlhandle, CURLOPT_CONNECTTIMEOUT, TIMEOUT);  // 设置连接超时，单位秒
	curl_easy_setopt(curlhandle, CURLOPT_USERPWD, "zry:123456");

	curl_easy_setopt(curlhandle, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curlhandle, CURLOPT_VERBOSE, 1L);

	ret = im_curl_perform(curlhandle);


	return ret;

}

int im_curl_post_back(CURL *curlhandle, char *url, void *data)
{
	char *postdata = NULL;
	int ret = -1;

	if (!curlhandle || !url || !data)
	{
		_LOG("Bad param!!!");
		return -1;
	}

	postdata = (char *)data;

	curl_easy_setopt(curlhandle, CURLOPT_URL, url);
	curl_easy_setopt(curlhandle, CURLOPT_CONNECTTIMEOUT, TIMEOUT);  //
	curl_easy_setopt(curlhandle, CURLOPT_USERPWD, "zry:123456");

	curl_easy_setopt(curlhandle, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curlhandle, CURLOPT_VERBOSE, 1L);

	curl_easy_setopt(curlhandle, CURLOPT_POSTFIELDS, postdata);
	//	curl_easy_setopt(curl, CURLOPT_POST, 1L);

	ret = im_curl_perform(curlhandle);

	return ret;

}

int im_curl_head_back(CURL *curlhandle, char *url, void *data)
{
	int ret = -1;
	if (!curlhandle || !url)
	{
		_LOG("Bad param!!!");
		return -1;
	}

	curl_easy_setopt(curlhandle, CURLOPT_URL, url);
	curl_easy_setopt(curlhandle, CURLOPT_HEADER, NULL);
	curl_easy_setopt(curlhandle, CURLOPT_NOBODY, 1L);
	curl_easy_setopt(curlhandle, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curlhandle, CURLOPT_VERBOSE, 1L);
	ret = im_curl_perform(curlhandle);

	return ret;
}


int im_header_back(struct curl_slist **headers)
{
	*headers = curl_slist_append(*headers, "Hey-server-hey: how are you?");
	*headers = curl_slist_append(*headers, "X-silly-content: haha");
	return 0;
}

int im_customer_header_back(CURL *curlhandle, char *url, void *data)
{
	struct curl_slist *headers = NULL;
	int ret = -1;
	pfunc_header p_headfunc = (pfunc_header) data;

	if (!curlhandle || !url || !data)
	{
		_LOG("Bad param!!!");
		return -1;
	}

	im_construct_header(&headers, p_headfunc);

	curl_easy_setopt(curlhandle, CURLOPT_URL, url);
	curl_easy_setopt(curlhandle, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curlhandle, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curlhandle, CURLOPT_VERBOSE, 1L);

	ret = im_curl_perform(curlhandle);

	curl_slist_free_all(headers); /* free the header list */

	return ret;
}


size_t im_get_contentlength(void *ptr, size_t size, size_t nmemb, void *stream)
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

size_t my_get_contentLength(double *filesize, char *url)
{

    int err = 0;
	CURL *curlhandle = NULL;
	CURLcode res;

	err = im_curl_init(&curlhandle);
	if (err < 0)
	{
		_LOG("im_curl_init err");
		return err;
	}

    curl_easy_setopt(curlhandle, CURLOPT_URL, url);
    curl_easy_setopt(curlhandle, CURLOPT_HEADER, 1L);
    curl_easy_setopt(curlhandle, CURLOPT_NOBODY, 1L);

    res = curl_easy_perform(curlhandle);
    if (res == CURLE_OK)
    {
        curl_easy_getinfo(curlhandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, filesize);
        _LOG("my filesize: %f", *filesize);
    }
    else
    {
        _LOG("%s\n", curl_easy_strerror(res));
        err = -1;
    }

    im_curl_clean(curlhandle);
    return err;
}

int im_get_remote_file_len_back(long *filesize, char *url)
{
	int err = 0;
	CURL *curlhandle = NULL;
	CURLcode res;

	err = im_curl_init(&curlhandle);
	if (err < 0)
	{
		_LOG("im_curl_init err");
		return err;
	}
	curl_easy_setopt(curlhandle, CURLOPT_URL, url);
	curl_easy_setopt(curlhandle, CURLOPT_NOBODY, 1L);
	curl_easy_setopt(curlhandle, CURLOPT_CONNECTTIMEOUT, TIMEOUT);  // 设置连接超时，单位秒

	curl_easy_setopt(curlhandle, CURLOPT_HEADERFUNCTION, im_get_contentlength);
	curl_easy_setopt(curlhandle, CURLOPT_HEADERDATA, filesize);
	curl_easy_setopt(curlhandle, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curlhandle, CURLOPT_VERBOSE, 0L);

	res = curl_easy_perform(curlhandle);
	if (res != CURLE_OK)
	{
		  _LOG("%s\n", curl_easy_strerror(res));
		  err = -1;
	}
	//_LOG("im filesize:%d", *filesize);

	im_curl_clean(curlhandle);
	return err;
}

size_t my_download_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
	int written = fwrite(ptr, size, nmemb, (FILE *)stream);
	//_LOG("%ld",written);
	return written;
}

int my_progress_func(void *clientp,
                     curl_off_t  dltotal, /* dltotal */
                     curl_off_t  dlnow, /* dlnow */
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

int my_get_localsize(const char* localpath)
{
    struct stat file_info;

    if(stat(localpath, &file_info) == 0)
	{
		return  file_info.st_size;
	}

    return -1;
}

int my_curl_download_back(CURL *curlhandle, char *url, void *data)
{
	CURLcode res;			//定义CURLcode类型的变量，保存返回状态码
	FILE *file = NULL;
	long retcode = 0;
	int err = 0;
    struct progressinfo progressstruct;

	curl_off_t local_file_len = -1 ;
	int use_resume = 0;
	char *local_url = NULL;
	long filesize;

	if(!curlhandle || !url || !data)
	{
		_LOG("Bad argument");
		return -1;
	}


	//err = im_curl_init(&curlhandle);

	local_url = (char *)data;
	if((local_file_len =  my_get_localsize(local_url))>0)
    {
        use_resume	= 1;
    }

	file = fopen(local_url, "ab+");
	if (file == NULL)
	{
		_LOG("open file %s fail", local_url);
		err = -1;
		goto openfile_err;
	}

	curl_easy_setopt(curlhandle, CURLOPT_URL, url);
	curl_easy_setopt(curlhandle, CURLOPT_CONNECTTIMEOUT, TIMEOUT);  // 设置连接超时，单位秒

	// 设置文件续传的位置给libcurl
	curl_easy_setopt(curlhandle, CURLOPT_RESUME_FROM_LARGE, use_resume?local_file_len:0);
	curl_easy_setopt(curlhandle, CURLOPT_NOPROGRESS, 1L);

	curl_easy_setopt(curlhandle, CURLOPT_WRITEDATA, file);
	curl_easy_setopt(curlhandle, CURLOPT_WRITEFUNCTION, my_download_data);

	curl_easy_setopt(curlhandle, CURLOPT_HEADERFUNCTION, im_get_contentlength);
	curl_easy_setopt(curlhandle, CURLOPT_HEADERDATA, &filesize);

	//跟踪下载进度
	curl_easy_setopt(curlhandle, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curlhandle, CURLOPT_XFERINFOFUNCTION, my_progress_func);
    progressstruct.curl = curlhandle;
    progressstruct.lasttime = 0;
	curl_easy_setopt(curlhandle, CURLOPT_XFERINFODATA, &progressstruct);

	//curl_easy_setopt(curlhandle, CURLOPT_VERBOSE, 1L);

	err = im_curl_perform(curlhandle);

	if(err == CURLM_OK)
    {
        fprintf(stderr, "\n总用时: %f, \n", progressstruct.lasttime);

        fprintf(stderr,"已下载 %d, 总大小 %d , 平均速度 %.2fBytes/s, 进度 100%%\n",\
                filesize, filesize, filesize/progressstruct.lasttime);
    }

out:
		fclose(file);
openfile_err:
		//im_curl_clean(curlhandle);

	return err;
}


size_t im_update_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
	int n = 0;

	n = fread((char *)ptr, size, nmemb, (FILE *)stream);
	//printf("%s ptr=%s\n",__func__, (char *)ptr);
	return n;
}

int im_curl_upload_back(CURL *curlhandle, char *url, void *data)
{
	int ret = 0;
	char *local_url = NULL;
	FILE *file = NULL;

	if (!curlhandle || !url || !data)
	{
		_LOG("Bad param!!!");
		return -1;
	}

	local_url = (char *)data;
	file = fopen(local_url, "r");
	if (file == NULL)
	{
		_LOG("fopen error:%s", local_url);
		return -1;
	}

	curl_easy_setopt(curlhandle, CURLOPT_URL, url);

	curl_easy_setopt(curlhandle, CURLOPT_READFUNCTION, im_update_data);
	curl_easy_setopt(curlhandle, CURLOPT_READDATA, file);

	curl_easy_setopt(curlhandle, CURLOPT_UPLOAD, 1L);

	curl_easy_setopt(curlhandle, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curlhandle, CURLOPT_VERBOSE, 1L);

	ret = im_curl_perform(curlhandle);
	fclose(file);

	return ret;

}

int test(char *purl)
{
	CURL *curlhandle = NULL;			 //定义CURL类型的指针
	int res = -1;
	char url[100] = {0};

	strcpy(url, purl);

	res = im_curl_init(&curlhandle);
	if (res < 0)
	{
		_LOG("im_curl_init fail\n");
		return -1;
	}

	res = im_curl_get_back(curlhandle, url, NULL);
	if (res < 0)
	{
		_LOG("im_curl_get fail\n");
		return -1;
	}

	res = im_curl_clean(curlhandle);
	if (res < 0)
	{
		_LOG("im_curl_clean fail\n");
		return -1;
	}

	return 0;
}

int test_get_content_length(char* purl)
{
    size_t filesize = 0;

    im_get_remote_file_len_back(&filesize,purl);
   // my_get_contentLength(&myfilesize,purl);
}

int test_download_file(const char* purl, const char* localpath)
{
    CURL *curlhandle = NULL;			 //
	int res = -1;
	char url[100] = {0};

	strcpy(url, purl);

	res = im_curl_init(&curlhandle);
	if (res < 0)
	{
		_LOG("im_curl_init fail\n");
		return -1;
	}
    size_t filesize = 0;
    size_t localsize = 0;
    localsize = my_get_localsize(localpath);
    im_get_remote_file_len_back(&filesize,purl);

    if (filesize==0)
    {
        _LOG("remote file size zero");
        return -1;
    }

    if (filesize == localsize)
    {
        _LOG("file already exist %ld %ld");
        return -2;
    }

	res = my_curl_download_back(curlhandle, url, localpath);
	if (res < 0)
	{
		_LOG("my_curl_download_back fail\n");
		return -1;
	}

	res = im_curl_clean(curlhandle);
	if (res < 0)
	{
		_LOG("im_curl_clean fail\n");
		return -1;
	}

	return 0;
}
