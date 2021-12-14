#ifndef __SimpleWork_IO_AvFrame_h__
#define __SimpleWork_IO_AvFrame_h__

#include "av.h"
#include "SAvStreaming.h"

SIMPLEWORK_AV_NAMESPACE_ENTER 

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER0(AvFrame)
public:
    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvFrame", 211206)
        EAvStreamingType getStreamingType() {
            return getStreaming()->getStreamingType();
        }

        int getStreamingId() {
            return getStreaming()->getStreamingId();
        }

        const CAvSampleMeta& getSampleMeta() {
            return getStreaming()->getSampleMeta();
        }

        //
        // 获取流
        //
        virtual SAvStreaming& getStreaming() = 0;

        //
        // 获取当前帧的数据, 张量数据格式为
        //     视频：
        //          张量维度：width * height * bytes ()
        //          张量类型：unsigned char
        //     音频：
        //          张量维度：样本数量 * 通道数
        //          张量类型：unsigned char，short
        //
        virtual sw::math::STensor& getData() = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    SIMPLEWORK_INTERFACECLASS_ENTER(AvFrameFactory, "sw.av.AvFrameFactory")
        SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvFrameFactory", 211214)
            //
            // 创建流对象
            //
            virtual SAvFrame createFrame(
                                SAvStreaming& rStreaming,
                                sw::math::STensor& spData ) = 0;

        SIMPLEWORK_INTERFACE_LEAVE
    SIMPLEWORK_INTERFACECLASS_LEAVE(AvFrameFactory)

SIMPLEWORK_INTERFACECLASS_LEAVE(AvFrame)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_IO_AvFrame_h__