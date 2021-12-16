#ifndef __SimpleWork_IO_AvStreaming_h__
#define __SimpleWork_IO_AvStreaming_h__

#include "av.h"
#include "SAvSample.h"

SIMPLEWORK_AV_NAMESPACE_ENTER

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER0(AvStreaming)
public:
    //
    // 音视频样本类型
    //
    enum EAvStreamingType {
        AvStreamingType_None,
        AvStreamingType_Video,
        AvStreamingType_Audio,
    };


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
        virtual const CAvSampleMeta& getSampleMeta() = 0;

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
                                const CAvSampleMeta& sampleMeta) = 0;

        SIMPLEWORK_INTERFACE_LEAVE
    SIMPLEWORK_INTERFACECLASS_LEAVE(AvStreamingFactory)

    SAvStreaming createVideoStream(int iStreamingId, int nTimeRate, int nWidth, int nHeight) {
        CAvSampleMeta sampleMeta;
        sampleMeta.eSampleType = EAvSampleType::AvSampleType_Video_RGB;
        sampleMeta.nVideoWidth = nWidth;
        sampleMeta.nVideoHeight = nHeight;
        return getFactory()->createStreaming(iStreamingId, nTimeRate, EAvStreamingType::AvStreamingType_Video, sampleMeta);
    }

    SAvStreaming createAudioStream(int iStreamingId, int nTimeRate, int nSampleRate, int nChannels) {
        CAvSampleMeta sampleMeta;
        sampleMeta.eSampleType = EAvSampleType::AvSampleType_Audio_S16;
        sampleMeta.nAudioRate = nSampleRate;
        sampleMeta.nAudioChannels = nChannels;
        return getFactory()->createStreaming(iStreamingId, nTimeRate, EAvStreamingType::AvStreamingType_Audio, sampleMeta);
    }

    static SAvStreamingFactory& getFactory() {
        static SAvStreamingFactory g_factory = SObject::createObject<SAvStreamingFactory>();
        return g_factory;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(AvStreaming)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_IO_AvStreaming_h__