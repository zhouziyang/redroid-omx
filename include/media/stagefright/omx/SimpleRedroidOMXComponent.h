/*
 * Copyright (C) 2011 The Android Open Source Project
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

#pragma once

#include "RedroidOMXComponent.h"

#include <atomic>
#include <media/stagefright/foundation/AHandlerReflector.h>
#include <utils/RefBase.h>
#include <utils/threads.h>
#include <utils/Vector.h>

namespace android {

struct ALooper;
struct ABuffer;

struct CodecProfileLevel {
    OMX_U32 mProfile;
    OMX_U32 mLevel;
};

struct SimpleRedroidOMXComponent : public RedroidOMXComponent {
    SimpleRedroidOMXComponent(
            const char *name,
            const OMX_CALLBACKTYPE *callbacks,
            OMX_PTR appData,
            OMX_COMPONENTTYPE **component);

    virtual void prepareForDestruction();

    void onMessageReceived(const sp<AMessage> &msg);

protected:
    struct BufferInfo {
        OMX_BUFFERHEADERTYPE *mHeader;
        bool mOwnedByUs;
        bool mFrameConfig;
    };

    struct PortInfo {
        OMX_PARAM_PORTDEFINITIONTYPE mDef;
        Vector<BufferInfo> mBuffers;
        List<BufferInfo *> mQueue;

        enum {
            NONE,
            DISABLING,
            ENABLING,
        } mTransition;
    };

    enum {
        kStoreMetaDataExtensionIndex = OMX_IndexVendorStartUnused + 1,
        kPrepareForAdaptivePlaybackIndex,
    };

    void addPort(const OMX_PARAM_PORTDEFINITIONTYPE &def);

    virtual OMX_ERRORTYPE internalGetParameter(
            OMX_INDEXTYPE index, OMX_PTR params);

    virtual OMX_ERRORTYPE internalSetParameter(
            OMX_INDEXTYPE index, const OMX_PTR params);

    virtual OMX_ERRORTYPE internalSetConfig(
            OMX_INDEXTYPE index, const OMX_PTR params, bool *frameConfig);

    virtual void onQueueFilled(OMX_U32 portIndex);
    List<BufferInfo *> &getPortQueue(OMX_U32 portIndex);

    virtual void onPortFlushCompleted(OMX_U32 portIndex);
    virtual void onPortEnableCompleted(OMX_U32 portIndex, bool enabled);
    virtual void onReset();

    PortInfo *editPortInfo(OMX_U32 portIndex);

private:
    enum {
        kWhatSendCommand,
        kWhatEmptyThisBuffer,
        kWhatFillThisBuffer,
    };

    Mutex mLock;

    sp<ALooper> mLooper;
    sp<AHandlerReflector<SimpleRedroidOMXComponent> > mHandler;

    OMX_STATETYPE mState;
    OMX_STATETYPE mTargetState;

    Vector<PortInfo> mPorts;
    std::atomic_bool mFrameConfig;

    bool isSetParameterAllowed(
            OMX_INDEXTYPE index, const OMX_PTR params) const;

    virtual OMX_ERRORTYPE sendCommand(
            OMX_COMMANDTYPE cmd, OMX_U32 param, OMX_PTR data);

    virtual OMX_ERRORTYPE getParameter(
            OMX_INDEXTYPE index, OMX_PTR params);

    virtual OMX_ERRORTYPE setParameter(
            OMX_INDEXTYPE index, const OMX_PTR params);

    virtual OMX_ERRORTYPE setConfig(
            OMX_INDEXTYPE index, const OMX_PTR params);

    virtual OMX_ERRORTYPE useBuffer(
            OMX_BUFFERHEADERTYPE **buffer,
            OMX_U32 portIndex,
            OMX_PTR appPrivate,
            OMX_U32 size,
            OMX_U8 *ptr);

    virtual OMX_ERRORTYPE allocateBuffer(
            OMX_BUFFERHEADERTYPE **buffer,
            OMX_U32 portIndex,
            OMX_PTR appPrivate,
            OMX_U32 size);

    virtual OMX_ERRORTYPE freeBuffer(
            OMX_U32 portIndex,
            OMX_BUFFERHEADERTYPE *buffer);

    virtual OMX_ERRORTYPE emptyThisBuffer(
            OMX_BUFFERHEADERTYPE *buffer);

    virtual OMX_ERRORTYPE fillThisBuffer(
            OMX_BUFFERHEADERTYPE *buffer);

    virtual OMX_ERRORTYPE getState(OMX_STATETYPE *state);

    void onSendCommand(OMX_COMMANDTYPE cmd, OMX_U32 param);
    void onChangeState(OMX_STATETYPE state);
    void onPortEnable(OMX_U32 portIndex, bool enable);
    void onPortFlush(OMX_U32 portIndex, bool sendFlushComplete);

    void checkTransitions();

    DISALLOW_EVIL_CONSTRUCTORS(SimpleRedroidOMXComponent);
};

}  // namespace android
