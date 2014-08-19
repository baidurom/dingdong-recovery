#define LOG_TAG "AALTOOL_JNI"

#include <jni.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <utils/Log.h>
#include <utils/Singleton.h>
#include <utils/StrongPointer.h>
#include <linux/sensors_io.h>
#include "IAALService.h"
 
using namespace android;

class AALToolService: public Singleton<AALToolService>
{
    friend class Singleton<AALToolService>;
public:
    static sp<IAALService> getAALService() {
        return getInstance().mAALService;
    }

    static int getALSDev() {
        return getInstance().mALSFd;
    }

private:    
    AALToolService() {
        const String16 serviceName("AAL");
        while (getService(serviceName, &mAALService) != NO_ERROR) {
            usleep(100000);
        }

        mALSFd = open("/dev/als_ps", O_RDONLY);
        if (mALSFd < 0)
            ALOGE("Fail to open alsps device (error: %s)\n", strerror(errno));
        else
            ALOGE("Open alsps device %d\n", mALSFd);        
    }

    ~AALToolService() {
        if (mALSFd >= 0)
            close(mALSFd);
    }

    
    sp<IAALService> mAALService;
    int mALSFd;
};


ANDROID_SINGLETON_STATIC_INSTANCE(AALToolService);


#ifdef __cplusplus
extern "C" {
#endif


JNIEXPORT jint JNICALL Java_com_mediatek_aaltool_AALALSCalibration_nGetALSRawData(JNIEnv * env, jobject jobj)
{
    int fd = AALToolService::getALSDev();
    if (fd != -1)
    {
        int err = 0;
        int als = 0;
        if ((err = ioctl(fd, ALSPS_GET_ALS_RAW_DATA, &als)))
        {
            ALOGE("ioctl ALSPS_GET_ALS_RAW_DATA error: %d\n", err);
            return -1;
        }
        ALOGD("als = %d\n", als);
        return als;
    }
    return -1;
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_aaltool_AALTuning_nSetLABCLevel(JNIEnv * env, jobject jobj, jint level)
{
    ALOGD("LABC level = %d", level);
    
    sp<IAALService> s(AALToolService::getAALService()); 
    if (s->setLABCLevel(level) != 0)
    {
        ALOGE("fail to set LABC level");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_aaltool_AALTuning_nSetCABCLevel(JNIEnv * env, jobject jobj, jint level)
{
    ALOGD("CABC level = %d", level);
    sp<IAALService> s(AALToolService::getAALService());
    if (s->setCABCLevel(level) != 0)
    {
        ALOGE("fail to set CABC level");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_aaltool_AALTuning_nSetDRELevel(JNIEnv * env, jobject jobj, jint level)
{
    ALOGD("DRE level = %d", level);
    sp<IAALService> s(AALToolService::getAALService());
    if (s->setDRELevel(level) != 0)
    {
        ALOGE("fail to set DRE level");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_aaltool_AALTuning_nSetLightSensorValue(JNIEnv * env, jobject jobj, jint value)
{
    ALOGD("Light sensor value = %d", value);
    sp<IAALService> s(AALToolService::getAALService());
    if (s->setLightSensorValue(value) != 0)
    {
        ALOGE("fail to set light sensor value");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_aaltool_AALTuning_nSetBacklight(JNIEnv * env, jobject jobj, jint level)
{
    ALOGD("Backlight level = %d", level);
    sp<IAALService> s(AALToolService::getAALService());
    if (s->setBacklightBrightness(level) != 0)
    {
        ALOGE("fail to set backlight level");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

#ifdef __cplusplus
}
#endif
