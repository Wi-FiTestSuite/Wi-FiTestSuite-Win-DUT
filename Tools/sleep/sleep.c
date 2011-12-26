#include <stdio.h>
#include <stdlib.h>
#ifdef DOS
int Sleep(long);
#else
#include <windows.h>
#endif

int main(int argc,char **argv)
{
    long s;
    char *p;
    char *msg = " time-to-sleep-in-seconds";

    if ( argc >= 2 && !(*argv[1] == '/' && *(argv[1] + 1) == '?') ) {
        s = strtol(argv[1],&p,10);
        if ( argv[1] != p && s > 0 && s <= 2147483 ) { /* 2147483647(2^31) / 1000 */
            Sleep(s * 1000);
        }
    } else {
        fputs("Usage:  ",stdout);
        fputs(argv[0],stdout);
        puts(msg);
        return 1;
    }
    return 0;
}
