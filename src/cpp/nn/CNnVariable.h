#ifndef __SimpleWork_NN_CNnVariable_H__
#define __SimpleWork_NN_CNnVariable_H__

#include "nn.h"

using namespace sw;
using namespace std;

class CNnVariable;

enum ENnVariableType {
    EVInput,
    EVState,
    EVWeight,
    EVOperator,
};

//
// 神经网络计算器，为了实现高速计算，参数和函数地址，都是直接的指针
//
SIMPLEWORK_INTERFACECLASS_ENTER0(NnInternalVariable)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.nn.INnInternalVariable", 211202)

        virtual int getSize() = 0;
        virtual void* getData(unsigned int idType) = 0;
        virtual CNnVariable* getVariablePtr() = 0;
        virtual ENnVariableType getVariableType() = 0;
        virtual int getSubVariables(SNnInternalVariable pSubVariables[4]) = 0;
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

class CNnVariable : public CObject, public INnVariable, public INnInternalVariable {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnVariable)
        SIMPLEWORK_INTERFACE_ENTRY(INnInternalVariable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://INnVariable
    int getDimension(SDimension& spDimension) {
        spDimension = m_spDimension;
        return 0;
    }
    int getSize() {
        if(!m_spDimension) {
            return -1;
        }

        int nDims = m_spDimension.size();
        const int* pDims = m_spDimension.data();
        if( pDims == nullptr ) {
            return -1;
        }

        int size = 1;
        for(int i=0; i<nDims; i++) {
            size *= pDims[i];
        }
        return size;
    }

protected:
    SDimension m_spDimension;
};
#endif//__SimpleWork_NN_CNnVariable_H__
