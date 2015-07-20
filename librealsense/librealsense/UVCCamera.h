#pragma once

#ifndef UVC_CAMERA_H
#define UVC_CAMERA_H

#include "libuvc/libuvc.h"

#include <stdint.h>
#include <string>
#include <vector>
#include <iostream>
#include <exception>
#include <mutex>
#include <map>

#include "Common.h"
#include "R200_XU.h"
#include "R200_SPI.h"
#include "R200_CameraHeader.h"

struct StreamInfo
{
    int width;
    int height;
    int fps;
    uvc_frame_format format;
};

struct DeviceInfo
{
    std::string serial;
    uint16_t vid;
    uint16_t pid;
};

class UVCCamera;
struct StreamHandle
{
    UVCCamera * camera = nullptr;
    uvc_frame_format fmt = UVC_FRAME_FORMAT_UNKNOWN;
    uvc_stream_ctrl_t ctrl = {0};
};

////////////////
// UVC Camera //
////////////////

class UVCCamera
{
    NO_MOVE(UVCCamera);
    
    uvc_device_t * device = nullptr;
    uvc_device_handle_t * deviceHandle = nullptr;
    
    bool isInitialized = false;
    bool isStreaming = false;
    
    uint64_t frameCount = 0;
    int cameraNum;
    
    StreamInfo sInfo = {};
    DeviceInfo dInfo = {};
    
    std::mutex frameMutex;
    std::unique_ptr<TripleBufferedFrame> depthFrame;
    std::unique_ptr<TripleBufferedFrame> colorFrame;
    
    std::unique_ptr<SPI_Interface> spiInterface;
    
    std::map<int, StreamHandle *> streamHandles;
    
    static void cb(uvc_frame_t * frame, void * ptr)
    {
        StreamHandle * handle = static_cast<StreamHandle*>(ptr);
        handle->camera->frameCallback(frame, handle);
    }
    
    void frameCallback(uvc_frame_t * frame, StreamHandle * handle);
    
public:
    
    UVCCamera(uvc_device_t * device, int num);
    
    ~UVCCamera();
    
    bool ProbeDevice(int deviceNum);
    
    void StartStream(int streamNum, const StreamInfo & info);
    
    void StopStream(int streamNum);
    
    // @tofix get rid of this function
    void DumpInfo();
    
    uint16_t * GetDepthImage();
    
    uint8_t * GetColorImage();
    
    int GetCameraIndex() { return cameraNum; }
    
    bool IsStreaming() { return isStreaming; }
    
    RectifiedIntrinsics GetCalibrationDataRectZ()
    {
        return spiInterface->GetZIntrinsics();
    }
    
};

#endif