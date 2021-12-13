#ifndef __SimpleWork_IO_AvFrame_h__
#define __SimpleWork_IO_AvFrame_h__

#include "av.h"

SIMPLEWORK_AV_NAMESPACE_ENTER 

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER0(AvFrame)
public:
    enum AvFrameType {
        AVSTREAMTYPE_UNKNOWN = 0,
        AVSTREAMTYPE_VIDEO   = 1,
        AVSTREAMTYPE_AUDIO   = 2,
        AVSTREAMTYPE_SUBTITLE= 3
    };

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvFrame", 211206)
        //
        // 获取帧数据类型
        //
        virtual AvFrameType getFrameType() = 0;

        //
        // 获取流编号
        //
        virtual int getStreamingId() = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(AvFrame)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_IO_AvFrame_h__