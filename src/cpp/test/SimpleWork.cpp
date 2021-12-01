#include <iostream>
#include "../inc/SimpleWork.h"

using namespace sw::io;
using namespace sw::core;
using namespace sw::tensor;

    SIMPLEWORK_INTERFACE_ENTER(IMyObject, IObject, "TestSimpleWork.IMyObject", 011130)
        virtual void sayHi() = 0;
    SIMPLEWORK_INTERFACE_LEAVE

    class CMyObject : public CObject, public IMyObject {
        SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
            SIMPLEWORK_INTERFACE_ENTRY(IMyObject)
        SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

    public:
        void sayHi() {
            std::cout << "Great ! Hi everyone!";
        }
    };
    SIMPLEWORK_FACTORY_REGISTER(CMyObject, "TestSimpleWork.MyObject")

    class MyObject : public TObject<MyObject, IMyObject> {
        SIMPLEWORK_OBJECT_DATACONVERSION(MyObject)
    public:
        void sayHi() {
            if(m_autoPtr) {
                m_autoPtr->sayHi();
            }
        }
    };

int main(int argc, char *argv[]){
    
/*
    Factory factory;
    Object object;
    Object object2(object);
    //Object object = factory;
    factory = object;
    object = factory;
    //int* pv = &v;

    Factory spFactory = CObject::createFactory<CMyObject>();
    
    std::cout << "startRegister = " << (spFactory ? "nullprt" : "validptr" );
    
    getCoreApi()->registerFactory("MyObject", spFactory.getPtr());
    
    TObject spOject = getCoreApi()->createObject("MyObject");

    TObject spObject = spFactory.createObject();
    spObject = IObjectNullptr;
    if( !spObject ) {
        std::cout << "Great\n";
    }
    
*/
    Object tensor = Object::createObject("sw.math.Tensor");
    Factory factory = Object::createFactory("sw.math.Tensor");
    tensor = factory;
    IObjectPtr spPtr = factory.getAutoPtr();
    if( tensor ) {
        std::cout << "Great\n";
    }

    MyObject myObject = Object::createObject("TestSimpleWork.MyObject");
    if( myObject ) {
        myObject.sayHi();
    }else {
        std::cout << "??？ What happen?";
    }
    return 0;
}


void say_hello(){
    /*
    TObject ppo;
    TObject ptr1, ptr0, ptrn(ptr1);
    Tensor ppt;
    Tensor ptr2, ptr3(ptr2);
    //char szV[] = "hi";
    //swcore::Tensor ptrTensor(sizeof(szV)/sizeof(char), szV);
    //char* ptrChar = ptrTensor.getDataPtr<char>();
    //swcore::Tensor tensor = swcore::Tensor::createTensor<char>(1);
    IFactory* pFactory = nullptr;
    ITensorPtr ppO;
    ppO = pFactory;
    //ptr2 = pFactory;
    ptr3 = ptr2;
    ptr2 = ptr1;
    ptr1 = ptr2;
    ptr3 = ptr2 = ptr1 = ptr0;
    ptr3 = ptr2.getPtr();
    */
}
