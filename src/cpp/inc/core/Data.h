#ifndef __SimpleWork_BasicType_h__
#define __SimpleWork_BasicType_h__

#include <typeinfo>

__SimpleWork_Core_Namespace_Enter__

class Object;

//
// 基础类型定义
//
struct Data {
    enum DataType{
        DATATYPE_UNKNOWN  = 0,
        DATATYPE_BOOL     = 101,
        DATATYPE_CHAR     = 102,
        DATATYPE_SHORT    = 103,
        DATATYPE_INT      = 104,
        DATATYPE_LONG     = 105,
        DATATYPE_FLOAT    = 201,
        DATATYPE_DOUBLE   = 202,
        DATATYPE_OBJECT   = 301,
    };

    template<typename T> static DataType getType() {
        static DataType s_eRawType = getRawType<T>();
        return s_eRawType;
    };

private:
    template<typename T> static DataType getRawType() {
        if(typeid(T) == typeid(bool)) {
            return Data::DATATYPE_BOOL;
        } else if(typeid(T) == typeid(char)) {
            return Data::DATATYPE_CHAR;
        } else if(typeid(T) == typeid(int)) {
            return Data::DATATYPE_INT;
        } else if(typeid(T) == typeid(short)) {
            return Data::DATATYPE_SHORT;
        } else if(typeid(T) == typeid(long)) {
            return Data::DATATYPE_LONG;
        } else if(typeid(T) == typeid(float)) {
            return Data::DATATYPE_FLOAT;
        } else if(typeid(T) == typeid(double)) {
            return Data::DATATYPE_DOUBLE;
        }else if(typeid(T) == typeid(Object)) {
            return Data::DATATYPE_OBJECT;
        }
        return Data::DATATYPE_UNKNOWN;
    };
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_BasicType_h__