#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    if(argc <2)
    {
        printf("usage: cmd url [path]\n");
        return -1;
    }
    if(argc>2)
    test_download_file(argv[1],argv[2]);

    return 0;
}
