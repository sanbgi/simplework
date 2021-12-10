#ifndef __SimpleWork_IO_AvFrame_h__
#define __SimpleWork_IO_AvFrame_h__

SIMPLEWORK_AV_NAMESPACE_ENTER 

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER0(AvFrame)
public:
    enum AvFrameType {
        AVFRAMETYPE_UNKNOWN = 0,
        AVFRAMETYPE_VIDEO   = 1,
        AVFRAMETYPE_AUDIO   = 2,
        AVFRAMETYPE_SUBTITLE= 3
    };

    enum AvFrameImageType {
        AVFRAMEIMAGETYPE_RGB24  = 1,
        AVFRAMEIMAGETYPE_RGB32  = 2,
        AVFRAMEIMAGETYPE_YUV420 = 3,
    };

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.io.IAvFrame", 211206)

        //
        // 获取帧数据类型
        //
        virtual AvStreaming::AvStreamingType getStreamingType() = 0;

        //
        // 获取帧对应的流
        //
        virtual AvStreaming& getStreaming() = 0;

        //
        // 获取帧视频的图像张量
        //
        virtual sw::math::Tensor getVideoImage(AvFrameImageType eImageType) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(AvFrame)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_IO_AvFrame_h__