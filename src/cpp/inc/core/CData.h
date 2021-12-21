#ifndef __SimpleWork_Core_CData_h__
#define __SimpleWork_Core_CData_h__

#include "core.h"
#include "PData.h"
#include "SData.h"

#define SIMPLEWORK_PDATAKEY(x) static const char* __getClassKey() { return x; }

__SimpleWork_Core_Namespace_Enter__

//
// 纯数据参数类，用于传递结构类型的函数参数，作为带类型的参数垫片
//
template<typename TType> class CData : public PData {
public:
    CData(const TType& rData){
        init(&rData);
    }
    CData(const TType* pData){
        init(pData);
    }
    CData(const PData& rData){
        init(rData);
    }

public:
    //
    // 判断当前数据是否是当前类型数据
    //
    //  当CData实例为自己创建的时候：这个时候肯定是有效的，无论数据是否为空，空数据也是有效数据
    //  当CData为接收的函数参数时：函数判断参数是否时当前类型的参数，这个判断与数据是否为空无关
    //
    bool isThisType() const {
        return getThisType() == m_idInternalType;
    }

    //
    // 判断数据是否为空
    //
    bool isEmpty() const {
        return m_pInternalData == nullptr;
    }

public:
    operator const TType*() {
        return (const TType*)m_pInternalData;
    }

public:

    //
    // 获取当前数据类型
    //
    static unsigned int getThisType() {
        static unsigned int s_idType = SData::getStructTypeIdentifier<TType>();
        return s_idType;
    }

public:
    const TType* getDataPtr() const {
        return (const TType*)m_pInternalData;
    }

    const TType& getData() const {
        return *(const TType*)m_pInternalData;
    }

private:
    void init(const TType* pData) {
        //
        // PData两个指针一个指向自己，用于：
        //      1, 判断当前指针是否是有效的CData指针;
        //      2，读取当前数据的类型;
        //
        m_pInternalPointer = this;
        m_idInternalType = getThisType();
        m_pInternalData = (void*)pData;
    }
    void init(const PData& rData) {
        if(rData.m_pInternalPointer == (void*)&rData ) {
            const CData* pSrc = (const CData*)&rData;
            m_idInternalType = pSrc->m_idInternalType;
            m_pInternalData = (getThisType() == m_idInternalType) ? pSrc->m_pInternalData : nullptr;
        }else{
            m_idInternalType = 0;
            m_pInternalData = nullptr;
        }
        m_pInternalPointer = this;
    }
};


template<typename TBasicType> class CBasicData : public PData {
public:
    CBasicData(const TBasicType& rData){
        init(&rData);
    }
    CBasicData(const TBasicType* pData){
        init(pData);
    }
    CBasicData(const PData& rData){
        init(rData);
    }

public:
    //
    // 判断当前数据是否是当前类型数据
    //
    //  当CData实例为自己创建的时候：这个时候肯定是有效的，无论数据是否为空，空数据也是有效数据
    //  当CData为接收的函数参数时：函数判断参数是否时当前类型的参数，这个判断与数据是否为空无关
    //
    bool isThisType() const {
        return getThisType() == m_idInternalType;
    }

    //
    // 判断数据是否为空
    //
    bool isEmpty() const {
        return m_pInternalData == nullptr;
    }

public:
    //
    // 获取当前数据类型
    //
    static unsigned int getThisType() {
        static unsigned int s_idType = SData::getBasicTypeIdentifier<TBasicType>();
        return s_idType;
    }

public:
    const TBasicType* getDataPtr() const {
        return (const TBasicType*)m_pInternalData;
    }

    const TBasicType& getData() const {
        return *(const TBasicType*)m_pInternalData;
    }

private:
    void init(const TBasicType* pData) {
        //
        // PData两个指针一个指向自己，用于：
        //      1, 判断当前指针是否是有效的CData指针;
        //      2，读取当前数据的类型;
        //
        m_pInternalPointer = this;
        m_idInternalType = getThisType();
        m_pInternalData = (void*)pData;
    }
    void init(const PData& rData) {
        if(rData.m_pInternalPointer == (void*)&rData ) {
            const CBasicData* pSrc = (const CBasicData*)&rData;
            m_idInternalType = pSrc->m_idInternalType;
            m_pInternalData = (getThisType() == m_idInternalType) ? pSrc->m_pInternalData : nullptr;
        }else{
            m_idInternalType = 0;
            m_pInternalData = nullptr;
        }
        m_pInternalPointer = this;
    }
};
__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_CData_h__