#ifndef __SimpleWork_AV_AvOut_h__
#define __SimpleWork_AV_AvOut_h__

#include "av.h"

SIMPLEWORK_AV_NAMESPACE_ENTER

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER(AvOut, "sw.av.AvOut")

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvOut", 211206)

        //
        // 写入帧
        //
        virtual int writeFrame(const SAvFrame& frame) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    SIMPLEWORK_INTERFACECLASS_ENTER(AvOutFactory, "sw.av.AvOutFactory")
        SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvOutFactory", 211206)
            virtual SAvOut openWindow(const char* szWindowName, CAvSampleMeta& sampleMeta) = 0;

            virtual SAvOut openSpeaker(const char* szName, CAvSampleMeta& sampleMeta) = 0;

            virtual SAvOut openAvFile(const char* szFileName, int nStreamings, SAvStreaming* pStreamings) = 0;
        SIMPLEWORK_INTERFACE_LEAVE
    SIMPLEWORK_INTERFACECLASS_LEAVE(AvOutFactory)

public:
    static inline SAvOut openWindow(const char* szWindowName, CAvSampleMeta& sampleMeta) {
        return getFactory()->openWindow(szWindowName, sampleMeta);
    }

    static inline SAvOut openSpeaker(const char* szName, CAvSampleMeta& sampleMeta) {
        return getFactory()->openSpeaker(szName, sampleMeta);
    }

private:
    static SAvOutFactory& getFactory() {
        static SAvOutFactory g_factory = SObject::createObject<SAvOutFactory>();
        return g_factory;
    }
SIMPLEWORK_INTERFACECLASS_LEAVE(AvOut)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_AvOut_h__