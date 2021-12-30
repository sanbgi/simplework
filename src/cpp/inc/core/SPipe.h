#ifndef __SimpleWork_Core_SPipe_h__
#define __SimpleWork_Core_SPipe_h__


#include "core.h"
#include "PData.h"
#include "IVisitor.h"

__SimpleWork_Core_Namespace_Enter__

//
//
//  管道，推入管道数据，经过管道处理后，返回结果数据，适用于立即处理并且立即获取返回结果的管道模式
//
//
SIMPLEWORK_INTERFACECLASS_ENTER0(Pipe)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.core.IPipe", 011220)

        //
        // 推入数据，并且通过pReceiver获得返回结果
        //
        virtual int pushData(const PData& rData, IVisitor<const PData&>* pReceiver) = 0; 

    SIMPLEWORK_INTERFACE_LEAVE
    
SIMPLEWORK_INTERFACECLASS_LEAVE(Pipe)

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_SPipe_h__