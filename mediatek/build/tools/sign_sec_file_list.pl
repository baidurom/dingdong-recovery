#!/usr/bin/perl

##########################################################
# Initialize Variables
##########################################################
my $prj = $ARGV[0];
my $key_dir = "mediatek/custom/$prj/security/image_auth";
my $cfg_dir = "mediatek/custom/$prj/security/sec_file_list";
my $cipher_tool = "mediatek/build/tools/CipherTool/CipherTool";
my $sign_tool = "mediatek/build/tools/SignTool/SignTool.sh";


##########################################################
# Sign ANDROID Secure File List
##########################################################
print "\n\n*** Sign ANDROID Secure File List ***\n\n";

my $and_secfl = "mediatek/custom/$prj/security/sec_file_list/ANDRO_SFL.ini";
my $s_andro_fl = "mediatek/external/seclib/S_ANDRO_SFL.ini";

if (-e "$and_secfl")
{
	if (-e "$s_andro_fl")
	{
		print "remove old file list (1) ... \n";
		system("rm -f $s_andro_fl");
	}
					
	system("./$sign_tool $key_dir/IMG_AUTH_KEY.ini $cfg_dir/SFL_CFG.ini $and_secfl $s_andro_fl");
	
	if (! -e "$s_andro_fl")
	{
		die "sign failed. please check";
	}
}
else
{
	print "file doesn't exist\n";	
}


##########################################################
# Sign SECRO Secure File List
##########################################################
print "\n\n*** Sign SECRO Secure File List ***\n\n";

my $secro_secfl = "mediatek/custom/$prj/security/sec_file_list/SECRO_SFL.ini";
my $s_secro_fl_o1 = "mediatek/custom/$prj/secro/S_SECRO_SFL.ini";
my $s_secro_fl_o2 = "mediatek/external/seclib/S_SECRO_SFL.ini";

if (-e "$secro_secfl")
{				
	if (-e "$s_secro_fl_o1")
	{
		print "remove old file list (1) ... \n";
		system("rm -f $s_secro_fl_o1");
	}

	if (-e "$s_secro_fl_o2")
	{
		print "remove old file list (2) ... \n";
		system("rm -f $s_secro_fl_o2");
	}

	system("./$sign_tool $key_dir/IMG_AUTH_KEY.ini $cfg_dir/SFL_CFG.ini $secro_secfl $s_secro_fl_o1");

	if (! -e "$s_secro_fl_o1")
	{
		die "sign failed. please check";
	}
	
	system("cp -f $s_secro_fl_o1 $s_secro_fl_o2");	
}
else
{
	print "file doesn't exist\n";
}

