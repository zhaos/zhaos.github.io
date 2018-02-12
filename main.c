#include <stdio.h>
#include <stdlib.h>
#include "_LOG.h"
#include "mylibcurl.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

#define NUMTHREAD 5

struct downloadargs{
	char* requestURL;
	char saveto[256];
	char* user;
	char* password;
	pfunc_xferinfo xferinfo_callback;
	void* xferinfo_data;
	pthread_mutex_t* plock;
	int pid;
};

void* pull_one_url(void* data);

pthread_mutex_t lock;


const char* const urls[ NUMTHREAD]={
	"http://10.165.92.189:6060/HTTPDIR/codeblocks.rar",
	"http://10.165.92.189:6060/HTTPDIR/FileZilla-setup.rar",
	"http://10.165.92.189:6060/HTTPDIR/data.html",
	"http://10.165.92.189:6060/HTTPDIR/KB.html",
	"sftp://100.100.187.246/home/zry/TortoiseSVN_V1.6.16(windows_client)[1].rar"

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

	for(i = 0; i<NUMTHREAD-1; ++i)
	{
		args[i].requestURL = urls[i];
		filename_of_url(urls[i], args[i].saveto);
		args[i].user = "zry";
		args[i].password = "123456";
		args[i].xferinfo_callback = NULL;
		args[i].xferinfo_data = NULL;
		args[i].plock = &lock;
		args[i].pid = 0;
	}
		args[i].requestURL = urls[i];
		filename_of_url(urls[i], args[i].saveto);
		args[i].user = "root";
		args[i].password = "Huawei789";
		args[i].xferinfo_callback = NULL;
		args[i].xferinfo_data = NULL;
		args[i].plock = &lock;
		args[i].pid = 0;

	sqm_curl_init();//global init;

		for(i = 0; i< NUMTHREAD; i++)
		{
			error = pthread_create(&tid[i],
							    NULL, /* default attributes please */
								pull_one_url,
								(void *)&args[i]);
			if(0 != error)
				fprintf(stderr, "Couldn't create No%d thread, errno %d\n", i, error);
			else
			;
				//fprintf(stderr, "Thread %d, gets %s\n", args[i].pid, urls[i]);
				//fprintf(stderr, "Task %d, gets %s\n", i, urls[i]);
		}
	for(i = 0; i< NUMTHREAD; i++)
	{
    	error = pthread_join(tid[i], NULL);
    	fprintf(stderr, "Thread %d terminated\n", args[i].pid);
	}

    sqm_curl_clean();//global cleanup
	pthread_mutex_destroy(&lock);

    return 0;
}


void* pull_one_url(void* data)
{
	struct downloadargs* args=(struct downloadargs*) data;
	//the pid of current thread
	args->pid=syscall(SYS_gettid);
	fprintf(stderr, "Thread %d, gets %s\n", args->pid, args->requestURL);
	sqm_filedownload(args->requestURL,args->saveto,args->user,\
		args->password,args->xferinfo_callback,args->xferinfo_data,\
		args->plock, args->pid);
}
