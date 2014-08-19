#ifndef __IAALSERVICE_H__
#define __IAALSERVICE_H__

#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/BinderService.h>

namespace android
{
//
//  Holder service for pass objects between processes.
//
class IAALService : public IInterface 
{
protected:
    enum {
        AAL_SET_MODE = IBinder::FIRST_CALL_TRANSACTION,
        AAL_SET_BACKLIGHT_COLOR,
        AAL_SET_BACKLIGHT_BRIGHTNESS,
        AAL_SET_LIGHT_SENSOR_VALUE,
        AAL_SET_SCREEN_BRIGHTNESS,        
        AAL_SET_SCREEN_STATE,
        AAL_SET_LABC_LEVEL,
        AAL_SET_CABC_LEVEL,
        AAL_SET_DRE_LEVEL
    };

    enum {
        SCREEN_STATE_OFF = 0,
        SCREEN_STATE_ON = 1,
        SCREEN_STATE_DIM = 2
    };

public:
    DECLARE_META_INTERFACE(AALService);

    virtual status_t setMode(int32_t mode) = 0;
    virtual status_t setBacklightColor(int32_t color) = 0;
    virtual status_t setBacklightBrightness(int32_t brightness) = 0;
    virtual status_t setLightSensorValue(int32_t value) = 0;
    virtual status_t setScreenBrightness(int32_t brightness) = 0;
    virtual status_t setScreenState(int32_t state, int32_t brightness) = 0;
    
    virtual status_t setLABCLevel(int32_t level) = 0;
    virtual status_t setCABCLevel(int32_t level) = 0;
    virtual status_t setDRELevel(int32_t level) = 0;
};

class BnAALService : public BnInterface<IAALService> 
{
    virtual status_t onTransact(uint32_t code,
                                const Parcel& data,
                                Parcel* reply,
                                uint32_t flags = 0);
};    

};

#endif




