/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef ANDROID_ACC_SENSOR_H
#define ANDROID_ACC_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>


#include "sensors.h"
#include "SensorBase.h"
#include "InputEventReader.h"

/*****************************************************************************/

struct input_event;

class AccSensor : public SensorBase {
public:
            AccSensor();
    virtual ~AccSensor();

    virtual int setDelay(int32_t handle, int64_t ns);
    virtual int setEnable(int32_t handle, int enabled);
    virtual int getEnable(int32_t handle);
    virtual int readEvents(sensors_event_t* data, int count);
    void processEvent(int code, int value);

private:
	int sensor_get_class_path(char *class_path);
	int is_sensor_enabled();
	int enable_sensor();
	int disable_sensor();
	int set_delay(int64_t ns);

	int update_delay();

	int readDisable();
	int writeDisable(int isDisable);
	int writeDelay(int64_t ns);

    enum {
        Primary = 0,
        Secondary,
        numChannels
    };

    uint32_t mEnabled;
    uint32_t mPendingMask;
    char mClassPath[PATH_MAX];
    InputEventCircularReader mInputReader;
    sensors_event_t mPendingEvent;
    int64_t mDelay;
};

/*****************************************************************************/

#endif  // ANDROID_Acc_SENSOR_H
