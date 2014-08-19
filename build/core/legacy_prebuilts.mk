#
# Copyright (C) 2010 The Android Open Source Project
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
#

# This is the list of modules grandfathered to use ALL_PREBUILT

# DO NOT ADD ANY NEW MODULE TO THIS FILE
#
# ALL_PREBUILT modules are hard to control and audit and we don't want
# to add any new such module in the system

GRANDFATHERED_ALL_PREBUILT := \
	akmd2 \
	am \
	ap_gain.bin \
	AVRCP.kl \
	batch \
	bitmap_size.txt \
	bmgr \
	bp.img \
	brcm_guci_drv \
	bypassfactory \
	cdt.bin \
	chat-ril \
	content \
	cpcap-key.kl \
	data \
	dbus.conf \
	dev \
	egl.cfg \
	firmware_error.565 \
	firmware_install.565 \
	ftmipcd \
	gps.conf \
	gpsconfig.xml \
	gps.stingray.so \
	gralloc.omap3.so \
	gralloc.tegra.so \
	hosts \
	hwcomposer.tegra.so \
	ime \
	init.goldfish.rc \
	init.goldfish.sh \
	init.olympus.rc \
	init.rc \
	init.sholes.rc \
	init.stingray.rc \
	input \
	kernel \
	lbl \
	libEGL_POWERVR_SGX530_121.so \
	libEGL_tegra.so \
	libGLESv1_CM_POWERVR_SGX530_121.so \
	libGLESv1_CM_tegra.so \
	libGLESv2_POWERVR_SGX530_121.so \
	libGLESv2_tegra.so \
	libmoto_ril.so \
	libpppd_plugin-ril.so \
	libril_rds.so \
	location \
	location.cfg \
	main.conf \
	mbm.bin \
	mbm_consumer.bin \
	mdm_panicd \
	monkey \
	pm \
	pppd-ril \
	pppd-ril.options \
	proc \
	qwerty.kl \
	radio.img \
	rdl.bin \
	RFFspeed_501.bmd \
	RFFstd_501.bmd \
	savebpver \
	sbin \
	sholes-keypad.kl \
	suplcerts.bks \
	svc \
	sys \
	system \
	tcmd \
	ueventd.goldfish.rc \
	ueventd.olympus.rc \
	ueventd.rc \
	ueventd.stingray.rc \
	vold.fstab \
	wl1271.bin \
        modules.order \
        oper.lis \
        sound \
        testpattern1.wav \
        ringtone.wav \
        images \
        lcd_test_00.png \
        lcd_test_01.png \
        lcd_test_02.png \
        lcd_test_03.png \
        lcd_test_04.png \
        lcd_test_05.png \
        lcd_test_06.png \
        lcd_test_07.png \
        matv \
        matv_pattern.jpg \
        d7e8dc79.0 \
        b727005e.0 \
        c692a373.0 \
        5f267794.0 \
        b6c5745d.0 \
        40547a79.0 \
        080911ac.0 \
        4f316efb.0 \
        2e4eed3c.0 \
        790a7190.0 \
        8160b96c.0 \
        ef2f636c.0 \
        a760e1bd.0 \
        c0ff1f52.0 \
        4304c5e5.0 \
        9b353c9a.0 \
        244b5494.0 \
        1ec4d31a.0 \
        57b0f75e.0 \
        3b2716e5.0 \
        4bfab552.0 \
        b1b8a7f3.0 \
        ae8153b9.0 \
        024dc131.0 \
        2b349938.0 \
        2e5ac55d.0 \
        653b494a.0 \
        706f604c.0 \
        e2799e36.0 \
        c47d9980.0 \
        c99398f3.0 \
        e113c810.0 \
        93bc0acc.0 \
        442adcac.0 \
        18856ac4.0 \
        062cdee6.0 \
        4a6481c9.0 \
        157753a5.0 \
        f3377b1b.0 \
        5c44d531.0 \
        5ad8a5d6.0 \
        2207fa1d.0 \
        b7e7231a.0 \
        12345d5c.0 \
        aee5f10d.0 \
        b13cc6df.0 \
        0996ae1d.0 \
        f39fc864.0 \
        5042e2e2.0 \
        02265526.0 \
        b66938e9.0 \
        9818ca0b.0 \
        f081611a.0 \
        039c618a.0 \
        fcac10e3.0 \
        cbf06781.0 \
        46117fcc.0 \
        ee64a828.0 \
        4f9ecf48.0 \
        667c66d4.0 \
        3513523f.0 \
        ce026bf8.0 \
        79ad8b43.0 \
        f90208f7.0 \
        7d5a75e4.0 \
        ad088e1d.0 \
        88f89ea7.0 \
        988a38cb.0 \
        c8841d13.0 \
        0b759015.0 \
        2cfc4974.0 \
        9af9f759.0 \
        3bde41ac.0 \
        8e52d3cd.0 \
        812e17de.0 \
        cbeee9e2.0 \
        a5fd78f0.0 \
        3ee7e181.0 \
        2c543cd1.0 \
        65b876bd.0 \
        cfa1c2ee.0 \
        2251b13a.0 \
        76faf6c0.0 \
        201cada0.0 \
        cb59f961.0 \
        6410666e.0 \
        9f541fb4.0 \
        778e3cb0.0 \
        bd1910d4.0 \
        381ce4dd.0 \
        578d5c04.0 \
        6f2c1157.0 \
        5620c4aa.0 \
        480720ec.0 \
        ba89ed3b.0 \
        d9d12c58.0 \
        9d520b32.0 \
        eacdeb40.0 \
        98ec67f0.0 \
        6b99d060.0 \
        a8dee976.0 \
        415660c1.0 \
        57bbd831.0 \
        b1159c4c.0 \
        57bcb2da.0 \
        3e45d192.0 \
        bad35b78.0 \
        09789157.0 \
        03f0efa4.0 \
        b204d74a.0 \
        55a10908.0 \
        20d096ba.0 \
        76cb8f92.0 \
        0c4c9b6c.0 \
        f060240e.0 \
        4597689c.0 \
        6faac4e3.0 \
        9c2e7d30.0 \
        a6a593ba.0 \
        c01cdfa2.0 \
        8867006a.0 \
        c0319fa7.0 \
        ca6e4ad9.0 \
        8d86cdd1.0 \
        6cc3c4c3.0 \
        2ab3b959.0 \
        f387163d.0 \
        b42ff584.0
