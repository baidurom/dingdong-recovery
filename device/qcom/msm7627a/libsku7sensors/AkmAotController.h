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

#ifndef ANDROID_AKM_AOT_CONTROLLER_H
#define ANDROID_AKM_AOT_CONTROLLER_H

#include <cstdint>

#include <utils/threads.h>

class AkmAotController {
public:
	AkmAotController();
	~AkmAotController();

private:
	/* non-copyable */
	AkmAotController(AkmAotController const &);
	AkmAotController& operator = (AkmAotController const &);

public:
	static AkmAotController & getInstance();

	enum {
		ACCEL_AXIS = 3,
	};

	enum SensorType {
		AKM_SENSOR_TYPE_ACCELEROMETER = 0,
		AKM_SENSOR_TYPE_MAGNETOMETER,
		AKM_SENSOR_TYPE_ORIENTATION,
		AKM_SENSOR_TYPE_UNKNOWN,
	};

	void writeAccels(int16_t value[ACCEL_AXIS]);

	bool getEnabled(SensorType sensorType);
	void setEnabled(SensorType sensorType, bool isEnabled);
	void setDelay(int64_t ns[3]);

	bool openDevice();
	void closeDevice();

private:
	int ioctl(unsigned int cmd, void *argp);
	bool openDevice_internal();
	void closeDevice_internal();

private:
	int aot_fd;
	short m_magEn;
	short m_accEn;
	short m_oriEn;
	android::Mutex mLock;
};

#endif // end of AKM_AOT_INTERFACE_H_

