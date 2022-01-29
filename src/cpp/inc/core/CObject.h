#ifndef __SimpleWork_CObject__h__
#define __SimpleWork_CObject__h__

#include "core.h"
#include "SObject.h"
#include "CPointer.h"

__SimpleWork_Core_Namespace_Enter__

//
//  对象基类：建议对象都要从这个基类派生。具体提供如下能力：
//
//      1，提供对象创建能力，参照：CObject::createObject<TObject>
//      2, 所有对象基类
//
//  其中，符合要求具体包括：
//
//      1, 对象类必须从CObject派生，或者从同样符合要求的对象派生；
//      2，如果对象类实现了自定义的接口，则需要在类定义内部使用如下三个宏来定义所
//  实现的接口，否则（无法查询到对象接口）
//          
//          SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
//              SIMPLEWORK_INTERFACE_ENTRY(Facctory)
//          SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)
//      
//      范例一：
//      
//      class CMyObject : CObject : IMyObject {
//          SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
//              SIMPLEWORK_INTERFACE_ENTRY(IMyObject)
//          SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)
//      private:
//          ...
//      }
//
//      class CMyObject2 : CMyObject : IMyObject2 {
//          SIMPLEWORK_INTERFACE_ENTRY_ENTER(CMyObject)
//              SIMPLEWORK_INTERFACE_ENTRY(IMyObject2)
//          SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CMyObject)
//      private:
//          ...
//      }
//
//      注意：这些类对象是强烈不建议直接创建，而是通过CObject::createObject来创建，
//  这样可以统一管理对象的创建和销毁，避免内存泄漏，具体创建范例：
//  
//      SObject obj = CObject::createObject<CMyObject>();
//
class CObject {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER0
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE0

public:
    //
    // 创建对象
    //
    template<typename TObject> static int createObject(CPointer<TObject>& spObject, const PData* pData=nullptr) {
        return __CObjectImp<TObject>::__createObject(spObject, pData);
    }
    template<typename TObject> static SObject createObject(const PData* pData=nullptr) {
        CPointer<TObject> spObject;
        __CObjectImp<TObject>::__createObject(spObject, pData);
        return spObject;
    }

private:
    struct __IObjectImp : public IObject {};
    template<typename TObject> class __CObjectImp : public TObject, public __IObjectImp {
        SIMPLEWORK_INTERFACE_ENTRY_ENTER(TObject)
        SIMPLEWORK_INTERFACE_ENTRY_LEAVE(TObject)

    public:
        static int __createObject(CPointer<TObject>& spPointer, const PData* pData=nullptr) {
            __CObjectImp* pCreateObj = new __CObjectImp();
            if( int retCode = pCreateObj->__initialize(pData) != SError::ERRORTYPE_SUCCESS ) {
                return retCode;
            }
            
            spPointer.take(pCreateObj,(__IObjectImp*)pCreateObj);
            return SError::ERRORTYPE_SUCCESS;
        }

    private:
        __CObjectImp() : m_nRefCnt(0) {}
        int __swAddRef() { 
            return ++m_nRefCnt; 
        }
        int __swDecRef() { 
            int nRefCnt = --m_nRefCnt;
            if( nRefCnt == 0 ) {
                 delete this; 
            }
            return nRefCnt;
        }
        int m_nRefCnt;
    };
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_CObject__h__