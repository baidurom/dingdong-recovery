#!/usr/bin/perl -w
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
($#ARGV == 2 || $#ARGV == 1 || $#ARGV == 0) || &Usage;
($prj, $platform, $emmc_support) = @ARGV;

(exit 0) if ($prj eq "generic");
$logDir = "out/target/product/$prj";

if (!-d $logDir){
print "Can NOT find $logDir\n";
exit 0;
} 

my @lnmatrix = ();
if ($ENV{'KBUILD_OUTPUT_SUPPORT'} eq "yes")
{
   @lnmatrix = (
  "mediatek/preloader/preloader_${prj}.bin",
  "bootable/bootloader/lk/build-${prj}/lk.bin",
  "bootable/bootloader/lk/build-${prj}/logo.bin",
  "kernel/out/kernel_${prj}.bin",
  );
}
else
{
  @lnmatrix = (
  "mediatek/preloader/preloader_${prj}.bin",
  "bootable/bootloader/lk/build-${prj}/lk.bin",
  "bootable/bootloader/lk/build-${prj}/logo.bin",
  "kernel/kernel_${prj}.bin",
  );   
}
  
if ($emmc_support eq "yes")
{
  push(@lnmatrix,"mediatek/misc/${platform}_Android_scatter_emmc.txt");
  push(@lnmatrix,"mediatek/misc/MBR");
	opendir (DIR,"mediatek/misc");
	@dir = readdir(DIR);
	foreach $temp (@dir){
		if($temp=~/(EBR\d)/){
			#print "~~~~~~~~~copy  $temp $1\n";
			push(@lnmatrix,"mediatek/misc/$1");
		}
	}
	closedir DIR;
}
else{
  push(@lnmatrix,"mediatek/misc/${platform}_Android_scatter.txt");	
}

chdir($logDir);
$relDir = $logDir;
$relDir =~ s/[^\/]+/../g;

foreach $i (@lnmatrix) {
  $lnfile = "${relDir}/$i";
  $i =~ /([^\/]+)$/;
  $j = $1;
  if ($j =~ /kernel\.bin/) {
    $j = "kernel.bin";
  }
  system("rm $j") if (-e $j);
  if (!-e $lnfile) {
     print("$lnfile does NOT exist!\n");
     next;
  }
  if ($lnfile =~ /kernel\.bin/) {
    system("ln -s $lnfile kernel.bin");
  } else {
    system("ln -s $lnfile .");
  }
}

exit 0;

sub Usage {
  warn << "__END_OF_USAGE";
Usage: (\$prj, \$platform, \$emmc_support) = @ARGV 
__END_OF_USAGE
  exit 1;
}

