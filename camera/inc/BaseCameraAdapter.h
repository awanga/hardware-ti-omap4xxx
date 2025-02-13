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



#ifndef BASE_CAMERA_ADAPTER_H
#define BASE_CAMERA_ADAPTER_H

#include "CameraHal.h"

namespace android {

class BaseCameraAdapter : public CameraAdapter
{

public:

    BaseCameraAdapter();
    virtual ~BaseCameraAdapter();

    ///Initialzes the camera adapter creates any resources required
    virtual status_t initialize(CameraProperties::Properties*) = 0;

    virtual int setErrorHandler(ErrorNotifier *errorNotifier);

    //Message/Frame notification APIs
    virtual void enableMsgType(int32_t msgs, frame_callback callback=NULL, event_callback eventCb=NULL, void* cookie=NULL);
    virtual void disableMsgType(int32_t msgs, void* cookie);
    virtual void returnFrame(void * frameBuf, CameraFrame::FrameType frameType);
    virtual void addFramePointers(void *frameBuf, void *y_uv);
    virtual void removeFramePointers();

    //APIs to configure Camera adapter and get the current parameter set
    virtual status_t setParameters(const hardware::camera::common::V1_0::helper::CameraParameters& params) = 0;
    virtual void getParameters(hardware::camera::common::V1_0::helper::CameraParameters& params)  = 0;

    //API to send a command to the camera
    virtual status_t sendCommand(CameraCommands operation, int value1 = 0, int value2 = 0, int value3 = 0 );

    virtual status_t registerImageReleaseCallback(release_image_buffers_callback callback, void *user_data);

    virtual status_t registerEndCaptureCallback(end_image_capture_callback callback, void *user_data);

    //Retrieves the current Adapter state
    virtual AdapterState getState();
    //Retrieves the next Adapter state
    virtual AdapterState getNextState();

    // Rolls the state machine back to INTIALIZED_STATE from the current state
    virtual status_t rollbackToInitializedState();

protected:
    //The first two methods will try to switch the adapter state.
    //Every call to setState() should be followed by a corresponding
    //call to commitState(). If the state switch fails, then it will
    //get reset to the previous state via rollbackState().
    virtual status_t setState(CameraCommands operation);
    virtual status_t commitState();
    virtual status_t rollbackState();

    // Retrieves the current Adapter state - for internal use (not locked)
    virtual status_t getState(AdapterState &state);
    // Retrieves the next Adapter state - for internal use (not locked)
    virtual status_t getNextState(AdapterState &state);

    //-----------Interface that needs to be implemented by deriving classes --------------------

    //Should be implmented by deriving classes in order to start image capture
    virtual status_t takePicture();

    //Should be implmented by deriving classes in order to start image capture
    virtual status_t stopImageCapture();

    //Should be implmented by deriving classes in order to start temporal bracketing
    virtual status_t startBracketing(int range);

    //Should be implemented by deriving classes in order to stop temporal bracketing
    virtual status_t stopBracketing();

    //Should be implemented by deriving classes in oder to initiate autoFocus
    virtual status_t autoFocus();

    //Should be implemented by deriving classes in oder to initiate autoFocus
    virtual status_t cancelAutoFocus();

    //Should be called by deriving classes in order to do some bookkeeping
    virtual status_t startVideoCapture();

    //Should be called by deriving classes in order to do some bookkeeping
    virtual status_t stopVideoCapture();

    //Should be implemented by deriving classes in order to start camera preview
    virtual status_t startPreview();

    //Should be implemented by deriving classes in order to stop camera preview
    virtual status_t stopPreview();

    //Should be implemented by deriving classes in order to start smooth zoom
    virtual status_t startSmoothZoom(int targetIdx);

    //Should be implemented by deriving classes in order to stop smooth zoom
    virtual status_t stopSmoothZoom();

    //Should be implemented by deriving classes in order to stop smooth zoom
    virtual status_t useBuffers(CameraMode mode, void* bufArr, int num, size_t length, unsigned int queueable);

    //Should be implemented by deriving classes in order queue a released buffer in CameraAdapter
    virtual status_t fillThisBuffer(void* frameBuf, CameraFrame::FrameType frameType);

    //API to get the frame size required to be allocated. This size is used to override the size passed
    //by camera service when VSTAB/VNF is turned ON for example
    virtual status_t getFrameSize(size_t &width, size_t &height);

    //API to get required data frame size
    virtual status_t getFrameDataSize(size_t &dataFrameSize, size_t bufferCount);

    //API to get required picture buffers size with the current configuration in hardware::camera::common::V1_0::helper::CameraParameters
    virtual status_t getPictureBufferSize(size_t &length, size_t bufferCount);

    // Should be implemented by deriving classes in order to start face detection
    // ( if supported )
    virtual status_t startFaceDetection();

    // Should be implemented by deriving classes in order to stop face detection
    // ( if supported )
    virtual status_t stopFaceDetection();

    virtual status_t switchToExecuting();

    // Receive orientation events from CameraHal
    virtual void onOrientationEvent(uint32_t orientation, uint32_t tilt);

    // ---------------------Interface ends-----------------------------------

    status_t notifyFocusSubscribers(CameraHalEvent::FocusStatus status);
    status_t notifyShutterSubscribers();
    status_t notifyZoomSubscribers(int zoomIdx, bool targetReached);
    status_t notifyFaceSubscribers(sp<CameraFDResult> &faces);

    //Send the frame to subscribers
    status_t sendFrameToSubscribers(CameraFrame *frame);

    //Resets the refCount for this particular frame
    status_t resetFrameRefCount(CameraFrame &frame);

    //A couple of helper functions
    void setFrameRefCount(void* frameBuf, CameraFrame::FrameType frameType, int refCount);
    int getFrameRefCount(void* frameBuf, CameraFrame::FrameType frameType);
    int setInitFrameRefCount(void* buf, unsigned int mask);

// private member functions
private:
    status_t __sendFrameToSubscribers(CameraFrame* frame,
                                      KeyedVector<int, frame_callback> *subscribers,
                                      CameraFrame::FrameType frameType);
    status_t rollbackToPreviousState();

// protected data types and variables
protected:
    enum FrameState {
        STOPPED = 0,
        RUNNING
    };

    enum FrameCommands {
        START_PREVIEW = 0,
        START_RECORDING,
        RETURN_FRAME,
        STOP_PREVIEW,
        STOP_RECORDING,
        DO_AUTOFOCUS,
        TAKE_PICTURE,
        FRAME_EXIT
    };

    enum AdapterCommands {
        ACK = 0,
        ERROR
    };

#if PPM_INSTRUMENTATION || PPM_INSTRUMENTATION_ABS

    struct timeval mStartFocus;
    struct timeval mStartCapture;

#endif

    mutable Mutex mReturnFrameLock;

    //Lock protecting the Adapter state
    mutable Mutex mLock;
    AdapterState mAdapterState;
    AdapterState mNextState;

    //Different frame subscribers get stored using these
    KeyedVector<int, frame_callback> mFrameSubscribers;
    KeyedVector<int, frame_callback> mFrameDataSubscribers;
    KeyedVector<int, frame_callback> mVideoSubscribers;
    KeyedVector<int, frame_callback> mImageSubscribers;
    KeyedVector<int, frame_callback> mRawSubscribers;
    KeyedVector<int, event_callback> mFocusSubscribers;
    KeyedVector<int, event_callback> mZoomSubscribers;
    KeyedVector<int, event_callback> mShutterSubscribers;
    KeyedVector<int, event_callback> mFaceSubscribers;

    //Preview buffer management data
    int *mPreviewBuffers;
    int mPreviewBufferCount;
    size_t mPreviewBuffersLength;
    KeyedVector<int, int> mPreviewBuffersAvailable;
    mutable Mutex mPreviewBufferLock;

    //Video buffer management data
    int *mVideoBuffers;
    KeyedVector<int, int> mVideoBuffersAvailable;
    int mVideoBuffersCount;
    size_t mVideoBuffersLength;
    mutable Mutex mVideoBufferLock;

    //Image buffer management data
    int *mCaptureBuffers;
    KeyedVector<int, bool> mCaptureBuffersAvailable;
    int mCaptureBuffersCount;
    size_t mCaptureBuffersLength;
    mutable Mutex mCaptureBufferLock;

    //Metadata buffermanagement
    int *mPreviewDataBuffers;
    KeyedVector<int, bool> mPreviewDataBuffersAvailable;
    int mPreviewDataBuffersCount;
    size_t mPreviewDataBuffersLength;
    mutable Mutex mPreviewDataBufferLock;

    TIUTILS::MessageQueue mFrameQ;
    TIUTILS::MessageQueue mAdapterQ;
    mutable Mutex mSubscriberLock;
    ErrorNotifier *mErrorNotifier;
    release_image_buffers_callback mReleaseImageBuffersCallback;
    end_image_capture_callback mEndImageCaptureCallback;
    void *mReleaseData;
    void *mEndCaptureData;
    bool mRecording;

    uint32_t mFramesWithDucati;
    uint32_t mFramesWithDisplay;
    uint32_t mFramesWithEncoder;

#ifdef DEBUG_LOG
    KeyedVector<int, bool> mBuffersWithDucati;
#endif

    KeyedVector<void *, CameraFrame *> mFrameQueue;
};

};

#endif //BASE_CAMERA_ADAPTER_H


