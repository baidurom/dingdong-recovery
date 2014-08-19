#!/usr/bin/perl
# 
# Copyright Statement:
# --------------------
# This software is protected by Copyright and the information contained
# herein is confidential. The software may not be copied and the information
# contained herein may not be used or disclosed except with the written
# permission of MediaTek Inc. (C) 2010
# 
# BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
# NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
# SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
# 
# BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
# LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
# 
# THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
# WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
# LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
# RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
# THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
# 
($rslt, $logFile, $act) = @ARGV;
if ($logFile eq "/dev/null") {
  if ($rslt == 0) {
    print "                    ==> [OK]    " . &CurrTimeStr . "\n";
  } else {
    print "                    ==> [FAIL]  " . &CurrTimeStr . "\n";
  }
  exit $rslt;
}

(($#ARGV < 1) || ($#ARGV > 2)) && &Usage;

die "Can NOT find $logFile" if (!-e $logFile);
$parseScrpt = "mediatek/build/tools/parseErr.pl";
$i = 0;
while (!-e $parseScrpt) {
  $i++;
  $parseScrpt = "../$parseScrpt";
  last if ($i > 8);
}

if ($logFile =~ /\/([^\/]+)_([^_]+)\.log$/) {
  $prj = $1;
  $curMod = $2;
} else {
  warn "Can NOT match project and module\n";
}

$chkBin = 1;
($prj = "generic") if ($prj eq "emulator");
if ($curMod eq "uboot") {
  $chkFile = "bootable/bootloader/uboot/u-boot-mt6516.bin";
  $chkFile = "bootable/bootloader/uboot/uboot_${prj}.bin";
} elsif ($curMod eq "kernel") {
  $chkFile = "kernel/Download/flash/${prj}_kernel.bin";
  if ($ENV{'KBUILD_OUTPUT_SUPPORT'} eq "yes")
  {  
    $chkFile = "kernel/out/kernel_${prj}.bin";
  }
  else 
  { 
    $chkFile = "kernel/kernel_${prj}.bin"; 
  }
} elsif ($curMod eq "android") {
  $chkFile = "out/target/product/${prj}/system.img";
}

if (($chkFile ne "") && (!-e $chkFile || -z $chkFile)) {
  if (($act ne "clean") && ($act ne "update-api")) {
    $chkBin = 0;
  }
}

if ($rslt == 0 && $chkBin == 1) {
  print "                    ==> [OK]    " . &CurrTimeStr . "\n";
} else {
  die "Can NOT find $parseScrpt" if (!-e $parseScrpt);
  p_system("perl $parseScrpt $logFile > ${logFile}_err 2>&1");
  $logFileRelPath = $logFile;
  $logFileRelPath =~ s/^.*(out\/)/$1/;
  if (-z "${logFile}_err") {
#    print "                         Can NOT parse any errors. Please check log by yourslef\n                         or mail it to BM\n";
  } else {
    print "                         ${logFileRelPath}_err\n";
  }
  print "                    ==> [FAIL]  " . &CurrTimeStr . "\n";
  exit 1;
}

exit 0;

sub CurrTimeStr {
  my($sec, $min, $hour, $mday, $mon, $year) = localtime(time);
  return (sprintf "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d", $year+1900, $mon+1, $mday, $hour, $min, $sec);
}

sub Usage {
  warn << "__END_OF_USAGE";
Usage: $myCmd result logFile
__END_OF_USAGE
  exit 1;
}

sub p_system
{
  my ($cmd) = @_;
  my ($debugp) = 0;
  my $result;
  ($debugp != 0) && print("$cmd\n");
  ($performanceChk == 1) && print &CurrTimeStr . " system $cmd\n";
  $result = system("$cmd");
  ($performanceChk == 1) && print &CurrTimeStr . " exit $cmd\n";
  return $result;
}

