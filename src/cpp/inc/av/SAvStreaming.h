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
        // 获取流数据类型
        //
        virtual EAvStreamingType getStreamingType() = 0;

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
                                EAvStreamingType eStreamingType, 
                                const PAvSample& sampleMeta) = 0;

        SIMPLEWORK_INTERFACE_LEAVE
    SIMPLEWORK_INTERFACECLASS_LEAVE(AvStreamingFactory)

    SAvStreaming createVideoStream(int iStreamingId, int nTimeRate, int nWidth, int nHeight) {
        PAvSample sampleMeta;
        sampleMeta.sampleType = EAvSampleType::AvSampleType_Video_RGB;
        sampleMeta.videoWidth = nWidth;
        sampleMeta.videoHeight = nHeight;
        return getFactory()->createStreaming(iStreamingId, nTimeRate, EAvStreamingType::AvStreamingType_Video, sampleMeta);
    }

    SAvStreaming createAudioStream(int iStreamingId, int nTimeRate, int nSampleRate, int nChannels) {
        PAvSample sampleMeta;
        sampleMeta.sampleType = EAvSampleType::AvSampleType_Audio_S16;
        sampleMeta.audioRate = nSampleRate;
        sampleMeta.audioChannels = nChannels;
        return getFactory()->createStreaming(iStreamingId, nTimeRate, EAvStreamingType::AvStreamingType_Audio, sampleMeta);
    }

    static SAvStreamingFactory& getFactory() {
        static SAvStreamingFactory g_factory = SObject::createObject<SAvStreamingFactory>();
        return g_factory;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(AvStreaming)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_IO_AvStreaming_h__