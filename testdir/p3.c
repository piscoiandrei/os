#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>

int main(int argc, char *argv[])
{
    char *filename=argv[1];
    struct stat buf;
    stat(filename, &buf);
    printf("%s\n", filename);
    printf("%ld\n", buf.st_size);
}