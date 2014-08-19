/*
 * Copyright (C) Texas Instruments - http://www.ti.com/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
* @file OMXExif.cpp
*
* This file contains functionality for handling EXIF insertion.
*
*/

#undef LOG_TAG

#define LOG_TAG "CameraHAL"

#include "CameraHal.h"
#include "OMXCameraAdapter.h"
#include <math.h>

namespace android {

status_t OMXCameraAdapter::setParametersEXIF(const CameraParameters &params,
                                             BaseCameraAdapter::AdapterState state)
{
    status_t ret = NO_ERROR;
    const char *valstr = NULL;
    double gpsPos;

    LOG_FUNCTION_NAME;

    if( ( valstr = params.get(CameraParameters::KEY_GPS_LATITUDE) ) != NULL )
        {
        gpsPos = strtod(valstr, NULL);

        if ( convertGPSCoord(gpsPos,
                             mEXIFData.mGPSData.mLatDeg,
                             mEXIFData.mGPSData.mLatMin,
                             mEXIFData.mGPSData.mLatSec,
                             mEXIFData.mGPSData.mLatSecDiv ) == NO_ERROR )
            {

            if ( 0 < gpsPos )
                {
                strncpy(mEXIFData.mGPSData.mLatRef, GPS_NORTH_REF, GPS_REF_SIZE);
                }
            else
                {
                strncpy(mEXIFData.mGPSData.mLatRef, GPS_SOUTH_REF, GPS_REF_SIZE);
                }

            mEXIFData.mGPSData.mLatValid = true;
            }
        else
            {
            mEXIFData.mGPSData.mLatValid = false;
            }
        }
    else
        {
        mEXIFData.mGPSData.mLatValid = false;
        }

    if( ( valstr = params.get(CameraParameters::KEY_GPS_LONGITUDE) ) != NULL )
        {
        gpsPos = strtod(valstr, NULL);

        if ( convertGPSCoord(gpsPos,
                             mEXIFData.mGPSData.mLongDeg,
                             mEXIFData.mGPSData.mLongMin,
                             mEXIFData.mGPSData.mLongSec,
                             mEXIFData.mGPSData.mLongSecDiv) == NO_ERROR )
            {

            if ( 0 < gpsPos )
                {
                strncpy(mEXIFData.mGPSData.mLongRef, GPS_EAST_REF, GPS_REF_SIZE);
                }
            else
                {
                strncpy(mEXIFData.mGPSData.mLongRef, GPS_WEST_REF, GPS_REF_SIZE);
                }

            mEXIFData.mGPSData.mLongValid= true;
            }
        else
            {
            mEXIFData.mGPSData.mLongValid = false;
            }
        }
    else
        {
        mEXIFData.mGPSData.mLongValid = false;
        }

    if( ( valstr = params.get(CameraParameters::KEY_GPS_ALTITUDE) ) != NULL )
        {
        gpsPos = strtod(valstr, NULL);
        mEXIFData.mGPSData.mAltitude = floor(fabs(gpsPos));
        if (gpsPos < 0) {
            mEXIFData.mGPSData.mAltitudeRef = 1;
        } else {
            mEXIFData.mGPSData.mAltitudeRef = 0;
        }
        mEXIFData.mGPSData.mAltitudeValid = true;
        }
    else
        {
        mEXIFData.mGPSData.mAltitudeValid= false;
        }

    if( (valstr = params.get(CameraParameters::KEY_GPS_TIMESTAMP)) != NULL )
        {
        long gpsTimestamp = strtol(valstr, NULL, 10);
        struct tm *timeinfo = gmtime( ( time_t * ) & (gpsTimestamp) );
        if ( NULL != timeinfo )
            {
            mEXIFData.mGPSData.mTimeStampHour = timeinfo->tm_hour;
            mEXIFData.mGPSData.mTimeStampMin = timeinfo->tm_min;
            mEXIFData.mGPSData.mTimeStampSec = timeinfo->tm_sec;
            mEXIFData.mGPSData.mTimeStampValid = true;
            }
        else
            {
            mEXIFData.mGPSData.mTimeStampValid = false;
            }
        }
    else
        {
        mEXIFData.mGPSData.mTimeStampValid = false;
        }

    if( ( valstr = params.get(CameraParameters::KEY_GPS_TIMESTAMP) ) != NULL )
        {
        long gpsDatestamp = strtol(valstr, NULL, 10);
        struct tm *timeinfo = gmtime( ( time_t * ) & (gpsDatestamp) );
        if ( NULL != timeinfo )
            {
            strftime(mEXIFData.mGPSData.mDatestamp, GPS_DATESTAMP_SIZE, "%Y:%m:%d", timeinfo);
            mEXIFData.mGPSData.mDatestampValid = true;
            }
        else
            {
            mEXIFData.mGPSData.mDatestampValid = false;
            }
        }
    else
        {
        mEXIFData.mGPSData.mDatestampValid = false;
        }

    if( ( valstr = params.get(CameraParameters::KEY_GPS_PROCESSING_METHOD) ) != NULL )
        {
        strncpy(mEXIFData.mGPSData.mProcMethod, valstr, GPS_PROCESSING_SIZE-1);
        mEXIFData.mGPSData.mProcMethodValid = true;
        }
    else
        {
        mEXIFData.mGPSData.mProcMethodValid = false;
        }

    if( ( valstr = params.get(TICameraParameters::KEY_GPS_MAPDATUM) ) != NULL )
        {
        strncpy(mEXIFData.mGPSData.mMapDatum, valstr, GPS_MAPDATUM_SIZE-1);
        mEXIFData.mGPSData.mMapDatumValid = true;
        }
    else
        {
        mEXIFData.mGPSData.mMapDatumValid = false;
        }

    if( ( valstr = params.get(TICameraParameters::KEY_GPS_VERSION) ) != NULL )
        {
        strncpy(mEXIFData.mGPSData.mVersionId, valstr, GPS_VERSION_SIZE-1);
        mEXIFData.mGPSData.mVersionIdValid = true;
        }
    else
        {
        mEXIFData.mGPSData.mVersionIdValid = false;
        }

    if( ( valstr = params.get(TICameraParameters::KEY_EXIF_MODEL ) ) != NULL )
        {
        CAMHAL_LOGVB("EXIF Model: %s", valstr);
        mEXIFData.mModelValid= true;
        }
    else
        {
        mEXIFData.mModelValid= false;
        }

    if( ( valstr = params.get(TICameraParameters::KEY_EXIF_MAKE ) ) != NULL )
        {
        CAMHAL_LOGVB("EXIF Make: %s", valstr);
        mEXIFData.mMakeValid = true;
        }
    else
        {
        mEXIFData.mMakeValid= false;
        }

    LOG_FUNCTION_NAME_EXIT;

    return ret;
}

status_t OMXCameraAdapter::setupEXIF()
{
    status_t ret = NO_ERROR;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_TI_CONFIG_SHAREDBUFFER sharedBuffer;
    OMX_TI_CONFIG_EXIF_TAGS *exifTags;
    unsigned char *sharedPtr = NULL;
    struct timeval sTv;
    struct tm *pTime;
    OMXCameraPortParameters * capData = NULL;
    MemoryManager memMgr;
    OMX_U8** memmgr_buf_array = NULL;
    int buf_size = 0;

    LOG_FUNCTION_NAME;

    sharedBuffer.pSharedBuff = NULL;
    capData = &mCameraAdapterParameters.mCameraPortParams[mCameraAdapterParameters.mImagePortIndex];

    if ( OMX_StateInvalid == mComponentState )
        {
        CAMHAL_LOGEA("OMX component is in invalid state");
        ret = -EINVAL;
        }

    if ( NO_ERROR == ret )
        {
        OMX_INIT_STRUCT_PTR (&sharedBuffer, OMX_TI_CONFIG_SHAREDBUFFER);
        sharedBuffer.nPortIndex = mCameraAdapterParameters.mImagePortIndex;

        //We allocate the shared buffer dynamically based on the
        //requirements of the EXIF tags. The additional buffers will
        //get stored after the EXIF configuration structure and the pointers
        //will contain offsets within the shared buffer itself.
        buf_size = sizeof(OMX_TI_CONFIG_EXIF_TAGS) +
                          ( EXIF_MODEL_SIZE ) +
                          ( EXIF_MAKE_SIZE ) +
                          ( EXIF_DATE_TIME_SIZE ) +
                          ( GPS_MAPDATUM_SIZE ) +
                          ( GPS_PROCESSING_SIZE );
        buf_size = ((buf_size+4095)/4096)*4096;
        sharedBuffer.nSharedBuffSize = buf_size;

        memmgr_buf_array = (OMX_U8 **)memMgr.allocateBuffer(0, 0, NULL, buf_size, 1);
        sharedBuffer.pSharedBuff =  ( OMX_U8 * ) memmgr_buf_array[0];

        if ( NULL == sharedBuffer.pSharedBuff )
            {
            CAMHAL_LOGEA("No resources to allocate OMX shared buffer");
            ret = -1;
            }

        //Extra data begins right after the EXIF configuration structure.
        sharedPtr = sharedBuffer.pSharedBuff + sizeof(OMX_TI_CONFIG_EXIF_TAGS);
        }

    if ( NO_ERROR == ret )
        {
        exifTags = ( OMX_TI_CONFIG_EXIF_TAGS * ) sharedBuffer.pSharedBuff;
        OMX_INIT_STRUCT_PTR (exifTags, OMX_TI_CONFIG_EXIF_TAGS);
        exifTags->nPortIndex = mCameraAdapterParameters.mImagePortIndex;

        eError = OMX_GetConfig(mCameraAdapterParameters.mHandleComp,
                               ( OMX_INDEXTYPE ) OMX_TI_IndexConfigExifTags,
                               &sharedBuffer );
        if ( OMX_ErrorNone != eError )
            {
            CAMHAL_LOGEB("Error while retrieving EXIF configuration structure 0x%x", eError);
            ret = -1;
            }
        }

    if ( NO_ERROR == ret )
        {
        if ( ( OMX_TI_TagReadWrite == exifTags->eStatusModel ) &&
              ( mEXIFData.mModelValid ) )
            {
            strncpy(( char * ) sharedPtr,
                    ( char * ) mParams.get(TICameraParameters::KEY_EXIF_MODEL ),
                    EXIF_MODEL_SIZE - 1);

            exifTags->pModelBuff = ( OMX_S8 * ) ( sharedPtr - sharedBuffer.pSharedBuff );
            exifTags->ulModelBuffSizeBytes = strlen((char*)sharedPtr) + 1;
            sharedPtr += EXIF_MODEL_SIZE;
            exifTags->eStatusModel = OMX_TI_TagUpdated;
            }

         if ( ( OMX_TI_TagReadWrite == exifTags->eStatusMake) &&
               ( mEXIFData.mMakeValid ) )
             {
             strncpy( ( char * ) sharedPtr,
                          ( char * ) mParams.get(TICameraParameters::KEY_EXIF_MAKE ),
                          EXIF_MAKE_SIZE - 1);

             exifTags->pMakeBuff = ( OMX_S8 * ) ( sharedPtr - sharedBuffer.pSharedBuff );
             exifTags->ulMakeBuffSizeBytes = strlen((char*)sharedPtr) + 1;
             sharedPtr += EXIF_MAKE_SIZE;
             exifTags->eStatusMake = OMX_TI_TagUpdated;
             }

        if ( ( OMX_TI_TagReadWrite == exifTags->eStatusFocalLength ))
        {
            unsigned int numerator = 0, denominator = 0;
            ExifElementsTable::stringToRational(mParams.get(CameraParameters::KEY_FOCAL_LENGTH),
                                                &numerator, &denominator);
            if (numerator || denominator) {
                exifTags->ulFocalLength[0] = (OMX_U32) numerator;
                exifTags->ulFocalLength[1] = (OMX_U32) denominator;
                CAMHAL_LOGVB("exifTags->ulFocalLength = [%u] [%u]",
                             (unsigned int)(exifTags->ulFocalLength[0]),
                             (unsigned int)(exifTags->ulFocalLength[1]));
                exifTags->eStatusFocalLength = OMX_TI_TagUpdated;
            }
        }

         if ( OMX_TI_TagReadWrite == exifTags->eStatusDateTime )
             {
             int status = gettimeofday (&sTv, NULL);
             pTime = localtime (&sTv.tv_sec);
             if ( ( 0 == status ) && ( NULL != pTime ) )
                {
                snprintf(( char * ) sharedPtr, EXIF_DATE_TIME_SIZE,
                         "%04d:%02d:%02d %02d:%02d:%02d",
                         pTime->tm_year + 1900,
                         pTime->tm_mon + 1,
                         pTime->tm_mday,
                         pTime->tm_hour,
                         pTime->tm_min,
                         pTime->tm_sec );
                }

             exifTags->pDateTimeBuff = ( OMX_S8 * ) ( sharedPtr - sharedBuffer.pSharedBuff );
             sharedPtr += EXIF_DATE_TIME_SIZE;
             exifTags->ulDateTimeBuffSizeBytes = EXIF_DATE_TIME_SIZE;
             exifTags->eStatusDateTime = OMX_TI_TagUpdated;
             }

         if ( OMX_TI_TagReadWrite == exifTags->eStatusImageWidth )
             {
             exifTags->ulImageWidth = capData->mWidth;
             exifTags->eStatusImageWidth = OMX_TI_TagUpdated;
             }

         if ( OMX_TI_TagReadWrite == exifTags->eStatusImageHeight )
             {
             exifTags->ulImageHeight = capData->mHeight;
             exifTags->eStatusImageHeight = OMX_TI_TagUpdated;
             }

         if ( ( OMX_TI_TagReadWrite == exifTags->eStatusGpsLatitude ) &&
              ( mEXIFData.mGPSData.mLatValid ) )
            {
            exifTags->ulGpsLatitude[0] = abs(mEXIFData.mGPSData.mLatDeg);
            exifTags->ulGpsLatitude[2] = abs(mEXIFData.mGPSData.mLatMin);
            exifTags->ulGpsLatitude[4] = abs(mEXIFData.mGPSData.mLatSec);
            exifTags->ulGpsLatitude[1] = 1;
            exifTags->ulGpsLatitude[3] = 1;
            exifTags->ulGpsLatitude[5] = abs(mEXIFData.mGPSData.mLatSecDiv);
            exifTags->eStatusGpsLatitude = OMX_TI_TagUpdated;
            }

        if ( ( OMX_TI_TagReadWrite == exifTags->eStatusGpslatitudeRef ) &&
             ( mEXIFData.mGPSData.mLatValid ) )
            {
            exifTags->cGpslatitudeRef[0] = ( OMX_S8 ) mEXIFData.mGPSData.mLatRef[0];
            exifTags->cGpslatitudeRef[1] = '\0';
            exifTags->eStatusGpslatitudeRef = OMX_TI_TagUpdated;
            }

         if ( ( OMX_TI_TagReadWrite == exifTags->eStatusGpsLongitude ) &&
              ( mEXIFData.mGPSData.mLongValid ) )
            {
            exifTags->ulGpsLongitude[0] = abs(mEXIFData.mGPSData.mLongDeg);
            exifTags->ulGpsLongitude[2] = abs(mEXIFData.mGPSData.mLongMin);
            exifTags->ulGpsLongitude[4] = abs(mEXIFData.mGPSData.mLongSec);
            exifTags->ulGpsLongitude[1] = 1;
            exifTags->ulGpsLongitude[3] = 1;
            exifTags->ulGpsLongitude[5] = abs(mEXIFData.mGPSData.mLongSecDiv);
            exifTags->eStatusGpsLongitude = OMX_TI_TagUpdated;
            }

        if ( ( OMX_TI_TagReadWrite == exifTags->eStatusGpsLongitudeRef ) &&
             ( mEXIFData.mGPSData.mLongValid ) )
            {
            exifTags->cGpsLongitudeRef[0] = ( OMX_S8 ) mEXIFData.mGPSData.mLongRef[0];
            exifTags->cGpsLongitudeRef[1] = '\0';
            exifTags->eStatusGpsLongitudeRef = OMX_TI_TagUpdated;
            }

        if ( ( OMX_TI_TagReadWrite == exifTags->eStatusGpsAltitude ) &&
             ( mEXIFData.mGPSData.mAltitudeValid) )
            {
            exifTags->ulGpsAltitude[0] = ( OMX_U32 ) mEXIFData.mGPSData.mAltitude;
            exifTags->ulGpsAltitude[1] = 1;
            exifTags->eStatusGpsAltitude = OMX_TI_TagUpdated;
            }

        if ( ( OMX_TI_TagReadWrite == exifTags->eStatusGpsAltitudeRef ) &&
             ( mEXIFData.mGPSData.mAltitudeValid) )
            {
            exifTags->ucGpsAltitudeRef = (OMX_U8) mEXIFData.mGPSData.mAltitudeRef;
            exifTags->eStatusGpsAltitudeRef = OMX_TI_TagUpdated;
            }

        if ( ( OMX_TI_TagReadWrite == exifTags->eStatusGpsMapDatum ) &&
             ( mEXIFData.mGPSData.mMapDatumValid ) )
            {
            memcpy(sharedPtr, mEXIFData.mGPSData.mMapDatum, GPS_MAPDATUM_SIZE);

            exifTags->pGpsMapDatumBuff = ( OMX_S8 * ) ( sharedPtr - sharedBuffer.pSharedBuff );
            exifTags->ulGpsMapDatumBuffSizeBytes = GPS_MAPDATUM_SIZE;
            exifTags->eStatusGpsMapDatum = OMX_TI_TagUpdated;
            sharedPtr += GPS_MAPDATUM_SIZE;
            }

        if ( ( OMX_TI_TagReadWrite == exifTags->eStatusGpsProcessingMethod ) &&
             ( mEXIFData.mGPSData.mProcMethodValid ) )
            {
            exifTags->pGpsProcessingMethodBuff = ( OMX_S8 * ) ( sharedPtr - sharedBuffer.pSharedBuff );
            memcpy(sharedPtr, ExifAsciiPrefix, sizeof(ExifAsciiPrefix));
            sharedPtr += sizeof(ExifAsciiPrefix);

            memcpy(sharedPtr,
                   mEXIFData.mGPSData.mProcMethod,
                   ( GPS_PROCESSING_SIZE - sizeof(ExifAsciiPrefix) ) );
            exifTags->ulGpsProcessingMethodBuffSizeBytes = GPS_PROCESSING_SIZE;
            exifTags->eStatusGpsProcessingMethod = OMX_TI_TagUpdated;
            sharedPtr += GPS_PROCESSING_SIZE;
            }

        if ( ( OMX_TI_TagReadWrite == exifTags->eStatusGpsVersionId ) &&
             ( mEXIFData.mGPSData.mVersionIdValid ) )
            {
            exifTags->ucGpsVersionId[0] = ( OMX_U8 ) mEXIFData.mGPSData.mVersionId[0];
            exifTags->ucGpsVersionId[1] =  ( OMX_U8 ) mEXIFData.mGPSData.mVersionId[1];
            exifTags->ucGpsVersionId[2] = ( OMX_U8 ) mEXIFData.mGPSData.mVersionId[2];
            exifTags->ucGpsVersionId[3] = ( OMX_U8 ) mEXIFData.mGPSData.mVersionId[3];
            exifTags->eStatusGpsVersionId = OMX_TI_TagUpdated;
            }

        if ( ( OMX_TI_TagReadWrite == exifTags->eStatusGpsTimeStamp ) &&
             ( mEXIFData.mGPSData.mTimeStampValid ) )
            {
            exifTags->ulGpsTimeStamp[0] = mEXIFData.mGPSData.mTimeStampHour;
            exifTags->ulGpsTimeStamp[2] = mEXIFData.mGPSData.mTimeStampMin;
            exifTags->ulGpsTimeStamp[4] = mEXIFData.mGPSData.mTimeStampSec;
            exifTags->ulGpsTimeStamp[1] = 1;
            exifTags->ulGpsTimeStamp[3] = 1;
            exifTags->ulGpsTimeStamp[5] = 1;
            exifTags->eStatusGpsTimeStamp = OMX_TI_TagUpdated;
            }

        if ( ( OMX_TI_TagReadWrite == exifTags->eStatusGpsDateStamp ) &&
             ( mEXIFData.mGPSData.mDatestampValid ) )
            {
            strncpy( ( char * ) exifTags->cGpsDateStamp,
                         ( char * ) mEXIFData.mGPSData.mDatestamp,
                         GPS_DATESTAMP_SIZE );
            exifTags->eStatusGpsDateStamp = OMX_TI_TagUpdated;
            }

        eError = OMX_SetConfig(mCameraAdapterParameters.mHandleComp,
                               ( OMX_INDEXTYPE ) OMX_TI_IndexConfigExifTags,
                               &sharedBuffer );

        if ( OMX_ErrorNone != eError )
            {
            CAMHAL_LOGEB("Error while setting EXIF configuration 0x%x", eError);
            ret = -1;
            }
        }

    if ( NULL != memmgr_buf_array )
        {
        memMgr.freeBuffer(memmgr_buf_array);
        }

    LOG_FUNCTION_NAME_EXIT;

    return ret;
}

status_t OMXCameraAdapter::setupEXIF_libjpeg(ExifElementsTable* exifTable)
{
    status_t ret = NO_ERROR;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    struct timeval sTv;
    struct tm *pTime;
    OMXCameraPortParameters * capData = NULL;

    LOG_FUNCTION_NAME;

    capData = &mCameraAdapterParameters.mCameraPortParams[mCameraAdapterParameters.mImagePortIndex];

    if ((NO_ERROR == ret) && (mEXIFData.mModelValid)) {
        ret = exifTable->insertElement(TAG_MODEL, mParams.get(TICameraParameters::KEY_EXIF_MODEL));
    }

     if ((NO_ERROR == ret) && (mEXIFData.mMakeValid)) {
        ret = exifTable->insertElement(TAG_MAKE, mParams.get(TICameraParameters::KEY_EXIF_MAKE));
     }

    if ((NO_ERROR == ret)) {
        unsigned int numerator = 0, denominator = 0;
        ExifElementsTable::stringToRational(mParams.get(CameraParameters::KEY_FOCAL_LENGTH),
                                            &numerator, &denominator);
        if (numerator || denominator) {
            char temp_value[256]; // arbitrarily long string
            snprintf(temp_value,
                 sizeof(temp_value)/sizeof(char),
                 "%u/%u", numerator, denominator);
            ret = exifTable->insertElement(TAG_FOCALLENGTH, temp_value);

        }
    }

    if ((NO_ERROR == ret)) {
        int status = gettimeofday (&sTv, NULL);
        pTime = localtime (&sTv.tv_sec);
        char temp_value[EXIF_DATE_TIME_SIZE + 1];
        if ((0 == status) && (NULL != pTime)) {
            snprintf(temp_value, EXIF_DATE_TIME_SIZE,
                     "%04d:%02d:%02d %02d:%02d:%02d",
                     pTime->tm_year + 1900,
                     pTime->tm_mon + 1,
                     pTime->tm_mday,
                     pTime->tm_hour,
                     pTime->tm_min,
                     pTime->tm_sec );

            ret = exifTable->insertElement(TAG_DATETIME, temp_value);
        }
     }

    if ((NO_ERROR == ret)) {
        char temp_value[5];
        snprintf(temp_value, sizeof(temp_value)/sizeof(char), "%lu", capData->mWidth);
        ret = exifTable->insertElement(TAG_IMAGE_WIDTH, temp_value);
     }

    if ((NO_ERROR == ret)) {
        char temp_value[5];
        snprintf(temp_value, sizeof(temp_value)/sizeof(char), "%lu", capData->mHeight);
        ret = exifTable->insertElement(TAG_IMAGE_LENGTH, temp_value);
     }

    if ((NO_ERROR == ret) && (mEXIFData.mGPSData.mLatValid)) {
        char temp_value[256]; // arbitrarily long string
        snprintf(temp_value,
                 sizeof(temp_value)/sizeof(char) - 1,
                 "%d/%d,%d/%d,%d/%d",
                 abs(mEXIFData.mGPSData.mLatDeg), 1,
                 abs(mEXIFData.mGPSData.mLatMin), 1,
                 abs(mEXIFData.mGPSData.mLatSec), abs(mEXIFData.mGPSData.mLatSecDiv));
        ret = exifTable->insertElement(TAG_GPS_LAT, temp_value);
    }

    if ((NO_ERROR == ret) && (mEXIFData.mGPSData.mLatValid)) {
        ret = exifTable->insertElement(TAG_GPS_LAT_REF, mEXIFData.mGPSData.mLatRef);
    }

    if ((NO_ERROR == ret) && (mEXIFData.mGPSData.mLongValid)) {
        char temp_value[256]; // arbitrarily long string
        snprintf(temp_value,
                 sizeof(temp_value)/sizeof(char) - 1,
                 "%d/%d,%d/%d,%d/%d",
                 abs(mEXIFData.mGPSData.mLongDeg), 1,
                 abs(mEXIFData.mGPSData.mLongMin), 1,
                 abs(mEXIFData.mGPSData.mLongSec), abs(mEXIFData.mGPSData.mLongSecDiv));
        ret = exifTable->insertElement(TAG_GPS_LONG, temp_value);
    }

    if ((NO_ERROR == ret) && (mEXIFData.mGPSData.mLongValid)) {
        ret = exifTable->insertElement(TAG_GPS_LONG_REF, mEXIFData.mGPSData.mLongRef);
    }

    if ((NO_ERROR == ret) && (mEXIFData.mGPSData.mAltitudeValid)) {
        char temp_value[256]; // arbitrarily long string
        snprintf(temp_value,
                 sizeof(temp_value)/sizeof(char) - 1,
                 "%d/%d",
                 abs( mEXIFData.mGPSData.mAltitude), 1);
        ret = exifTable->insertElement(TAG_GPS_ALT, temp_value);
    }

    if ((NO_ERROR == ret) && (mEXIFData.mGPSData.mAltitudeValid)) {
        char temp_value[5];
        snprintf(temp_value,
                 sizeof(temp_value)/sizeof(char) - 1,
                 "%d", mEXIFData.mGPSData.mAltitudeRef);
        ret = exifTable->insertElement(TAG_GPS_ALT_REF, temp_value);
    }

    if ((NO_ERROR == ret) && (mEXIFData.mGPSData.mMapDatumValid)) {
        ret = exifTable->insertElement(TAG_GPS_MAP_DATUM, mEXIFData.mGPSData.mMapDatum);
    }

    if ((NO_ERROR == ret) && (mEXIFData.mGPSData.mProcMethodValid)) {
        char temp_value[GPS_PROCESSING_SIZE];

        memcpy(temp_value, ExifAsciiPrefix, sizeof(ExifAsciiPrefix));
        memcpy(temp_value + sizeof(ExifAsciiPrefix),
               mParams.get(CameraParameters::KEY_GPS_PROCESSING_METHOD),
               (GPS_PROCESSING_SIZE - sizeof(ExifAsciiPrefix)));
        ret = exifTable->insertElement(TAG_GPS_PROCESSING_METHOD, temp_value);
    }

    if ((NO_ERROR == ret) && (mEXIFData.mGPSData.mVersionIdValid)) {
        char temp_value[256]; // arbitrarily long string
        snprintf(temp_value,
                 sizeof(temp_value)/sizeof(char) - 1,
                 "%d,%d,%d,%d",
                 mEXIFData.mGPSData.mVersionId[0],
                 mEXIFData.mGPSData.mVersionId[1],
                 mEXIFData.mGPSData.mVersionId[2],
                 mEXIFData.mGPSData.mVersionId[3]);
        ret = exifTable->insertElement(TAG_GPS_VERSION_ID, temp_value);
    }

    if ((NO_ERROR == ret) && (mEXIFData.mGPSData.mTimeStampValid)) {
        char temp_value[256]; // arbitrarily long string
        snprintf(temp_value,
                 sizeof(temp_value)/sizeof(char) - 1,
                 "%d/%d,%d/%d,%d/%d",
                 mEXIFData.mGPSData.mTimeStampHour, 1,
                 mEXIFData.mGPSData.mTimeStampMin, 1,
                 mEXIFData.mGPSData.mTimeStampSec, 1);
        ret = exifTable->insertElement(TAG_GPS_TIMESTAMP, temp_value);
    }

    if ((NO_ERROR == ret) && (mEXIFData.mGPSData.mDatestampValid) ) {
        ret = exifTable->insertElement(TAG_GPS_DATESTAMP, mEXIFData.mGPSData.mDatestamp);
    }

    if ((NO_ERROR == ret) && mParams.get(CameraParameters::KEY_ROTATION) ) {
        const char* exif_orient =
           ExifElementsTable::degreesToExifOrientation(mParams.get(CameraParameters::KEY_ROTATION));

        if (exif_orient) {
           ret = exifTable->insertElement(TAG_ORIENTATION, exif_orient);
        }
    }

    LOG_FUNCTION_NAME_EXIT;

    return ret;
}

status_t OMXCameraAdapter::convertGPSCoord(double coord,
                                           int &deg,
                                           int &min,
                                           int &sec,
                                           int &secDivisor)
{
    double tmp;

    LOG_FUNCTION_NAME;

    if ( coord == 0 ) {

        LOGE("Invalid GPS coordinate");

        return -EINVAL;
    }

    deg = (int) floor(fabs(coord));
    tmp = ( fabs(coord) - floor(fabs(coord)) ) * GPS_MIN_DIV;
    min = (int) floor(tmp);
    tmp = ( tmp - floor(tmp) ) * ( GPS_SEC_DIV * GPS_SEC_ACCURACY );
    sec = (int) floor(tmp);
    secDivisor = GPS_SEC_ACCURACY;

    if( sec >= ( GPS_SEC_DIV * GPS_SEC_ACCURACY ) ) {
        sec = 0;
        min += 1;
    }

    if( min >= 60 ) {
        min = 0;
        deg += 1;
    }

    LOG_FUNCTION_NAME_EXIT;

    return NO_ERROR;
}

};
