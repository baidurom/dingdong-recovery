#!/usr/bin/env python
#
# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
Given a target-files zipfile, remove misc_info.txt in zipfile

Usage:  remove_fs input_target_files output_image_zip

"""

import sys

if sys.hexversion < 0x02040000:
  print >> sys.stderr, "Python 2.4 or newer is required."
  sys.exit(1)

import errno
import os
import re
import tempfile
import zipfile

# missing in Python 2.4 and before
if not hasattr(os, "SEEK_SET"):
  os.SEEK_SET = 0

import common

OPTIONS = common.OPTIONS


def main(argv):
  bootable_only = [False]

  def option_handler(o, a):
    return True

  args = common.ParseOptions(argv, __doc__,
                             extra_opts="b:z",
                             extra_long_opts=["board_config=",
                                              "bootable_zip"],
                             extra_option_handler=option_handler)

  if len(args) != 2:
    common.Usage(__doc__)
    sys.exit(1)

  print "start up..."

  input_zip = zipfile.ZipFile(args[0], "r")
  output_zip = zipfile.ZipFile(args[1], "w")

  for i in input_zip.infolist():
    fdata = input_zip.read(i.filename)
    if i.filename == "META/misc_info.txt":
      remdata = ""
      for line in fdata.split("\n"):
        if line.find("extfs_sparse_flag") < 0:
          remdata += line
          remdata += "\n"
      output_zip.writestr(i, remdata)
    else:
      output_zip.writestr(i, fdata)

  input_zip.close()
  output_zip.close()

  print "done."


if __name__ == '__main__':
  try:
    common.CloseInheritedPipes()
    main(sys.argv[1:])
  except common.ExternalError, e:
    print
    print "   ERROR: %s" % (e,)
    print
    sys.exit(1)
