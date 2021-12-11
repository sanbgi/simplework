#ifndef __SimpleWork_IO_AvStreaming_h__
#define __SimpleWork_IO_AvStreaming_h__

SIMPLEWORK_AV_NAMESPACE_ENTER

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER0(AvStreaming)
public:
    enum AvStreamingType {
        AVSTREAMTYPE_UNKNOWN = 0,
        AVSTREAMTYPE_VIDEO   = 1,
        AVSTREAMTYPE_AUDIO   = 2,
        AVSTREAMTYPE_SUBTITLE= 3
    };

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvStreaming", 211206)

        //
        // 获取流数据类型
        //
        virtual AvStreamingType getStreamingType() = 0;

        //
        //  获取流的序号
        //
        virtual int getStreamingIndex() = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(AvStreaming)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_IO_AvStreaming_h__