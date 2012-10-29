#!/usr/sbin/dtrace -qFs

BEGIN {
    self->trace = 0;
}

pid$target:libclcomm:*remote*invoke*:entry
{
    self->trace = 1;
    ustack();
}

pid$target:libclcomm:*remote*invoke*:return
/self->trace == 1/
{
    printf("%s\n", probefunc);
    printf("------------------------------------------------------\n");
    ustack();
    self->trace = 0;
}

