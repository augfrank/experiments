#!/usr/sbin/dtrace -s
pid$target:::entry
{
   self->ts[probefunc] = timestamp;
}
pid$target:::return
/self->ts[probefunc]/
{
   @func_time[probefunc] = sum(timestamp - self->ts[probefunc]);
}

syscall:::entry
{
       self->ts[probefunc] = timestamp;
}
syscall:::return
/self->ts[probefunc]/
{
       @func_time[probefunc] = sum(timestamp - self->ts[probefunc]);
}

