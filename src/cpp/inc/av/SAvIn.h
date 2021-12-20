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
        // 访问所有流数据
        //
        virtual int visitStreamings(PAvStreaming::FVisitor visitor) = 0;

        //
        // 读取一帧数据，可能是视频帧，也可能是音频帧
        //
        virtual int readFrame(PAvFrame::FVisitor visitor) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    SIMPLEWORK_INTERFACECLASS_ENTER(AvInFactory, "sw.av.AvInFactory")
        SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvInFactory", 211206)
            //打开视频文件
            virtual SAvIn openVideoFile(const char* szFileName) = 0;

            //打开摄像头
            virtual SAvIn openVideoDevice(const char* szName) = 0;

            //打开摄像头
            virtual SAvIn openAudioDevice(const char* szName) = 0;

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

private:
    static SAvInFactory& getFactory() {
        static SAvInFactory g_factory = SObject::createObject<SAvInFactory>();
        return g_factory;
    }
SIMPLEWORK_INTERFACECLASS_LEAVE(AvIn)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_AvIn_h__