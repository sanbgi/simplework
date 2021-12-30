#ifndef __SimpleWork_BasicType_h__
#define __SimpleWork_BasicType_h__

#include "core.h"
#include "PData.h"
#include "SCoreFactory.h"

__SimpleWork_Core_Namespace_Enter__

class SObject;

//
// 数据类，目前还不成熟，还需要思考
//
SIMPLEWORK_INTERFACECLASS_ENTER0(Data)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.core.IData", 211219)
        //
        //  获取数据指针
        //
        virtual const void* getDataPtr(unsigned int idType) = 0;

        //
        //  获取数据类型
        //
        virtual unsigned int getDataType() = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    template<typename TType> static unsigned int getTypeIdentifier(){
        static unsigned int s_tid = SCoreFactory::getFactory()->getTypeIdentifier(TType::__getClassKey());
        return s_tid;
    }

    template<typename Q> const Q* getDataPtr() {
        if( !(*this) ) {
            return nullptr;
        }
        return (const Q*)(*this)->getDataPtr(getTypeIdentifier<Q>());
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(Data)

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_BasicType_h__