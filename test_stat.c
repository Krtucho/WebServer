#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    struct stat buf;
    char mtime[100];

    int a = stat("test.txt", &buf);
    printf("st_mode = %o\n", buf.st_mode);

    strcpy(mtime, ctime(&buf.st_mtime));

    printf("st_mtime = %s\n", mtime);

    printf("%d\n", a);

    return 0;
}
