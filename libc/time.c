/*
FUNCTION
<<time>>---get current calendar time (as single number)

INDEX
        time

ANSI_SYNOPSIS
        #include <time.h>
        time_t time(time_t *<[t]>);

TRAD_SYNOPSIS
        #include <time.h>
        time_t time(<[t]>)
        time_t *<[t]>;

DESCRIPTION
<<time>> looks up the best available representation of the current
time and returns it, encoded as a <<time_t>>.  It stores the same
value at <[t]> unless the argument is <<NULL>>.

RETURNS
A <<-1>> result means the current time is not available; otherwise the
result represents the current time.

PORTABILITY
ANSI C requires <<time>>.

Supporting OS subroutine required: Some implementations require
<<gettimeofday>>.
*/

/* Most times we have a system call in newlib/libc/sys/.. to do this job */

#include <time.h>

#include <stdint.h>

time_t
_DEFUN (time, (t),
        time_t * t)
{
    time_t tmp;

    uint64_t val;

    // TODO : implement
    __asm__ __volatile__ ( "rdtsc" : "=A"(val) );

    tmp = val;

    if (t) *t = tmp;

    return tmp;
}
