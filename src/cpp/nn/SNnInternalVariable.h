#ifndef __SimpleWork_NN_SNnInternalVariable_H__
#define __SimpleWork_NN_SNnInternalVariable_H__

#include "nn.h"

using namespace sw;
using namespace std;

enum ENnVariableType {
    EVInput,
    EVState,
    EVWeight,
    EVOperator,
    EVMax
};

//
// 神经网络计算器，为了实现高速计算，参数和函数地址，都是直接的指针
//
SIMPLEWORK_INTERFACECLASS_ENTER0(NnInternalVariable)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.nn.INnInternalVariable", 211202)

        virtual int getSize() = 0;
        virtual void* getData(PDATATYPE idType) = 0;
        virtual ENnVariableType getVariableType() = 0;
        virtual int getDimension(SDimension& spDimension) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    SDimension dimension() const{
        SDimension spDimension;
        IFace* pFace = getPtr();
        if(pFace) {
            pFace->getDimension(spDimension);
        }
        return spDimension;
    }
    
SIMPLEWORK_INTERFACECLASS_LEAVE(NnInternalVariable)

#endif//__SimpleWork_NN_SNnInternalVariable_H__
