#include <iostream>
#include "../inc/SimpleWork.h"

using namespace sw::core;
using namespace sw::tensor;

class CMyObject : public CObject {

}; 

int main(int argc, char *argv[]){

/*
    Factory spFactory = CFactory::createFactory<CMyObject>();
    
    std::cout << "startRegister = " << (spFactory ? "nullprt" : "validptr" );
    
    getCoreApi()->registerFactory("MyObject", spFactory.getPtr());
    
    TObject spOject = getCoreApi()->createObject("MyObject");

    TObject spObject = spFactory.createObject();
    spObject = IObjectNullptr;
    if( !spObject ) {
        std::cout << "Great\n";
    }
    
*/
    ICoreApiPtr spCoreApi = getCoreApi();
    IObjectPtr spTensor = spCoreApi->createObject("sw.math.Tensor");
    if( spTensor ) {
        std::cout << "Great\n";
    }

    //TObject<IFactory> fac;
    //TObject<ICoreApi> coreapi;
    //TObject<IModule> module = fac;
    //fac = coreapi;

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
