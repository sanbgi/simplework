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
//      2，提供创建对象的工厂创建能力，参照：CObject::createFactory<TObject>
//      3, 所有对象基类
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
//      SFactory fac = CObject::createFactory<CMyObject>()
//
class CObject {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER0
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE0

public:
    //
    // 创建对象
    //
    template<typename TObject> static SObject createObject() {
        CPointer<TObject> spObject;
        __CObjectImp<TObject>::__createObject(spObject);
        return spObject;
    }
    template<typename TObject> static int createObject(CPointer<TObject>& spObject) {
        return __CObjectImp<TObject>::__createObject(spObject);
    }
    

    //
    // 创建工厂
    //
    template<typename TObject> static int createFactory(SObject& rFactory, bool bSingletonFactory=false) {
        
        class __CFactoryImp : public IFactory {
            SIMPLEWORK_INTERFACE_ENTRY_ENTER0
                SIMPLEWORK_INTERFACE_ENTRY(IFactory)
            SIMPLEWORK_INTERFACE_ENTRY_LEAVE0

        public://IFactory
            int createObject(SObject& rObject) const {
                CPointer<TObject> spObject;
                __CObjectImp<TObject>::__createObject(spObject);
                rObject = spObject.getObject();
                return SError::ERRORTYPE_SUCCESS;
            }
        };

        class __CSingletonFactoryImp : public IFactory {
        SIMPLEWORK_INTERFACE_ENTRY_ENTER0
            SIMPLEWORK_INTERFACE_ENTRY(IFactory)
        SIMPLEWORK_INTERFACE_ENTRY_LEAVE0

        public://IFactory
            int createObject(SObject& rObject) const {
                static CPointer<TObject> g_spObject;
                static int r = __CObjectImp<TObject>::__createObject(g_spObject);
                rObject = g_spObject.getObject();
                return SError::ERRORTYPE_SUCCESS;
            }
        };

        if(bSingletonFactory) {
            CPointer<__CSingletonFactoryImp> spFactory;
            __CObjectImp<__CSingletonFactoryImp>::__createObject(spFactory);
            rFactory = spFactory.getObject();
        }else{
            CPointer<__CFactoryImp> spFactory;
            __CObjectImp<__CFactoryImp>::__createObject(spFactory);
            rFactory = spFactory.getObject();
        }
        return SError::ERRORTYPE_SUCCESS;
    }
    template<typename TObject> static SObject createFactory(bool bSingleton=false) {
        SObject spFactory;
        createFactory<TObject>(spFactory, bSingleton);
        return spFactory;
    }

private:
    struct __IObjectImp : public IObject {};
    template<typename TObject> class __CObjectImp : public TObject, public __IObjectImp {
        SIMPLEWORK_INTERFACE_ENTRY_ENTER(TObject)
        SIMPLEWORK_INTERFACE_ENTRY_LEAVE(TObject)

    public:
        static int __createObject(CPointer<TObject>& spPointer) {
            __CObjectImp* pNewObj = new __CObjectImp();
            spPointer.take(pNewObj,(__IObjectImp*)pNewObj);
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