/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/* crypto/x509/x509_d2.c */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include <stdio.h>
#include "cryptlib.h"
#include <openssl/crypto.h>
#include <openssl/x509.h>

#ifndef OPENSSL_NO_STDIO
int X509_STORE_set_default_paths(X509_STORE *ctx)
	{
	X509_LOOKUP *lookup;

	lookup=X509_STORE_add_lookup(ctx,X509_LOOKUP_file());
	if (lookup == NULL) return(0);
	X509_LOOKUP_load_file(lookup,NULL,X509_FILETYPE_DEFAULT);

	lookup=X509_STORE_add_lookup(ctx,X509_LOOKUP_hash_dir());
	if (lookup == NULL) return(0);
	X509_LOOKUP_add_dir(lookup,NULL,X509_FILETYPE_DEFAULT);
	
	/* clear any errors */
	ERR_clear_error();

	return(1);
	}

int X509_STORE_load_locations(X509_STORE *ctx, const char *file,
		const char *path)
	{
	X509_LOOKUP *lookup;

	if (file != NULL)
		{
		lookup=X509_STORE_add_lookup(ctx,X509_LOOKUP_file());
		if (lookup == NULL) return(0);
		if (X509_LOOKUP_load_file(lookup,file,X509_FILETYPE_PEM) != 1)
		    return(0);
		}
	if (path != NULL)
		{
		lookup=X509_STORE_add_lookup(ctx,X509_LOOKUP_hash_dir());
		if (lookup == NULL) return(0);
		if (X509_LOOKUP_add_dir(lookup,path,X509_FILETYPE_PEM) != 1)
		    return(0);
		}
	if ((path == NULL) && (file == NULL))
		return(0);
	return(1);
	}
//!!== Chiwei [2010/10/30]: Android AGPS porting ==
/* add by Will for DER format certificate pool */ //add-->
int X509_STORE_set_default_paths_ext(X509_STORE *ctx, int filetype)
	{
	X509_LOOKUP *lookup;

    if (filetype != X509_FILETYPE_ASN1)
        return X509_STORE_set_default_paths(ctx);
       
	lookup=X509_STORE_add_lookup(ctx,X509_LOOKUP_file());
	if (lookup == NULL) return(0);
	X509_LOOKUP_load_file(lookup,NULL,X509_FILETYPE_ASN1);

	lookup=X509_STORE_add_lookup(ctx,X509_LOOKUP_hash_dir());
	if (lookup == NULL) return(0);
	X509_LOOKUP_add_dir(lookup,NULL,X509_FILETYPE_ASN1);
	
	/* clear any errors */
	ERR_clear_error();
	return(1);
	}

int X509_STORE_load_locations_ext(X509_STORE *ctx, const char *file,
		const char *path, int filetype)
	{
	X509_LOOKUP *lookup;

    if (filetype != X509_FILETYPE_ASN1)
        return X509_STORE_load_locations(ctx, file, path);

	if (file != NULL)
		{
		lookup=X509_STORE_add_lookup(ctx,X509_LOOKUP_file());
		if (lookup == NULL) return(0);
		if (X509_LOOKUP_load_file(lookup,file,X509_FILETYPE_ASN1) != 1)
		    return(0);
		}
	if (path != NULL)
		{
		lookup=X509_STORE_add_lookup(ctx,X509_LOOKUP_hash_dir());
		if (lookup == NULL) return(0);
		if (X509_LOOKUP_add_dir(lookup,path,X509_FILETYPE_ASN1) != 1)
		    return(0);
		}
	if ((path == NULL) && (file == NULL))
		return(0);
	return(1);
	}
/* add by Will for DER format certificate pool */ //add<--
#endif
