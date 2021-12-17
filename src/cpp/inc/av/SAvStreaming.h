#ifndef __SimpleWork_IO_AvStreaming_h__
#define __SimpleWork_IO_AvStreaming_h__

#include "PAvStreaming.h"

SIMPLEWORK_AV_NAMESPACE_ENTER

//
// AV流
//
SIMPLEWORK_INTERFACECLASS_ENTER0(AvStreaming)
public:

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvStreaming", 211206)

        //
        // 获取流的序号
        //
        virtual int getStreamingId() = 0;

        //
        // 获取时钟频率
        //
        virtual int getTimeRate() = 0;

        //
        // 获得基于时钟频率的视频时长
        //
        virtual long getDuration() = 0;

        //
        // 获取流数据类型
        //
        virtual EAvSampleType getSampleType() = 0;

        //
        // 获取样本元信息，只有在版本冲突情况下，才会返回失败
        //
        virtual const PAvSample& getSampleMeta() = 0;

    SIMPLEWORK_INTERFACE_LEAVE
    
    SIMPLEWORK_INTERFACECLASS_ENTER(AvStreamingFactory, "sw.av.AvStreamingFactory")
        SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvStreamingFactory", 211214)
            //
            // 创建流对象
            //
            virtual SAvStreaming createStreaming(
                                int iStreamingId,
                                int nTimeRate,
                                EAvSampleType eStreamingType, 
                                const PAvSample& sampleMeta) = 0;

        SIMPLEWORK_INTERFACE_LEAVE
    SIMPLEWORK_INTERFACECLASS_LEAVE(AvStreamingFactory)

    static SAvStreamingFactory& getFactory() {
        static SAvStreamingFactory g_factory = SObject::createObject<SAvStreamingFactory>();
        return g_factory;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(AvStreaming)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_IO_AvStreaming_h__