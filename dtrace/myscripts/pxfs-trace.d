#!/usr/sbin/dtrace -qs
#pragma D option flowindent

BEGIN {
    self->update = 0;
}


/*
fbt:ufs:ufs_alloc_data:entry
{
    self->nfs = (curthread->t_flag & 0x0080) ? "TRUE" : "FALSE";
    printf("NFS Thread flag:%s\n", self->nfs);
}

fbt:ufs:ufs_alloc_data:return
{
    self->nfs = (curthread->t_flag & 0x0080) ? "TRUE" : "FALSE";
    printf("At Return NFS Thread flag:%s\n", self->nfs);
    printf("%s returns :%d\n", probefunc, arg1);
}

fbt:ufs:ufs_rdwr_data:entry
{
    self->nfs = (curthread->t_flag & 0x0080) ? "TRUE" : "FALSE";
    printf("NFS Thread flag:%s\n", self->nfs);
}

fbt:ufs:ufs_rdwr_data:return
{
    self->nfs = (curthread->t_flag & 0x0080) ? "TRUE" : "FALSE";
    printf("At Return NFS Thread flag:%s\n", self->nfs);
    printf("%s returns :%d\n", probefunc, arg1);
}
*/

fbt:ufs:ufs_remove:entry
{
    printf("%s called on %s\n", probefunc, (string)args[1]);
    stack();
}

fbt:ufs:ufs_rename:entry
{
    printf("%s called Old:%s, New:%s\n", probefunc, (string)args[1],
	    (string)args[3]);
    stack();
}

fbt:pxfs:*remove_fobj*:entry
{
    printf("%s enter :%s\n", probefunc, stringof(arg0));
    self->trace = 1;
}

fbt:pxfs:*remove_fobj*:return
{
    printf("%s enter :%d\n", probefunc, arg1);
    self->trace = 0;
}

fbt:pxfs:*empty_inactive_list*:
{
    printf("%s\n", probefunc);
    stack();
}

fbt:ufs:ufs_statvfs:entry
/self->update == 1/
{
    self->vfsp = args[0];
    ufsvfsp = (struct ufsvfs *)self->vfsp->vfs_data;
    fsp = ufsvfsp->vfs_bufp->b_un.b_fs;
    bfree = fsp->fs_cstotal.cs_nbfree * fsp->fs_frag +
	fsp->fs_cstotal.cs_nffree;
    printf("Entry: Max Avail:%d, Used:%d\n", (fsp->fs_dsize -
	    ufsvfsp->vfs_minfrags), (fsp->fs_dsize - bfree));

}

fbt:ufs:ufs_statvfs:return
/self->update == 1 && self->vfsp/
{
    ufsvfsp = (struct ufsvfs *)self->vfsp->vfs_data;
    fsp = ufsvfsp->vfs_bufp->b_un.b_fs;
    bfree = fsp->fs_cstotal.cs_nbfree * fsp->fs_frag +
	fsp->fs_cstotal.cs_nffree;
    printf("Return: Max Avail:%d, Used:%d\n", (fsp->fs_dsize -
	    ufsvfsp->vfs_minfrags), (fsp->fs_dsize - bfree));
}

END 
{
}

