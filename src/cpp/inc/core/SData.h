#ifndef __SimpleWork_BasicType_h__
#define __SimpleWork_BasicType_h__

#include <typeinfo>
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

    template<typename TType> static unsigned int getStructTypeIdentifier(){
        static unsigned int s_tid = SCoreFactory::getFactory()->getTypeIdentifier(TType::__getClassKey());
        return s_tid;
    }
    template<typename TBasicType> static unsigned int getBasicTypeIdentifier(){
        static unsigned int s_tid = SCoreFactory::getFactory()->getTypeIdentifier(getBasicTypeKey<TBasicType>());
        return s_tid;;
    }

    template<typename Q> const Q* getDataPtr() {
        if( !(*this) ) {
            return nullptr;
        }
        return (const Q*)(*this)->getDataPtr(getStructTypeIdentifier<Q>());
    }
private:
    template<typename T> static const char* getBasicTypeKey() {
        if(typeid(T) == typeid(bool)) {
            return "sw.core.Bool";
        } else if(typeid(T) == typeid(char)) {
            return "sw.core.Char";
        } else if(typeid(T) == typeid(unsigned char)) {
            return "sw.core.UChar";
        } else if(typeid(T) == typeid(int)) {
            return "sw.core.Int";
        } else if(typeid(T) == typeid(short)) {
            return "sw.core.Short";
        } else if(typeid(T) == typeid(long)) {
            return "sw.core.Long";
        } else if(typeid(T) == typeid(float)) {
            return "sw.core.Float";
        } else if(typeid(T) == typeid(double)) {
            return "sw.core.Double";
        }
        return nullptr;
    }


SIMPLEWORK_INTERFACECLASS_LEAVE(Data)

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_BasicType_h__