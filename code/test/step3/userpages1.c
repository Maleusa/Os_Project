#include "syscall.h"
#define WORD1 "Morty\n"
#define WORD2 "Rick\n"
#define WORD3 "Al\n"

const int N = 10; // Choose it large enough!

void puts(char *s)
{
    char *p;
    for (p = s; *p != '\0'; p++)
        PutChar(*p);
}
void f(void *s)
{
    int i;
    for (i = 0; i < N; i++)
        puts((char *)s);
}
int main()
{
    UserThreadCreate(f, (void *) WORD1);
    UserThreadCreate(f, (void *) WORD3);
    f((void*) WORD2);
}