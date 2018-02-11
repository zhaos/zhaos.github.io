#include <stdio.h>
#include <stdlib.h>
#include "_LOG.h"
#include "sqm_curl.h"
#include <pthread.h>

#define NUMTHREAD 4

struct downloadargs{
	char* requestURL;
	char saveto[256];
	char* user;
	char* password;
	pfunc_xferinfo xferinfo_callback;
	void* xferinfo_data;
	pthread_mutex_t* plock;
};

void* pull_one_url(void* data);

pthread_mutex_t lock;


const char* const urls[ NUMTHREAD ]={
	"http://10.165.92.189:6060/HTTPDIR/codeblocks.rar",
	"http://10.165.92.189:6060/HTTPDIR/FileZilla-setup.rar",
	"http://10.165.92.189:6060/HTTPDIR/data.html",
	"http://10.165.92.189:6060/HTTPDIR/KB.html"
};

struct downloadargs args [NUMTHREAD];

static int filename_of_url(char const* url, char *oname)
{
    int         ret = 0;
    char const  *u  = url;

    /* Remove "http(s)://" */
    u = strstr(u, "://");
    if (u) {
        u += strlen("://");
    }

    u = strrchr(u, '/');

    /* Remove last '/' */
    u++;

    /* Copy value as oname */
    while (*u != '\0') {
        //printf (".... %c\n", *u);
        *oname++ = *u++;
    }
    *oname = '\0';

    return ret;
}

int main(int argc, char* argv[])
{
    //if(argc <2)
    //{
     //   printf("usage: cmd url [path]\n");
     //  return -1;
    //}
	pthread_mutex_init(&lock,NULL);


	pthread_t tid[NUMTHREAD];
	int i;
	int error;

	for(i = 0; i<NUMTHREAD; ++i)
	{
		args[i].requestURL = urls[i];
		filename_of_url(urls[i], args[i].saveto);
		args[i].user = "zry";
		args[i].password = "123456";
		args[i].xferinfo_callback = NULL;
		args[i].xferinfo_data = NULL;
		args[i].plock = &lock;
	}

	sqm_curl_init();//global init;

		for(i = 0; i< NUMTHREAD; i++)
		{
			error = pthread_create(&tid[i],
							    NULL, /* default attributes please */
								pull_one_url,
								(void *)&args[i]);
			if(0 != error)
				fprintf(stderr, "Couldn't run thread number %d, errno %d\n", i, error);
			else
				fprintf(stderr, "Thread %d, gets %s\n", i, urls[i]);
		}
	for(i = 0; i< NUMTHREAD; i++)
	{
    	error = pthread_join(tid[i], NULL);
    	fprintf(stderr, "Thread %d terminated\n", i);
	}

    sqm_curl_clean();//global cleanup
		pthread_mutex_destroy(&lock);

    return 0;
}


void* pull_one_url(void* data)
{
	struct downloadargs* args=(struct downloadargs*) data;

	//_LOG("%S",args->requestURL);


	//_LOG("%S",args->saveto);
	sqm_filedownload(args->requestURL,args->saveto,args->user,\
		args->password,args->xferinfo_callback,args->xferinfo_data,args->plock);

}
