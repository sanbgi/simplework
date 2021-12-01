#ifndef __SimpleWork_CObject__h__
#define __SimpleWork_CObject__h__

__SimpleWork_Core_Namespace_Enter__

//
//  对象基类：负责实现IObject接口查询能力，所有对象都要从这个基类派生。具体提供如下能力：
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
//              SIMPLEWORK_INTERFACE_ENTRY(IFactory)
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
//      注意：这些类对象是无法直接创建的，只能通过CObject::createObject来创建(因为
//  存在IObject的纯虚函数定义)，也不建议通过实现这些纯虚函数，这样可以统一管理对象的创
//  建和销毁，避免内存泄漏，具体创建范例：
//      
//      TAutoPtr<IMyObject> spObj = CObject::createObject<CMyObject, IMyObject>();
//      TAutoPtr<IFactory> spFactory = CObject::createFactory<CMyObject, IFactory>()
//

//
//  对象基类：负责实现IObject接口查询能力，所有对象都要从这个基类派生。
//
class CObject : public IObject {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER0
        SIMPLEWORK_INTERFACE_ENTRY(IObject)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE0

//
// 下面是模板函数，用于实现对象或者工厂h的创建
//
public:
    //
    // 创建对象，并查询获取对象接口
    //
    template<typename TObject> static IObjectPtr createObject(bool bSingleton=false) {
        return CObjectImp<TObject>::createObjectImp(bSingleton);
    }

    //
    // 创建工厂
    //  @T 工厂用于创建的对象类
    //  @Q 工厂实现类
    //
    template<typename TObject> static IObjectPtr createFactory(bool bSingletonFactory=false) {
        if(bSingletonFactory) {
            return CObjectImp<CSingletonFactoryImp<TObject, CObject>>::createObjectImp();
        }else{
            return CObjectImp<CFactoryImp<TObject, CObject>>::createObjectImp();
        }
    }

private:
    template<typename TObject> class CObjectImp : public TObject {
    public:
        static IObjectPtr createObjectImp(bool bSingleton=false) {
            if(bSingleton) {
                static IObjectPtr g_spObject = createObjectImp(false);
                return g_spObject;
            }
            CObjectImp* pObjectImp = new CObjectImp();
            //
            //  CObject默认实现了__swGetIObject函数，但三种情况下，下面这一句无法成立
            //      1, 对象类不是从CObject派生
            //      2, 对象类是从CObject派生，但CObject基类不可访问，原因可能是：
            //          -- 基类是私有的并且对象类没有定义宏SIMPLEWORK_INTERFACE_ENTRY_ENTER
            //          -- 对象从多个CObject派生
            //  为了避免这个错误，系统要求对象的派生必须遵循规范为：
            //      1, 对象类只能从一个CObject基类或者符合规范的对象类派生；
            //      2，对象基类为private时，需要定义SIMPLEWORK_INTERFACE_ENTRY_ENTER
            //
            IObject* pObject = pObjectImp->__swGetIObject();
            if( pObject == nullptr ) {
                delete pObjectImp;
            }
            return pObject;
        }

    private:
        CObjectImp() : m_nRefCnt(0) {}
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

    template<typename TObject, typename TSuperClass> class CFactoryImp : public TSuperClass, public IFactory {
        SIMPLEWORK_INTERFACE_ENTRY_ENTER(TSuperClass)
            SIMPLEWORK_INTERFACE_ENTRY(IFactory)
        SIMPLEWORK_INTERFACE_ENTRY_LEAVE(TSuperClass)

    public://IFactory
        IObjectPtr createObject() {
            return CObjectImp<TObject>::createObjectImp(false);
        }
    };
    template<typename TObject, typename TSuperClass> class CSingletonFactoryImp : public TSuperClass, public IFactory {
        SIMPLEWORK_INTERFACE_ENTRY_ENTER(TSuperClass)
            SIMPLEWORK_INTERFACE_ENTRY(IFactory)
        SIMPLEWORK_INTERFACE_ENTRY_LEAVE(TSuperClass)

    public://IFactory
        IObjectPtr createObject() {
            return CObjectImp<TObject>::createObjectImp(true);
        }
    };
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_CObject__h__