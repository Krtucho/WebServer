#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>

int main(int argc, char const *argv[])
{
    struct stat buf;
    char mtime[100];

    int a = stat("/home/krtucho", &buf);
    printf("st_mode = %o\n", buf.st_mode);

    strcpy(mtime, ctime(&buf.st_mtime));

    printf("st_mtime = %s\n", mtime);

    printf("%d\n", a);

    printf("%d\n", buf.st_mode);
    printf( (S_ISDIR(buf.st_mode)) ? "d" : "-\n");

    return 0;
}
