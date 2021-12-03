#include <iostream>
#include "../inc/SimpleWork.h"

using namespace sw::core;
using namespace sw::math;

    SIMPLEWORK_INTERFACECLASS_ENTER(MyObject)

        SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.core.IModule", 211202)
            virtual void sayHi() = 0;
        SIMPLEWORK_OBJECT_INTERFACE_LEAVE

    public:
        void sayHi() {
            if(m_autoPtr) {
                m_autoPtr->sayHi();
            }
        }
    SIMPLEWORK_INTERFACE_LEAVE(MyObject)

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


int main(int argc, char *argv[]){
    
    Object tensor = Object::createObject("sw.math.Tensor");
    Factory factory = Object::createFactory("sw.math.Tensor");
    tensor = factory;
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
