#ifndef __SimpleWork_CObject__h__
#define __SimpleWork_CObject__h__

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
// 对象基类
//  @remark 
//      所有对象都要从这个基类派生，这个基类帮助实现
//      1，IObject接口，包括引用技术管理，接口转化等，如果子类有自己派生的接口，则需要实现一个接口转化函数
//
class CObject : public IObject {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER0
        SIMPLEWORK_INTERFACE_ENTRY(IObject)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE0
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_CObject__h__