#!/usr/bin/python
# Filename test.py

__author__ = 'duanqizhi01@baidu.com (duanqz)'


import os
import pickle
import shutil
from os import sys, path
sys.path.append(path.dirname(path.dirname(path.abspath(__file__))))
from internal.bootimg import Bootimg


def assertTypeEquals(bootfile, type):

    # Unpack
    Bootimg(bootfile).unpack("OUT/")

    # Load type
    fileHandle = open("OUT/.bootimg_type_file", "r")

    # Assert
    assert pickle.load(fileHandle) == type

    # Clear
    fileHandle.close()
    shutil.rmtree("OUT/")

### Start
if __name__ == '__main__':

    testDir = path.dirname(path.abspath(__file__)) + "/"

    print " >>> Test unpack common boot.img"
    bootfile = testDir + "common-boot.img"
    assertTypeEquals(bootfile, "COMMON")
    print " <<< Pass\n"

    print " >>> Test unpack mtk boot.img"
    bootfile = testDir + "mtk-boot.img"
    assertTypeEquals(bootfile, "MTK")
    print " <<< Pass\n"

    print " >>> Test unpack sony boot.img"
    bootfile = testDir + "sony-boot.img"
    assertTypeEquals(bootfile, "SONY")
    print " <<< Pass\n"
