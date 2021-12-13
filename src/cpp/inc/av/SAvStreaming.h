#ifndef __SimpleWork_IO_AvStreaming_h__
#define __SimpleWork_IO_AvStreaming_h__

#include "av.h"

SIMPLEWORK_AV_NAMESPACE_ENTER

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER0(AvStreaming)
public:

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvStreaming", 211206)

        //
        // 获取流数据类型
        //
        virtual SAvFrame::AvFrameType getFrameType() = 0;

        //
        //  获取流的序号
        //
        virtual int getStreamingId() = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(AvStreaming)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_IO_AvStreaming_h__