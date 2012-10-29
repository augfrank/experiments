#!/usr/sbin/dtrace -s
#pragma D option flowindent

BEGIN {
    self->trace = 1;
}


fbt:pxfs:*px_write*:entry
{
    self->trace = 1;
}

fbt:pxfs::entry
/self->trace == 1/
{
    self->data[probefunc] = vtimestamp;
}

fbt:pxfs::return
/self->trace == 1 && self->data[probefunc] != 0/
{
    @time[probefunc] = sum(vtimestamp - self->data[probefunc]);
    @num[probefunc] = count();
}


fbt:pxfs:*px_write*:return
/self->trace == 1/
{
    self->trace = 1;
}

lockstat:::adaptive-block,
lockstat:::rw-block
{
    @locks[probename, stack()] = count();
    @locktime[probename, stack()] = sum(arg1);
}

END 
{
    printf("\nTime:");
    printa(@time);
    printf("\nCount:");
    printa(@num);
    printf("\nLocks:");
    printa(@locks);
    printf("\nLocks Time:");
    printa(@locktime);
}

