#ifndef __SimpleWork_IO_AvFrame_h__
#define __SimpleWork_IO_AvFrame_h__

SIMPLEWORK_AV_NAMESPACE_ENTER 

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER0(AvFrame)
public:
    enum AvFrameImageType {
        AVFRAMEIMAGETYPE_RGB  = 1,  //RGB24，注意不是什么RGB888，  后者在不同平台或者有不同顺序定义
        AVFRAMEIMAGETYPE_RGBA  = 2, //RGB32，注意不是什么RGBA8888，后者在不同平台可能会有不同顺序定义
        //AVFRAMEIMAGETYPE_YUV420 = 3,
    };

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvFrame", 211206)

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
        virtual sw::math::Tensor getVideoImage(AvFrameImageType eType = AvFrame::AVFRAMEIMAGETYPE_RGB ) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(AvFrame)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_IO_AvFrame_h__