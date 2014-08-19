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

#ifndef ANDROID_SF_LAYER_STATE_H
#define ANDROID_SF_LAYER_STATE_H

#include <stdint.h>
#include <sys/types.h>

#include <utils/Errors.h>

#include <ui/Region.h>
#include <ui/Rect.h>
#include <gui/ISurface.h>

namespace android {

class Parcel;
class ISurfaceComposerClient;

struct layer_state_t {


    enum {
        eLayerHidden        = 0x01,
    };

    enum {
        ePositionChanged            = 0x00000001,
        eLayerChanged               = 0x00000002,
        eSizeChanged                = 0x00000004,
        eAlphaChanged               = 0x00000008,
        eMatrixChanged              = 0x00000010,
        eTransparentRegionChanged   = 0x00000020,
        eVisibilityChanged          = 0x00000040,
        eLayerStackChanged          = 0x00000080,
        eCropChanged                = 0x00000100,
    };

    // [MTK] {{{
    // setFlagsEx for extra layer control
    enum {
        eExInvalid           = 0x80000000,   // as need to update

        // BYTE#3 LOW4 for PQ control
        eExPQ_Mask           = 0x01000000,   // for PQ on/off
        eExPQ_On             = 0x01000000,

        eExPQ_Reserved_Mask  = 0x0E000000,   // reserved
        eExPQ_Reserved_bit0  = 0x02000000,
        eExPQ_Reserved_bit1  = 0x04000000,
        eExPQ_Reserved_bit2  = 0x08000000,

        // BYTE#2 for S3D layer control
        eExS3D_Mask          = 0x00FF0000,

        eExS3D_Layout_Mask   = 0x00F00000,   // for content layout
        eExS3D_Unknown       = 0x00100000,
        eExS3D_SideBySide    = 0x00200000,
        eExS3D_TopAndBottom  = 0x00400000,
        eExS3D_LRSwapped     = 0x00800000,

        eExS3D_Display_Mask  = 0x00080000,   // for display mode
        eExS3D_2D            = 0x00000000,
        eExS3D_3D            = 0x00080000,

        eExS3D_Reserved_Mask = 0x00070000,   // reserved
        eExS3D_Reserved_bit0 = 0x00010000,
        eExS3D_Reserved_bit1 = 0x00020000,
        eExS3D_Reserved_bit2 = 0x00040000,

        eExInitValue         = eExS3D_Unknown, // for layer init
    };
    // [MTK] }}}

    layer_state_t()
        :   surface(0), what(0),
            x(0), y(0), z(0), w(0), h(0), layerStack(0),
            alpha(0), flags(0), mask(0),
            reserved(0)
    {
        matrix.dsdx = matrix.dtdy = 1.0f;
        matrix.dsdy = matrix.dtdx = 0.0f;
        crop.makeInvalid();

        // [MTK] {{{
        // For setting extra surface flags
        flagsEx = 0x00000000;
        maskEx = 0x00000000;
        // [MTK] }}}
    }

    status_t    write(Parcel& output) const;
    status_t    read(const Parcel& input);

            struct matrix22_t {
                float   dsdx;
                float   dtdx;
                float   dsdy;
                float   dtdy;
            };
            SurfaceID       surface;
            uint32_t        what;
            float           x;
            float           y;
            uint32_t        z;
            uint32_t        w;
            uint32_t        h;
            uint32_t        layerStack;
            float           alpha;
            uint8_t         flags;
            uint8_t         mask;
            uint8_t         reserved;
            matrix22_t      matrix;
            Rect            crop;

            // [MTK] {{{
            // For setting extra surface flags
            uint32_t        flagsEx;
            uint32_t        maskEx;
            // [MTK] }}}

            // non POD must be last. see write/read
            Region          transparentRegion;
};

struct ComposerState {
    sp<ISurfaceComposerClient> client;
    layer_state_t state;
    status_t    write(Parcel& output) const;
    status_t    read(const Parcel& input);
};

struct DisplayState {

    enum {
        eOrientationDefault     = 0,
        eOrientation90          = 1,
        eOrientation180         = 2,
        eOrientation270         = 3,
        eOrientationUnchanged   = 4,
        eOrientationSwapMask    = 0x01
    };

    enum {
        eSurfaceChanged             = 0x01,
        eLayerStackChanged          = 0x02,
        eDisplayProjectionChanged   = 0x04
    };

    uint32_t what;
    sp<IBinder> token;
    sp<ISurfaceTexture> surface;
    uint32_t layerStack;
    uint32_t orientation;
    Rect viewport;
    Rect frame;
    status_t write(Parcel& output) const;
    status_t read(const Parcel& input);
};

}; // namespace android

#endif // ANDROID_SF_LAYER_STATE_H

