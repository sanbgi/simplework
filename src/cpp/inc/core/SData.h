#ifndef __SimpleWork_BasicType_h__
#define __SimpleWork_BasicType_h__

#include <typeinfo>
#include "core.h"

__SimpleWork_Core_Namespace_Enter__

class SObject;

//
// 数据类，目前还不成熟，还需要思考
//
SIMPLEWORK_INTERFACECLASS_ENTER0(Data)
public:
    typedef unsigned int tid;

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.core.IData", 211219)
    SIMPLEWORK_INTERFACE_LEAVE


    SIMPLEWORK_INTERFACECLASS_ENTER(DataFactory, "sw.core.DataFactory")
        SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IDataFactory", 211206)
        
            //获取指定名字数据类型对应的数据类型ID
            virtual tid getTypeIdentifier(const char* szDataTypeKey) = 0;

        SIMPLEWORK_INTERFACE_LEAVE
    SIMPLEWORK_INTERFACECLASS_LEAVE(DataFactory)

    template<typename TStructType> static tid getStructTypeIdentifier(){
        static tid s_tid = getFactory()->getTypeIdentifier(TStructType::__getClassKey());
        return s_tid;
    }
    template<typename TBasicType> static tid getBasicTypeIdentifier(){
        static tid s_tid = getFactory()->getTypeIdentifier(getBasicTypeKey<TBasicType>());
        return s_tid;;
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

    static SDataFactory& getFactory() {
        static SDataFactory g_factory = SObject::createObject<SDataFactory>();
        return g_factory;
    }
SIMPLEWORK_INTERFACECLASS_LEAVE(Data)

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_BasicType_h__