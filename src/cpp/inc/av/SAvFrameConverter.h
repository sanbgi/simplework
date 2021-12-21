#ifndef __SimpleWork_AV_SAvFrameConverter_h__
#define __SimpleWork_AV_SAvFrameConverter_h__

#include "av.h"

SIMPLEWORK_AV_NAMESPACE_ENTER

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER0(AvFrameConverter)

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvFrameConverter", 211206)
        //
        // 读取帧
        //
        virtual int pushFrame(const PAvFrame* pSrc, PAvFrame::FVisitor visitor) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    SIMPLEWORK_INTERFACECLASS_ENTER(AvFrameConverterFactory, "sw.av.AvFrameConverterFactory")
        SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvFrameConverterFactory", 211206)
            //打开视频文件
            virtual int createFilter(const PAvSample& targetSample, SAvFrameConverter& spFilter) = 0;

        SIMPLEWORK_INTERFACE_LEAVE
    SIMPLEWORK_INTERFACECLASS_LEAVE(AvFrameConverterFactory)

public:
    static inline int createFilter(const PAvSample& targetSample, SAvFrameConverter& spFilter) {
        return getFactory()->createFilter(targetSample, spFilter);
    }

private:
    static SAvFrameConverterFactory& getFactory() {
        static SAvFrameConverterFactory g_factory = SObject::createObject<SAvFrameConverterFactory>();
        return g_factory;
    }
SIMPLEWORK_INTERFACECLASS_LEAVE(AvFrameConverter)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_SAvFrameConverter_h__