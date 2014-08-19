#!/usr/bin/env python
# Filename bootimg.py

__author__ = 'duanqizhi01@baidu.com (duanqz)'


import commands
from internal.toolkit import *

class Bootimg:
    """
    Model of boot image. With the property of type.
    """

    TOOLKIT = Toolkit()

    def __init__(self, bootfile):
        self.bootfile = bootfile

    def setType(self, type):
        """
        Set the type for boot image
        """

        self.type = type

    def unpack(self, output):
        """
        Unpack the boot image into the output directory.
        """

        # Try unpack tool set to find the suitable one
        self.type = self.TOOLKIT.getType(self.bootfile)

        # Check whether the tools exists
        if self.type == None: raise ValueError("Unknown boot image type.")

        # Execute the unpack command
        unpackTool = self.TOOLKIT.getTools(self.type, "UNPACK")
        cmd = unpackTool + " " + self.bootfile + " " + output
        commands.getstatusoutput(cmd)
        print " >>> Unpack " + self.type + " " + self.bootfile + " --> " + output

        # Store the used tools to output
        Toolkit.storeType(self.type, output)

    def pack(self, output):
        """
        Pack the BOOT directory into the output image
        """

        # Retrieve the last used tools from boot directory
        self.type = Toolkit.retrieveType(self.bootfile)

        # Check whether the tools exists
        if self.type == None: raise ValueError("Unknown boot image type.")

        # Execute the pack command
        packTool = self.TOOLKIT.getTools(self.type, "PACK")
        cmd = packTool + " " + self.bootfile + " " + output
        commands.getstatusoutput(cmd)
        print " >>> Pack " + self.type + " " + self.bootfile + " --> " + output

### End of class Bootimg


