#include <iostream>
#include "../inc/SimpleWork.h"

using namespace SimpleWork;

class CMyObject : public CObject {

}; 

int main(int argc, char *argv[]){

    Factory spFactory = CFactory::createFactory<CMyObject>();
    
    std::cout << "startRegister = " << (spFactory.isNullPtr() ? "nullprt" : "validptr" );
    
    getSimpleWorkModule()->registerFactory("MyObject", spFactory.getPtr());

    Object spOject = getSimpleWorkModule()->createObject("MyObject");
    Object spObject = spFactory.createObject();
    if( !spObject.isNullPtr() ) {
        std::cout << "Great\n";
    }
    
    return 0;
}


void say_hello(){
    SimpleWork::Object ppo;
    SimpleWork::Object ptr1, ptr0, ptrn(ptr1);
    SimpleWork::Tensor ppt;
    SimpleWork::Tensor ptr2, ptr3(ptr2);
    //char szV[] = "hi";
    //swcore::Tensor ptrTensor(sizeof(szV)/sizeof(char), szV);
    //char* ptrChar = ptrTensor.getDataPtr<char>();
    //swcore::Tensor tensor = swcore::Tensor::createTensor<char>(1);
    ptr3 = ptr2;
    ptr2 = ptr1;
    ptr1 = ptr2;
    ptr3 = ptr2 = ptr1 = ptr0;
    ptr3 = ptr2.getPtr();
}

SimpleWork::IModule* getSimpleWorkModule() {
    static SimpleWork::IModule* s_pModule = SimpleWork::getCoreModule();
    return s_pModule;
}