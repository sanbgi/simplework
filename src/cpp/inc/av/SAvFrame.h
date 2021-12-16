#ifndef __SimpleWork_IO_AvFrame_h__
#define __SimpleWork_IO_AvFrame_h__

#include "PAvFrame.h"
#include "SAvStreaming.h"
SIMPLEWORK_AV_NAMESPACE_ENTER 

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER0(AvFrame)
public:
    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvFrame", 211206)
        //
        // 获取流
        //
        virtual SAvStreaming& getStreaming() = 0;

        //
        // 获取时间戳，单位是SAvStreaming::getTimeRate对应的时间单位
        //
        virtual long getTimeStamp() = 0;

        //
        // 获取当前帧的数据, 张量数据格式为
        //     视频：
        //          张量维度：height(行) * width(列) * bytes (单像素字节数)
        //          张量类型：unsigned char
        //     音频：
        //          张量维度：采样次数 * 通道数 * bytes (单采样点字节数)
        //          张量类型：unsigned char
        //
        virtual sw::math::STensor& getData() = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    SIMPLEWORK_INTERFACECLASS_ENTER(AvFrameFactory, "sw.av.AvFrameFactory")
        SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvFrameFactory", 211214)
            //
            // 创建流对象
            //
            virtual SAvFrame createFrame(const PAvFrame& avFrame) = 0;

        SIMPLEWORK_INTERFACE_LEAVE
    SIMPLEWORK_INTERFACECLASS_LEAVE(AvFrameFactory)

SIMPLEWORK_INTERFACECLASS_LEAVE(AvFrame)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_IO_AvFrame_h__