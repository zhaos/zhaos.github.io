#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>

#define NUMT 4
long int count = 20000;
FILE *fp = NULL;
pthread_mutex_t lock; 

const char * const name[NUMT]= {"file_001",  
	"file_002",  
	"file_003",  
	"file_004"};
static void *fpopen_one(void *ptr)
{  	
	char *name = (char*) ptr;
	pthread_mutex_lock(&lock); 
	FILE* fp = fopen(name,"wb");
	
	if(fp!=NULL)
		fclose(fp);
	else
		printf("fp=NULL, fopen error!\n");
	pthread_mutex_unlock(&lock); 
	pthread_exit(NULL); 
	return 0;
}

int main(int argc, char* argv[])
{
 while(count-->0)
 	{
	
	pthread_t tid[NUMT]; 
	int i;  
	int error;  
	
	pthread_mutex_init(&lock,NULL);  

	for(i = 0; i< NUMT; i++) 
	{    
		error = pthread_create(&tid[i],NULL,fpopen_one,(void *)name[i]);    
		if(0 != error)      
			fprintf(stderr, "Couldn't run thread number %d, errno %d\n", i, error);   
		else      
			fprintf(stderr, "Thread %d, gets %s\n", i, name[i]);  
	} 

		/* now wait for all threads to terminate */  
	for(i = 0; i< NUMT; i++)
	{    
		error = pthread_join(tid[i], NULL);    
		fprintf(stderr, "Thread %d terminated\n", i); 
	}
	
	pthread_mutex_destroy(&lock);  
 	}
	return 0;
}
