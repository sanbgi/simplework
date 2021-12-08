#ifndef __SimpleWork_BasicType_h__
#define __SimpleWork_BasicType_h__

#include <typeinfo>

__SimpleWork_Core_Namespace_Enter__

struct Object;

//
// 基础类型定义
//
struct BasicType {
    enum EType{
        UNKNOWN  = 0,
        BOOL     = 101,
        CHAR     = 102,
        SHORT    = 103,
        INT      = 104,
        LONG     = 105,
        FLOAT    = 201,
        DOUBLE   = 202,
        OBJECT   = 301,
    };

    template<typename T> static EType getType() {
        static EType s_eRawType = getRawType<T>();
        return s_eRawType;
    };

private:
    template<typename T> static EType getRawType() {
        if(typeid(T) == typeid(bool)) {
            return BasicType::BOOL;
        } else if(typeid(T) == typeid(char)) {
            return BasicType::CHAR;
        } else if(typeid(T) == typeid(int)) {
            return BasicType::INT;
        } else if(typeid(T) == typeid(short)) {
            return BasicType::SHORT;
        } else if(typeid(T) == typeid(long)) {
            return BasicType::LONG;
        } else if(typeid(T) == typeid(float)) {
            return BasicType::FLOAT;
        } else if(typeid(T) == typeid(double)) {
            return BasicType::DOUBLE;
        }else if(typeid(T) == typeid(Object)) {
            return BasicType::OBJECT;
        }
        return BasicType::UNKNOWN;
    };
};
typedef BasicType::EType EBasicType;

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_BasicType_h__