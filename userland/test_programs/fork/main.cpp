/*
main.cpp

Copyright (c) 28 Yann BOUCHER (yann)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <stdio.h>

#include <syscalls/syscall_list.hpp>

#include <errno.h>
#include <stdio.h>
#include <string.h>

int main()
{
    printf("Before fork : \n");
    int ret = fork();
    if (ret < 0)
    {
        printf("Error : %s\n", strerror(errno));
        return 0;
    }
    else if (ret == 0)
    {
                while (true)
                {
                    printf("Child!\n");
                    sched_yield();
                    exit(0);
                }
                return 1;
//        const char* argv[] = {0};
//        const char* envp[] = {0};
//        execve("/initrd/test_programs/MoreOrLess", argv, envp);
    }
    else
    {
        while (true)
        {
            int status;
            printf("Parent with child PID %d\n", ret);
            if (waitpid(ret, &status, 0) < 0)
            {
                perror("waitpid");
            }

            printf("Back to parent\n");
            while (true)
            {
                sched_yield();
            }
        }
        return 2;
    }
}
