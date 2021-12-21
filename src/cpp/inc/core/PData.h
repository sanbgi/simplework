#ifndef __SimpleWork_PData_h__
#define __SimpleWork_PData_h__

#include "core.h"

__SimpleWork_Core_Namespace_Enter__

//
// 通用参数数据
//  
//  作为参数传递的通用数据定义，仅用于传递函数参数，不允许直接使用，只能
//  通过模板类CData来使用，具体使用说明，请参考CData
//
struct PData {

protected:
    void* m_pInternalPointer;
    void* m_pInternalData;
    unsigned int m_idInternalType;

public:
    unsigned int getType() const {
        if(m_pInternalPointer == (void*)this ) {
            return m_idInternalType;
        }
        return 0;
    }

protected:
    //
    // 不允许直接构造
    //
    PData(){}

private:
    //
    // 不允许拷贝赋值和构造等等
    //
    PData(const PData& src) {}
    const PData& operator = (const PData& src) { return *this; }
    template<typename TType> friend class CData;
    template<typename TBasicType> friend class CBasicData;
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_PData_h__