#pragma -D option flowindent

fbt:pxfs::entry
/execname == "pxfs-test"/
{
    printf("%d", arg0);
}

fbt:pxfs::return
/execname == "pxfs-test"/
{
    printf("%s+%x returned %x", probefunc, arg0, arg1);
}

fbt:pxfs::return
/execname == "pxfs-test" && arg1 == 14/
{
    printf("XXXXXXXXXXXXXXXXXXXXXXX %s+%x returned %x", probefunc, arg0, arg1);
    pxfsEntry = 0;
}

fbt:ufs::entry
/execname == "pxfs-test"/
{
    printf("%d", arg0);
}

fbt:ufs::return
/execname == "pxfs-test"/
{
    printf("%s+%x returned %x", probefunc, arg0, arg1);
}
