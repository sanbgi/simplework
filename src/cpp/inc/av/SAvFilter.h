#ifndef __SimpleWork_AV_SAvFilter_h__
#define __SimpleWork_AV_SAvFilter_h__

#include "av.h"

SIMPLEWORK_AV_NAMESPACE_ENTER

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER0(AvFilter)

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvFilter", 211206)
        //
        // 读取帧
        //
        virtual int pushFrame(const PAvFrame* pSrc, PAvFrame::FVisitor visitor) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    SIMPLEWORK_INTERFACECLASS_ENTER(AvFilterFactory, "sw.av.AvFilterFactory")
        SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvFilterFactory", 211206)
            //打开视频文件
            virtual int createFilter(const PAvSample& targetSample, SAvFilter& spFilter) = 0;

        SIMPLEWORK_INTERFACE_LEAVE
    SIMPLEWORK_INTERFACECLASS_LEAVE(AvFilterFactory)

public:
    static inline int createFilter(const PAvSample& targetSample, SAvFilter& spFilter) {
        return getFactory()->createFilter(targetSample, spFilter);
    }

private:
    static SAvFilterFactory& getFactory() {
        static SAvFilterFactory g_factory = SObject::createObject<SAvFilterFactory>();
        return g_factory;
    }
SIMPLEWORK_INTERFACECLASS_LEAVE(AvFilter)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_SAvFilter_h__