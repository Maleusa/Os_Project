#include "syscall.h"
#define WORD1 "Morty\n"
#define WORD2 "Rick\n"

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
    f((void*) WORD2);
}