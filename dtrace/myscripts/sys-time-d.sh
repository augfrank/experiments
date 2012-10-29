#!/usr/sbin/dtrace -s
syscall:libsds_sc::entry
{
   self->ts[probefunc] = timestamp;
}
syscall:libsds_sc::return
/self->ts[probefunc]/
{
   @func_time[probefunc] = sum(timestamp - self->ts[probefunc]);
}
syscall:libsds_sc::entry
{
   self->ts[probefunc] = timestamp;
}
syscall:libsds_sc::return
/self->ts[probefunc]/
{
   @func_time[probefunc] = sum(timestamp - self->ts[probefunc]);
}

