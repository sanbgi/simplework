#ifndef __SimpleWork_AV_AvIn_h__
#define __SimpleWork_AV_AvIn_h__

#include "av.h"

SIMPLEWORK_AV_NAMESPACE_ENTER
class SAudioDevice;
class SVideoDevice;

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER0(AvIn)

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvIn", 211206)

        //
        // 获取流
        //
        virtual int getStreaming(SAvStreaming& rStreaming) = 0;

        //
        // 修改流采样数据格式
        //
        virtual int changeStreamingSampleMeta(int iStreamingId, const CAvSampleMeta& sampleMeta) = 0;

        //
        // 获取帧
        //
        virtual int readFrame(int& iStreamingId, SAvFrame& rFrame) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    SIMPLEWORK_INTERFACECLASS_ENTER(AvInFactory, "sw.av.AvInFactory")
        SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvInFactory", 211206)
            //打开视频文件
            virtual SAvIn openVideoFile(const char* szFileName) = 0;

            //打开摄像头
            virtual SAvIn openVideoDevice(const char* szName) = 0;

            //打开摄像头
            virtual SAvIn openAudioDevice(const char* szName) = 0;

            //获取系统音频输入设备
            virtual int getAudioDevice(SAudioDevice& rDevice) = 0;

            //获取系统视频输入设备
            virtual int getVideoDevice(SVideoDevice& rDevice) = 0;
        SIMPLEWORK_INTERFACE_LEAVE
    SIMPLEWORK_INTERFACECLASS_LEAVE(AvInFactory)

public:
    static inline SAvIn openVideoFile(const char* szFileName) {
        return getFactory()->openVideoFile(szFileName);
    }
    static inline SAvIn openVideoDevice(const char* szDeviceName) {
        return getFactory()->openVideoDevice(szDeviceName);
    }
    static inline SAvIn openAudioDevice(const char* szDeviceName) {
        return getFactory()->openAudioDevice(szDeviceName);
    }
    static inline int getAudioDevice(SAudioDevice& rDevice) {
        return getFactory()->getAudioDevice(rDevice);
    }
    static inline int getVideoDevice(SVideoDevice& rDevice) {
        return getFactory()->getVideoDevice(rDevice);
    }

private:
    static SAvInFactory& getFactory() {
        static SAvInFactory g_factory = SObject::createObject<SAvInFactory>();
        return g_factory;
    }
SIMPLEWORK_INTERFACECLASS_LEAVE(AvIn)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_AvIn_h__