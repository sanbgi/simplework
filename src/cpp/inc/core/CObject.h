#ifndef __SimpleWork_CObject__h__
#define __SimpleWork_CObject__h__

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
//      class MyObject2 : CMyObject : IMyObject2 {
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
//      Object obj = CObject::createObject<CMyObject>();
//      Factory fac = CObject::createFactory<CMyObject>()
//
class CObject {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER0
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE0

public:
    //
    // 带对象指针的数据结构，用于创建对象返回的数据可以带上对象指针
    //
    template<typename TObject> struct ObjectWithPtr {
        TObject* pObject;
        Object spObject;
    };

    //
    // 创建对象
    //
    template<typename TObject> static Object createObject(bool bSingleton=false) {
        return __CObjectImp<TObject>::createObjectWithPtr(bSingleton).spObject;
    }
    template<typename TObject, typename TPointer=TObject> static ObjectWithPtr<TPointer> createObjectWithPtr(bool bSingleton=false) {
        return __CObjectImp<TObject, TPointer>::createObjectWithPtr(bSingleton);
    }

    //
    // 创建工厂
    //
    template<typename TObject> static Object createFactory(bool bSingletonFactory=false) {
        if(bSingletonFactory) {
            return __CObjectImp<__CSingletonFactoryImp<TObject, CObject>>::createObjectWithPtr().spObject;
        }else{
            return __CObjectImp<__CFactoryImp<TObject, CObject>>::createObjectWithPtr().spObject;
        }
    }

private:
    struct __IObjectImp : public IObject {};
    template<typename TObject, typename TPointer=TObject> class __CObjectImp : public TObject, public __IObjectImp {
        SIMPLEWORK_INTERFACE_ENTRY_ENTER(TObject)
        SIMPLEWORK_INTERFACE_ENTRY_LEAVE(TObject)

    public:
        static ObjectWithPtr<TPointer> createObjectWithPtr(bool bSingleton=false) {
            if(bSingleton) {
                static ObjectWithPtr<TPointer> g_spObject = createObjectWithPtr(false);
                return g_spObject;
            }

            __CObjectImp* pNewObj = new __CObjectImp();
            ObjectWithPtr<TPointer> obj;
            obj.pObject = pNewObj;
            obj.spObject.setPtr((__IObjectImp*)pNewObj);
            return obj;
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

    template<typename TObject, typename TSuperClass> class __CFactoryImp : public TSuperClass, public IFactory {
        SIMPLEWORK_INTERFACE_ENTRY_ENTER(TSuperClass)
            SIMPLEWORK_INTERFACE_ENTRY(IFactory)
        SIMPLEWORK_INTERFACE_ENTRY_LEAVE(TSuperClass)

    public://IFactory
        Object createObject() {
            return __CObjectImp<TObject>::createObjectWithPtr(false).spObject;
        }
    };
    template<typename TObject, typename TSuperClass> class __CSingletonFactoryImp : public TSuperClass, public IFactory {
        SIMPLEWORK_INTERFACE_ENTRY_ENTER(TSuperClass)
            SIMPLEWORK_INTERFACE_ENTRY(IFactory)
        SIMPLEWORK_INTERFACE_ENTRY_LEAVE(TSuperClass)

    public://IFactory
        Object createObject() {
            return __CObjectImp<TObject>::createObjectWithPtr(true).spObject;
        }
    };
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_CObject__h__