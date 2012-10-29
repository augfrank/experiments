#!/usr/sbin/dtrace -s
pid$1:default_plugin::entry
{
   self->ts[probefunc] = timestamp;
}
pid$1:default_plugin::return
/self->ts[probefunc]/
{
   @func_time[probefunc] = sum(timestamp - self->ts[probefunc]);
}
pid$1:commandlog_plugin::entry
{
   self->ts[probefunc] = timestamp;
}
pid$1:commandlog_plugin::return
/self->ts[probefunc]/
{
   @func_time[probefunc] = sum(timestamp - self->ts[probefunc]);
}

