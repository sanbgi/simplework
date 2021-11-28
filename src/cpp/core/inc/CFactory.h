#ifndef __SimpleWork_CFactory__h__
#define __SimpleWork_CFactory__h__

__SimpleWork_Core_Namespace_Enter__

//
// 系统工厂：主要负责为符合要求的对象(具体要求参照后续说明)，提供如下能力：
//
//      1，提供对象创建能力，参照：CFactory::createObject<TObject>
//      2，提供创建对象的工厂创建能力，参照：CFactory::createFactory<TObject>
//
// 其中，符合要求具体包括：
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

//      注意：这些类对象是无法直接创建的，只能通过CFactory::createObject来创建(因为
//  存在IObject的纯虚函数定义)，也不建议通过实现这些纯虚函数，这样可以统一管理对象的创
//  建和销毁，避免内存泄漏，具体创建范例：
//      
//      SmartPtr<IMyObject> spObj = CFactory::createObject<CMyObject, IMyObject>();
//      SmartPtr<IFactory> spFactory = CFactory::createFactory<CMyObject, IFactory>()
//


//
// 工厂基类，其功能包括:
//      1, 提供对象创建能力CFactory::createObject
//      2, 提供工厂对象创建能力CFactory::createFactory
//      3, 可以作为自定义工厂的基类
//
class CFactory : CObject, IFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

//
// 下面是模板函数，用于实现对象活着工厂的创建
//
public:
    //
    // 创建对象，并查询获取对象接口
    //
    template<typename TObject> static IObjectPtr createObject() {
        return CObjectImp<TObject>::createObjectImp();
    }

    //
    // 创建工厂
    //  @T 工厂用于创建的对象类
    //  @Q 工厂实现类
    //
    template<typename TObject, typename TFactory=CFactory> static IObjectPtr createFactory() {
        return CFactoryImp<TObject,TFactory>::createFactoryImp();
    }

private:
    template<typename TObject> class CObjectImp : public TObject {
        SIMPLEWORK_INTERFACE_ENTRY_ENTER(TObject)
        SIMPLEWORK_INTERFACE_ENTRY_LEAVE(TObject)

    public://IObject
        CObjectImp() { m_nRefCnt = 0; }
        int __swAddRef() { return ++m_nRefCnt; }
        int __swDecRef() {
            int nRefCnt = --m_nRefCnt;
            if( nRefCnt == 0 ) {
                delete this;
            }
            return nRefCnt;
        }

        static IObjectPtr createObjectImp() {
            return IObjectPtr( (IObject*)(CObject*)(new CObjectImp<TObject>()) );
        }

    private:
        int m_nRefCnt;
    };

    template<typename TObject, typename TFactory> class CFactoryImp : public TFactory {
    public://IFactory
        IObjectPtr createObject() {
            return CObjectImp<TObject>::createObjectImp();
        }

    public:
        static IObjectPtr createFactoryImp() {
            return CObjectImp<CFactoryImp<TObject, TFactory>>::createObjectImp();
        }
    };
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_CFactory__h__