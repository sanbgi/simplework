#ifndef __SimpleWork_Core_CData_h__
#define __SimpleWork_Core_CData_h__

#include <typeinfo>
#include "core.h"
#include "PArg.h"

__SimpleWork_Core_Namespace_Enter__

#define SIMPLEWORK_PDATAKEY(TValue, name) \
    typedef TValue __TValue; \
    static const char* __getClassKey() { return name; }

//
// 纯数据参数类，用于传递结构类型的函数参数，作为带类型的参数垫片
//
template<typename TType> class CArg : public PArg {
public:
    typedef typename TType::__TValue TValue;

    //
    // 获取当前数据类型
    //
    static PDATATYPE getStaticType() {
        static PDATATYPE s_idType = SCore::getFactory()->getTypeIdentifier(TType::__getClassKey());
        return s_idType;
    }

public:
    CArg(const TValue& rData){
        init(&rData);
    }
    CArg(const TValue* pData){
        init(pData);
    }
    CArg(const PArg& rData){
        init(rData);
    }
    CArg(const PArg* pData){
        if(pData == nullptr) {
            m_pInternalPointer = this;
            m_idInternalType = 0;
            m_pInternalData = nullptr;
        }else{
            init(*pData);
        }
    }

public:
    //
    // 判断当前数据是否是当前类型数据
    //
    //  当CArg实例为自己创建的时候：这个时候肯定是有效的，无论数据是否为空，空数据也是有效数据
    //  当CArg为接收的函数参数时：函数判断参数是否时当前类型的参数，这个判断与数据是否为空无关
    //
    bool isThisType() const {
        return getStaticType() == m_idInternalType;
    }

    //
    // 判断数据是否为空
    //
    bool isEmpty() const {
        return m_pInternalData == nullptr;
    }

public:
    operator const TValue*() {
        return (const TValue*)m_pInternalData;
    }

    operator const PArg*() {
        return this;
    }

public:
    const TValue* getDataPtr() const {
        return (const TValue*)m_pInternalData;
    }

    const TValue& getData() const {
        return *(const TValue*)m_pInternalData;
    }

private:
    void init(const TValue* pData) {
        //
        // PArg两个指针一个指向自己，用于：
        //      1, 判断当前指针是否是有效的CArg指针;
        //      2，读取当前数据的类型;
        //
        m_pInternalPointer = this;
        m_idInternalType = getStaticType();
        m_pInternalData = (void*)pData;
    }
    void init(const PArg& rData) {
        if(rData.m_pInternalPointer == (void*)&rData ) {
            const CArg* pSrc = (const CArg*)&rData;
            m_idInternalType = pSrc->m_idInternalType;
            m_pInternalData = (getStaticType() == m_idInternalType) ? pSrc->m_pInternalData : nullptr;
        }else{
            m_idInternalType = 0;
            m_pInternalData = nullptr;
        }
        m_pInternalPointer = this;
    }
};

template<typename TValue> class CBasicData : public PArg{
public:
    static const char* __getClassKey() {
        static const char* s_key = getBasicTypeKey();
        return s_key;
    }
    typedef TValue __TValue;

    static PDATATYPE getStaticType() {
        static PDATATYPE s_idType = SCore::getFactory()->getTypeIdentifier(__getClassKey());
        return s_idType;
    }

private:
    static const char* getBasicTypeKey() {
        if(typeid(TValue) == typeid(bool)) {
            return "sw.core.Bool";
        } else if(typeid(TValue) == typeid(char)) {
            return "sw.core.Char";
        } else if(typeid(TValue) == typeid(unsigned char)) {
            return "sw.core.UChar";
        } else if(typeid(TValue) == typeid(int)) {
            return "sw.core.Int";
        } else if(typeid(TValue) == typeid(PDATATYPE)) {
            return "sw.core.UInt";
        } else if(typeid(TValue) == typeid(short)) {
            return "sw.core.Short";
        } else if(typeid(TValue) == typeid(long)) {
            return "sw.core.Long";
        } else if(typeid(TValue) == typeid(float)) {
            return "sw.core.Float";
        } else if(typeid(TValue) == typeid(double)) {
            return "sw.core.Double";
        } else if(typeid(TValue) == typeid(char*)) {
            return "sw.core.String";
        }
        return nullptr;
    }

public:
    CBasicData(const TValue& rData){
        init(&rData);
    }
    CBasicData(const TValue* pData){
        init(pData);
    }
    CBasicData(const PArg& rData){
        init(rData);
    }
    CBasicData(const PArg* pData){
        if(pData == nullptr) {
            m_pInternalPointer = this;
            m_idInternalType = 0;
            m_pInternalData = nullptr;
        }else{
            init(*pData);
        }
    }

public:
    //
    // 判断当前数据是否是当前类型数据
    //
    //  当CArg实例为自己创建的时候：这个时候肯定是有效的，无论数据是否为空，空数据也是有效数据
    //  当CArg为接收的函数参数时：函数判断参数是否时当前类型的参数，这个判断与数据是否为空无关
    //
    bool isThisType() const {
        return getStaticType() == m_idInternalType;
    }

    //
    // 判断数据是否为空
    //
    bool isEmpty() const {
        return m_pInternalData == nullptr;
    }

public:
    operator const TValue*() {
        return (const TValue*)m_pInternalData;
    }

public:
    const TValue* getDataPtr() const {
        return (const TValue*)m_pInternalData;
    }

    const TValue& getData() const {
        return *(const TValue*)m_pInternalData;
    }

private:
    void init(const TValue* pData) {
        //
        // PArg两个指针一个指向自己，用于：
        //      1, 判断当前指针是否是有效的CArg指针;
        //      2，读取当前数据的类型;
        //
        m_pInternalPointer = this;
        m_idInternalType = getStaticType();
        m_pInternalData = (void*)pData;
    }
    void init(const PArg& rData) {
        if(rData.m_pInternalPointer == (void*)&rData ) {
            const CBasicData* pSrc = (const CBasicData*)&rData;
            m_idInternalType = pSrc->m_idInternalType;
            m_pInternalData = (getStaticType() == m_idInternalType) ? pSrc->m_pInternalData : nullptr;
        }else{
            m_idInternalType = 0;
            m_pInternalData = nullptr;
        }
        m_pInternalPointer = this;
    }
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_CData_h__