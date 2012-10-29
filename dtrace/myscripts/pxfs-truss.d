#!/usr/sbin/dtrace -qs
#pragma D option flowindent

BEGIN {
    self->depth = 0;
}

fbt:pxfs::entry
/probefunc != "__1cFpxvfsRupdate_throughput6Fi_v_"/
{
	self->ts[self->depth++] = timestamp;
	printf("start:%s\n", probefunc);
	self->trace = 1
}

fbt:pxfs::return
/self->depth && self->ts[--self->depth] && self->trace/
{
	self->ts[self->depth] = 0;
	printf("Program %s Returns from:%s with:%d\n", execname,
		probefunc, arg1);
	self->trace = 0
}

fbt:ufs::entry
/self->trace == 1/
{
	self->ts[self->depth++] = timestamp;
	printf("start:%s\n", probefunc);
}

fbt:ufs::return
/self->trace == 1 && self->depth && self->ts[--self->depth]/
{
	self->ts[self->depth] = 0;
	printf("Returns from:%s with:%d\n", probefunc, arg1);
}
