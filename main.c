#include <stdio.h>
#include <unistd.h>
int main()
{
    char* cmd[] = {"ls", NULL};
    execvp(cmd[0], cmd);
    return 1;
}