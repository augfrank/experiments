#!/usr/sbin/dtrace -s

BEGIN {
    self->depth = 0;
}

syscall:::entry
/execname=="ms.sapQE1_ASCS00"/
{
	self->start = 1;
}

syscall:::return
/self->start/
{
	@badcall[ustack(), execname, timestamp - self->start] = count();
}

syscall:::return
{
	self->start = 0;
}

fbt:::entry
/self->start/
{
	self->ts[self->depth++] = timestamp;
}

fbt:::return
/self->start && self->depth && self->ts[--self->depth]/
{
	@[probefunc] = avg(timestamp - self->ts[self->depth]);
	self->ts[self->depth] = 0;
}

