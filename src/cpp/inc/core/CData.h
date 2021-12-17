#ifndef __SimpleWork_Core_CData_h__
#define __SimpleWork_Core_CData_h__

#include "core.h"
#include <typeinfo>

__SimpleWork_Core_Namespace_Enter__

//
// 纯数据参数类，用于传递函数参数，作为带类型的参数垫片
//
// 范例：
//
//  void fun(const CData& data) {
//     if( data.hasData<int>() ) {
//        std::cout << "i get the integer." << data.getData<int>();
//     }
//     if(data.hasData<double>() ) {
//         std::cout << "i get the double." << data.getData<double>();
//     }
//   }
//
//   int main(int argc, char *argv[]){
//      int i=10;
//      CData s(i);
//      fun(s);
//      fun(CData(i));
//      fun(CData(20));
//      const int* pPtr = s.getPtr<int>();
//
//      return 0;
//    }
//
struct CData {

public:
    typedef size_t CType;

    template<typename TData> CData(const TData* pData) {
        m_iType = getType<TData>();
        m_pData = (void*)pData;
    }
    template<typename TData> CData(const TData& rData) {
        m_iType = getType<TData>();
        m_pData = (void*)&rData;
    }
    template<typename TData> const TData* getPtr() const {
        if( getType<TData>() != m_iType) {
            return nullptr;
        }
        return (TData*)m_pData;
    }
    template<typename TData> const TData& getData() const {
        if( getType<TData>() != m_iType) {
            return *(TData*)nullptr;
        }
        return *(TData*)m_pData;
    }
    template<typename TData> bool hasData() const {
        if( getType<TData>() != m_iType) {
            return false;
        }
        return true;
    }
    template<typename TData> static CType getType() {
        return typeid(TData).hash_code();
    }
    CType getType() {
        return m_iType;
    }

private:
    //
    // 禁止缺省构造和拷贝赋值
    //
    CData(){}
    const CData& operator = (const CData& src){ return *this; }

private:
    //
    //  目前先用C++的type_info来进行判断（跨编译器不安全），后续切换为系统的更可靠的类型判断
    //
    CType m_iType;
    void* m_pData;

};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_CData_h__