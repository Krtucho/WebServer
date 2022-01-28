#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>

// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}

// void replace_str(char * str, char * target, char to_set){
//     char * output = strdup("");
//     replace()
//     // bool make_change = false;

//     // for(int i = 0; i < strlen(str); i++){
//     //     make_change = false;
//     //     if(str[i] == target[0]){
//     //         change = true;
//     //         for(int j = 1; j < strlen(target) && i+ j < strlen(str); j++){
//     //             if(str[i+j] == target[j])
//     //                 make_change = false;
//     //                 break;
//     //         }
//     //     }
//     //     if(make_change){
//     //         strcat(output, (char)to_set);
//     //     }
//     //     else{
//     //         strcat(output, (char)str[i]);
//     //     }
//     // }
// }

int main(int argc, char const *argv[])
{
    struct stat buf;
    char mtime[100];

    char * from = strdup("/home//krtucho/Series//Mouse  ok/Mouse 7.mp4");
    char * rep = strdup("%20");
    char * with = strdup(" ");
    char * output = strdup("");

    output = str_replace(from, rep, with);
    printf("Output: %s\n", output);
    if(output != NULL)
        free(output);

    int a = stat("/home//krtucho/Series//Mouse  ok/Mouse 7.mp4", &buf);
    printf("st_mode = %o\n", buf.st_mode);

    strcpy(mtime, ctime(&buf.st_mtime));

    printf("st_mtime = %s\n", mtime);

    printf("%d\n", a);

    printf("%d\n", buf.st_mode);
    printf( (S_ISDIR(buf.st_mode)) ? "d\n" : "-\n");

    return 0;
}
