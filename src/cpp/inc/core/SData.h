#ifndef __SimpleWork_BasicType_h__
#define __SimpleWork_BasicType_h__

#include <typeinfo>
#include "core.h"

__SimpleWork_Core_Namespace_Enter__

class SObject;

//
// 基础类型定义
//
struct SData {
    enum DataType{
        DATATYPE_UNKNOWN  = 0,
        DATATYPE_BOOL     = 101,
        DATATYPE_CHAR     = 102,
        DATATYPE_UCHAR    = 103,
        DATATYPE_SHORT    = 104,
        DATATYPE_INT      = 106,
        DATATYPE_LONG     = 108,
        DATATYPE_FLOAT    = 201,
        DATATYPE_DOUBLE   = 202,
        DATATYPE_OBJECT   = 301,
    };

    template<typename T> static DataType getType() {
        static DataType s_eRawType = getRawType<T>();
        return s_eRawType;
    };
    static bool isPuryMemoryType(DataType eDt) {
        switch(eDt) {
            case DATATYPE_BOOL:
            case DATATYPE_CHAR:
            case DATATYPE_UCHAR:
            case DATATYPE_SHORT:
            case DATATYPE_INT:
            case DATATYPE_LONG:
            case DATATYPE_FLOAT:
            case DATATYPE_DOUBLE:
                return true;
        }
        return false;
    }

private:
    template<typename T> static DataType getRawType() {
        if(typeid(T) == typeid(bool)) {
            return SData::DATATYPE_BOOL;
        } else if(typeid(T) == typeid(char)) {
            return SData::DATATYPE_CHAR;        
        } else if(typeid(T) == typeid(unsigned char)) {
            return SData::DATATYPE_UCHAR;
        } else if(typeid(T) == typeid(int)) {
            return SData::DATATYPE_INT;
        } else if(typeid(T) == typeid(short)) {
            return SData::DATATYPE_SHORT;
        } else if(typeid(T) == typeid(long)) {
            return SData::DATATYPE_LONG;
        } else if(typeid(T) == typeid(float)) {
            return SData::DATATYPE_FLOAT;
        } else if(typeid(T) == typeid(double)) {
            return SData::DATATYPE_DOUBLE;
        }else if(typeid(T) == typeid(SObject)) {
            return SData::DATATYPE_OBJECT;
        }
        return SData::DATATYPE_UNKNOWN;
    };
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_BasicType_h__