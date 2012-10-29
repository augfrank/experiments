#!/usr/sbin/dtrace -qs

BEGIN {
    self->depth = 0;
}

io:::start,io:::done, io:::wait-start, io:::wait-done
/args[2]->fi_fs == "ufs"/
{
    printf("Name:%s,Dir:%s,Path:%s,FS:%s,Mount:%s\n", 
	    args[2]->fi_name, args[2]->fi_dirname, args[2]->fi_pathname,
	    args[2]->fi_fs, args[2]->fi_mount);
    printf("Dev Name:%s, Dev StatName:%s, Dev Pathname:%s\n", 
	     args[1]->dev_name, args[1]->dev_statname, args[1]->dev_pathname);
}

END 
{
}

