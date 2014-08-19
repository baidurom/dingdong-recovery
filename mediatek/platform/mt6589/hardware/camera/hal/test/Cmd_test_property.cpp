/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

//
#include <utils/Log.h>
#include <binder/Parcel.h>
#include <cutils/memory.h>

//
#include <camera/ICamera.h>
#include <camera/CameraParameters.h>
#include <camera/MtkCameraParameters.h>
//
#include "inc/CamLog.h"
#include "inc/Utils.h"
#include "inc/Command.h"
#if defined(HAVE_COMMAND_property)
//
using namespace android;
//
//
//
/******************************************************************************
 *  Command
 *      property <set -k=key -v=value> <get -k=key> <-h>
 *
 *      -h:                     help
 *      <set -k=key -v=value>   set camera property by a pair of key/value
 *      <get -k=key>            get camera property by a key
 *
 ******************************************************************************/
namespace NSCmd_property {
struct CmdImp : public CmdBase
{
    static bool                 isInstantiate;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CmdBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Interface.
                                CmdImp(char const* szCmdName)
                                    : CmdBase(szCmdName)
                                {}

    virtual bool                execute(Vector<String8>& rvCmd);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
                                struct Argument : LightRefBase<Argument>
                                {
                                    String8     ms8Key;
                                    String8     ms8Value;
                                };

protected:  ////                Implementation.
    virtual bool                onParseArgumentCommand(Vector<String8>& rvCmd, sp<Argument> pArgument);
    virtual bool                onParseActionCommand(Vector<String8>& rvCmd, sp<Argument> pArgument);

    virtual bool                setProperty(String8 const& key, String8 const& value);
    virtual bool                getProperty(String8 const& key);

protected:  ////                Implementation.

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                Data Members (Parameters)
    sp<Argument>                mpArgument;

};
/******************************************************************************
 *
 ******************************************************************************/
bool CmdImp::isInstantiate = CmdMap::inst().addCommand(HAVE_COMMAND_property, new CmdImp(HAVE_COMMAND_property));
};  // NSCmd_test_preview
using namespace NSCmd_property;


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
execute(Vector<String8>& rvCmd)
{
    sp<Argument> pArgument = new Argument;
    onParseArgumentCommand(rvCmd, pArgument);
    return  onParseActionCommand(rvCmd, pArgument);
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onParseArgumentCommand(Vector<String8>& rvCmd, sp<Argument> pArgument)
{
    //  (1) Set default.
    pArgument->ms8Key   = String8::empty();
    pArgument->ms8Value = String8::empty();

    //  (2) Start to parse commands.
    for (size_t i = 1; i < rvCmd.size(); i++)
    {
        String8 const& s8Cmd = rvCmd[i];
        String8 key, val;
        if  ( ! parseOneCmdArgument(s8Cmd, key, val) ) {
            continue;
        }
//        MY_LOGD("<key/val>=<%s/%s>", key.string(), val.string());
        //
        //
        if  ( key == "-k" ) {
            pArgument->ms8Key = val;
            continue;
        }
        //
        if  ( key == "-v" ) {
            pArgument->ms8Value = val;
            continue;
        }
        //
    }
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onParseActionCommand(Vector<String8>& rvCmd, sp<Argument> pArgument)
{
    //  (1) Start to parse ACTION commands.
    for (size_t i = 1; i < rvCmd.size(); i++)
    {
        String8 const& s8Cmd = rvCmd[i];
        //
        if  ( s8Cmd == "-h" ) {
            String8 text;
            text += "\n";
            text += "\n   property <set -k=key -v=value> <get -k=key> <-h>";
            text += "\n   <-h>:                   help";
            text += "\n   <set -k=key -v=value>   set camera property by a pair of key/value";
            text += "\n   <get -k=key>            get camera property by a key";
            text += "\n";
            text += "\n   ex: set -k=KEY -v=VALUE";
            text += "\n   ex: get -k=KEY";
            MY_LOGD("%s", text.string());
            return  true;
        }
        //
        if  ( s8Cmd == "set" ) {
            return  setProperty(pArgument->ms8Key, pArgument->ms8Value);
        }
        //
        if  ( s8Cmd == "get" ) {
            return  getProperty(pArgument->ms8Key);
        }
        //
    }
    return  false;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
setProperty(String8 const& key, String8 const& value)
{
    status_t status = Camera::setProperty(key, value);
    if  ( OK == status ) {
        MY_LOGD("status[%d:%s] (%s)=(%s)", status, ::strerror(-status), key.string(), value.string());
    }
    else {
        MY_LOGE("status[%d:%s] (%s)=(%s)", status, ::strerror(-status), key.string(), value.string());
    }
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
getProperty(String8 const& key)
{
    String8 value;
    status_t status = Camera::getProperty(key, value);
    if  ( OK == status ) {
        MY_LOGD("status[%d:%s] (%s)=(%s)", status, ::strerror(-status), key.string(), value.string());
    }
    else {
        MY_LOGE("status[%d:%s] (%s)=(%s)", status, ::strerror(-status), key.string(), value.string());
    }
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
#endif  //  HAVE_COMMAND_xxx

