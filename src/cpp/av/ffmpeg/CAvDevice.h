#ifndef __SimpleWork_Av_CAvDevice_h__
#define __SimpleWork_Av_CAvDevice_h__

#include "av_ffmpeg.h"

FFMPEG_NAMESPACE_ENTER

class CAvDevice : public CObject, public IAudioDevice, public IVideoDevice {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAudioDevice)
        SIMPLEWORK_INTERFACE_ENTRY(IVideoDevice)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://IAvFrame
    const char* getName();
    const char* getDeviceName();
    int getNextDevice(SAudioDevice& rDevice);
    int getNextDevice(SVideoDevice& rDevice);

public:
    template<typename T> int processGetNextDevice(T& rDevice);
    static int s_getNextDevice(SVideoDevice& rDevice);
    static int s_getNextDevice(SAudioDevice& rDevice);
    static int s_getNextDevice(SVideoDevice& rDevice, AVInputFormat* pInputFormat);
    static int s_getNextDevice(SAudioDevice& rDevice, AVInputFormat* pInputFormat);

public:
    CAvDevice();

public:
    AVInputFormat* m_pInputFormat;
    CRefer<AVDeviceInfoList*> m_pDeviceLists;
    int m_iDevice;
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_Av_CAvDevice_h__