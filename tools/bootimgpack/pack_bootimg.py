#!/usr/bin/python
# Filename pack_bootimg.py

__author__ = 'duanqizhi01@baidu.com (duanqz)'


from internal.bootimg import Bootimg
import sys

### Start
if __name__ == '__main__':
    argLen = len(sys.argv)
    if argLen == 1:
        print "Usage: pack_bootimg.py BOOT_IMG_DIR [OUTPUT_IMG]"
        print "       - BOOT_IMG_DIR : the directory of boot image files."
        print "       - OUTPUT_IMG   : the output image after pack. If not present, out.img will be used"
        exit(1);
    elif argLen == 2:
        bootfile = sys.argv[1]
        output = "out.img"
    elif argLen >= 3:
        bootfile = sys.argv[1]
        output = sys.argv[2]

    Bootimg(bootfile).pack(output)
