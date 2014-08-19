#!/usr/bin/env python
# Filename tools.py

__author__ = 'duanqizhi01@baidu.com (duanqz)'


### Import blocks

import os
import shutil
import commands
import tempfile

try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET



### Class blocks

class Toolkit:
    """
    Toolkit including all tools
    """

    __DIR = os.path.dirname(os.path.abspath(__file__)) + "/"
    __TOOLKIT_XML = __DIR + "toolkit.xml"
    __BOOTIMG_TYPE_FILE = "type.config"

    allTools = {}

    def __init__(self):
        """
        Initialize tools factory from config.xml
        """

        tree = ET.parse(Toolkit.__TOOLKIT_XML)
        for tool in tree.findall("tool"):
            type = tool.attrib["type"]
            unpackTool = tool.find("unpack").text
            packTool = tool.find("pack").text
            self.allTools[type] =  { "UNPACK" : Toolkit.__DIR + unpackTool,
                                     "PACK"   : Toolkit.__DIR + packTool }

    def getType(self, bootfile):
        """
        Match appropriate tools for the boot image file.
        """

        toolsType = None

        # Try to unpack boot image for each type,
        # choose the appropriate one.
        for type in self.allTools.keys():
            # Try to unpack the boot image by unpack tool
            unpackTool = self.getTools(type, "UNPACK")
            if ToolsMatcher.tryUnpack(unpackTool, bootfile) == True:
                toolsType = type
                break

        ToolsMatcher.clearTempDir()
        return toolsType

    def getTools(self, type, attrib=None):
        """
        Get tools by type.
        """

        tools = self.allTools.get(type)
        if attrib == None :
            return tools
        else:
            return tools[attrib]

    @staticmethod
    def storeType(type, dir):
        # Serialize
        fileHandle = open(os.path.join(dir, Toolkit.__BOOTIMG_TYPE_FILE), "w")
        fileHandle.write(type)
        fileHandle.close()

    @staticmethod
    def retrieveType(dir):
        # De-serialize
        try:
            fileHandle = open(os.path.join(dir, Toolkit.__BOOTIMG_TYPE_FILE), "r")
            type = fileHandle.read().rstrip()
            fileHandle.close()
        except:
            print " >>> Can not find type.config, use COMMON as image type by default"
            type = "COMMON"
        return type

### End of class Toolkit


class ToolsMatcher:
    """
    Match out appropriate tools
    """

    # Directory for temporary data storage.
    TEMP_DIR=tempfile.mkdtemp()

    @staticmethod
    def tryUnpack(unpackTool, bootimg):
        """
        Try to unpack the boot image into TEMP_DIR.
        Return whether unpack successfully or not.
        """

        ToolsMatcher.clearTempDir()

        cmd = unpackTool + " " + bootimg + " " + ToolsMatcher.TEMP_DIR
        result = commands.getstatusoutput(cmd)

        # Debug code. Useless for release version
        ToolsMatcher.__debug("Try " + cmd)
        ToolsMatcher.__debug(result)

        return ToolsMatcher.isUnpackSuccess(result)

    @staticmethod
    def isUnpackSuccess(result):
        """
        Check whether unpack the boot image successfully or not.
        """

        kernel = ToolsMatcher.TEMP_DIR + "/kernel"
        initrc = ToolsMatcher.TEMP_DIR + "/RAMDISK/init.rc"

        # True : Result is correct and one the file exists
        return ToolsMatcher.isCorretResult(result) and \
               (os.path.exists(kernel) or os.path.exists(initrc))

    @staticmethod
    def isCorretResult(result):
        """
        Check whether the result contains error or not
        """

        errKey1 = "Could not find any embedded ramdisk images"
        errKey2 = "Aborted"
        strResult = str(result)

        # True : all the error keys are not found in result
        return strResult.find(errKey1) < 0 and \
               strResult.find(errKey2) < 0

    @staticmethod
    def clearTempDir():
        """
        Clear the temporary directory
        """

        if os.path.exists(ToolsMatcher.TEMP_DIR) == True:
            shutil.rmtree(ToolsMatcher.TEMP_DIR)

    @staticmethod
    def __debug(msg):
        if False: print msg
### End of class ToolsMatcher

