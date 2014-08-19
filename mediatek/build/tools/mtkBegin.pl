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
($#ARGV != 0) && &Usage;
($prj) = @ARGV;

($prj = "generic") if ($prj eq "emulator");

$flag_subrel = "mediatek/build/android/full/config.mk";
$flag_custrel = "mediatek/build/android/full/config.mk.custrel";
$srcDir = "vendor/mediatek/$prj/artifacts/out/";
$dstDir = "out/";
$tmpDir = "vendor/mediatek/$prj/artifacts/kernel/out/";

exit 0, if (!-e $flag_subrel && !-e $flag_custrel);
exit 0, if (-e $flag_subrel && -e $flag_custrel);

if (!-e $flag_subrel)
#if (0)
{
	if (-d $srcDir)
	{
	  system("rsync -av --exclude=.svn --exclude=.git --exclude=.cvs $srcDir $dstDir > auto_sync_android.log 2>&1");
	}
	exit 0;
}

if (!-e $flag_custrel)
#if (!-e $flag_subrel)
{
	my $binaryAppPath = $srcDir . "/target/product/$prj/system/app/";
	#print "app list $binaryAppPath\n";
	my @applist = <$binaryAppPath/*.apk>;
        
	foreach my $app (@applist)
	{
	  #print "Signing using customerization signature for $app \n";
	  &signApk($app);
	}
	if (-d $srcDir)
	{
	  system("rsync -av --exclude=.svn --exclude=.git --exclude=.cvs $srcDir $dstDir > auto_sync_android.log 2>&1");
	} 
	if (-d $tmpDir)
	{
	  system("rsync -av --exclude=.svn --exclude=.git --exclude=.cvs $tmpDir kernel/out/ > auto_sync_kernel.log 2>&1\n");
	}
}

exit 0;

sub Usage {
  warn << "__END_OF_USAGE";
Usage: $myCmd project
__END_OF_USAGE
  exit 1;
}

sub signApk {
  my ($src) = @_;
  my $keypath = "";
  my $src_tmp = $src . ".bak";
  my $signTool = $srcDir . "/host/linux-x86/framework/signapk.jar";
  if ($ENV{"MTK_SIGNATURE_CUSTOMIZATION"} eq "yes")
  {
    if ($ENV{"MTK_INTERNAL"} eq "yes")
    {
      $keypath = "build/target/product/security/common";
    }
    else
    {
      $keypath = "build/target/product/security/$prj";
    }
  }
  else
  {
    $keypath = "build/target/product/security";
  }
  my $key1 = "$keypath/platform.x509.pem";
  my $key2 = "$keypath/platform.pk8";
  #print "java -jar $signTool $key1 $key2 $src $src_tmp";
  system ("java -jar $signTool $key1 $key2 $src $src_tmp");
  system ("mv $src_tmp $src");
}

