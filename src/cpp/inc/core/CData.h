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
template<typename TStructType> class CStructData : public PData {
public:
    CStructData(const TStructType& rData){
        init(&rData);
    }
    CStructData(const TStructType* pData){
        init(pData);
    }
    CStructData(const PData& rData){
        init(rData);
    }

public:
    //
    // 判断当前数据是否是当前类型数据
    //
    //  当CStructData实例为自己创建的时候：这个时候肯定是有效的，无论数据是否为空，空数据也是有效数据
    //  当CStructData为接收的函数参数时：函数判断参数是否时当前类型的参数，这个判断与数据是否为空无关
    //
    bool isThisType() const {
        return getType() == m_idType;
    }

    //
    // 判断数据是否为空
    //
    bool isEmpty() const {
        return m_pData == nullptr;
    }

public:
    //
    // 获取当前数据类型
    //
    static SData::tid getType() {
        static SData::tid s_idType = SData::getStructTypeIdentifier<TStructType>();
        return s_idType;
    }

public:
    const TStructType* getDataPtr() const {
        return m_pData;
    }

    const TStructType& getData() const {
        return m_pData;
    }

private:
    void init(const TStructType* pData) {
        //
        // PData两个指针一个指向自己，用于：
        //      1, 判断当前指针是否是有效的CStructData指针;
        //      2，读取当前数据的类型;
        //
        __pInternalPointer = this;
        m_idType = getType();
        m_pData = pData;
    }
    void init(const PData& rData) {
        if(rData.__pInternalPointer == (void*)&rData ) {
            const CStructData* pSrc = (const CStructData*)&rData;
            m_idType = pSrc->m_idType;
            m_pData = getType() == m_idType ? pSrc->m_pData : nullptr;
        }else{
            m_idType = 0;
            m_pData = nullptr;
        }
    }

private:
    SData::tid m_idType;
    const TStructType* m_pData; 
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
    //  当CStructData实例为自己创建的时候：这个时候肯定是有效的，无论数据是否为空，空数据也是有效数据
    //  当CStructData为接收的函数参数时：函数判断参数是否时当前类型的参数，这个判断与数据是否为空无关
    //
    bool isThisType() const {
        return getType() == m_idType;
    }

    //
    // 判断数据是否为空
    //
    bool isEmpty() const {
        return m_pData == nullptr;
    }

public:
    //
    // 获取当前数据类型
    //
    static SData::tid getType() {
        static SData::tid s_idType = SData::getBasicTypeIdentifier<TBasicType>();
        return s_idType;
    }

public:
    const TBasicType* getDataPtr() const {
        return m_pData;
    }

    const TBasicType& getData() const {
        return m_pData;
    }

private:
    void init(const TBasicType* pData) {
        //
        // PData两个指针一个指向自己，用于：
        //      1, 判断当前指针是否是有效的CStructData指针;
        //      2，读取当前数据的类型;
        //
        __pInternalPointer = this;
        m_idType = getType();
        m_pData = pData;
    }
    void init(const PData& rData) {
        if(rData.__pInternalPointer == (void*)&rData ) {
            const CBasicData* pSrc = (const CBasicData*)&rData;
            m_idType = pSrc->m_idType;
            m_pData = getType() == m_idType ? pSrc->m_pData : nullptr;
        }else{
            m_idType = 0;
            m_pData = nullptr;
        }
    }

private:
    SData::tid m_idType;
    const TBasicType* m_pData; 
};
__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_CData_h__