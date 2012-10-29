#!/usr/sbin/dtrace -s

pid$target:::entry
{
    self->start = 1;
}

proc:::exec-success
/self->start == 1/
{
    x = pid;
    printf("%s(pid=%d) started by uid - %d\n",execname, pid, uid);
}

