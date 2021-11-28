#include <iostream>
#include "../inc/SimpleWork.h"

using namespace sw;

class CMyObject : public CObject {

}; 

int main(int argc, char *argv[]){

    Factory spFactory = CFactory::createFactory<CMyObject>();
    
    std::cout << "startRegister = " << (spFactory ? "nullprt" : "validptr" );
    
    getSimpleWorkModule()->registerFactory("MyObject", spFactory.getPtr());

    Object spOject = getSimpleWorkModule()->createObject("MyObject");
    Object spObject = spFactory.createObject();
    if( !spObject ) {
        std::cout << "Great\n";
    }
    
    return 0;
}


void say_hello(){
    sw::Object ppo;
    sw::Object ptr1, ptr0, ptrn(ptr1);
    sw::Tensor ppt;
    sw::Tensor ptr2, ptr3(ptr2);
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
