#include <stdio.h>
#include <stdlib.h>
#include "_LOG.h"
//#include "sqm_curl.h"

int main(int argc, char* argv[])
{
    if(argc <2)
    {
        printf("usage: cmd url [path]\n");
        return -1;
    }
    int i =0;
    for(i=argc;i<argc+6;++i)
    {
        argv[i] = NULL;

    }
    for (i=1;i<argc;++i)
        _LOG("%d, %s....",(i),argv[i]);
   // _LOG("argv[%d], %d",argc, argv[argc]);
    //_LOG("argv[%d], %d",(argc+1), argv[argc+1]);


    //_LOG("begin curl call ...");
    //getchar();
    sqm_curl_init();//global init
    sqm_filedownload(argv[1],argv[2],NULL,NULL,NULL,NULL);
    sqm_curl_clean();//global cleanup
    return 0;
}
