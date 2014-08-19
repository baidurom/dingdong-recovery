#!/usr/bin/python
# Filename unpack_bootimg.py

__author__ = 'duanqizhi01@baidu.com (duanqz)'


from internal.bootimg import Bootimg
import sys

### Start
if __name__ == '__main__':
    argLen = len(sys.argv)
    if argLen == 1:
        print "Usage: pack_bootimg.py BOOT_IMG [OUTPUT]"
        print "       - BOOT_IMG : the boot image to be unpack"
        print "       - OUTPUT   : the output directory after unpack. if not present OUT/ directory will be used."
        exit(1);
    elif argLen == 2:
        bootfile = sys.argv[1]
        output = "OUT/"
    elif argLen >= 3:
        bootfile = sys.argv[1]
        output = sys.argv[2]

    Bootimg(bootfile).unpack(output)
