dtrace -n fbt:pxfs::entry'{@a[probefunc] = count()}'
