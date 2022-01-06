
#include "CAvDevice.h"
#include "CAvIn.h"

FFMPEG_NAMESPACE_ENTER

static SCtx sCtx("CAvDevice");

CAvDevice::CAvDevice() {
    m_pInputFormat = nullptr;
}

const char* CAvDevice::getName() {
    return m_pInputFormat->name;
}

const char* CAvDevice::getDeviceName() {
    return m_pDeviceLists->devices[m_iDevice]->device_name;
}

//
// 获取当前设备的下一个设备
//
int CAvDevice::s_getNextDevice(SVideoDevice& rDevice) {
    CAvIn::initDeviceRegistry();
    if(rDevice) {
        return rDevice->getNextDevice(rDevice);
    }
    return s_getNextDevice(rDevice, nullptr);
}

int CAvDevice::s_getNextDevice(SAudioDevice& rDevice) {
    CAvIn::initDeviceRegistry();
    if(rDevice) {
        return rDevice->getNextDevice(rDevice);
    }
    return s_getNextDevice(rDevice, nullptr);
}

//
// 获取指定输入设备格式的下一个设备
//
int CAvDevice::s_getNextDevice(SVideoDevice& rDevice, AVInputFormat* pInputFormat) {
    pInputFormat = av_input_video_device_next(pInputFormat);
    if(pInputFormat == nullptr) {
        return sCtx.error();
    }

    CPointer<CAvDevice> spDevice;
    CObject::createObject(spDevice);
    spDevice->m_pInputFormat = pInputFormat;
    return spDevice->processGetNextDevice(rDevice);
}

int CAvDevice::s_getNextDevice(SAudioDevice& rDevice, AVInputFormat* pInputFormat) {
    pInputFormat = av_input_audio_device_next(pInputFormat);
    if(pInputFormat == nullptr) {
        return sCtx.error();
    }
    
    CPointer<CAvDevice> spDevice;
    CObject::createObject(spDevice);
    spDevice->m_pInputFormat = pInputFormat;
    return spDevice->processGetNextDevice(rDevice);
}

template<typename T> int CAvDevice::processGetNextDevice(T& rDevice) {
    if(!m_pDeviceLists) {
        AVDeviceInfoList* pDeviceLists = nullptr;
        int nDevices = avdevice_list_input_sources(m_pInputFormat, nullptr, nullptr, &pDeviceLists);
        if( nDevices < 0 ) {
            return sCtx.error();
        }
        m_pDeviceLists.take(pDeviceLists, [](AVDeviceInfoList* pPtr){avdevice_free_list_devices(&pPtr);});
        if(nDevices==0) {
            return s_getNextDevice(rDevice, m_pInputFormat);
        }

        m_iDevice = 0;
        rDevice.setPtr(this);
        return sCtx.success();
    }

    if( m_iDevice < m_pDeviceLists->nb_devices - 1) {

        CPointer<CAvDevice> spDevice;
        CObject::createObject(spDevice);
        spDevice->m_iDevice = m_iDevice+1;
        spDevice->m_pDeviceLists = m_pDeviceLists;
        spDevice->m_pInputFormat = m_pInputFormat;
    }

    return s_getNextDevice(rDevice, m_pInputFormat);
}

int CAvDevice::getNextDevice(SAudioDevice& rDevice) {
    return processGetNextDevice(rDevice);
}

int CAvDevice::getNextDevice(SVideoDevice& rDevice) {
    return processGetNextDevice(rDevice);
}

FFMPEG_NAMESPACE_LEAVE
