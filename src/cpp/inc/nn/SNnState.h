#ifndef __SimpleWork_NnState_h__
#define __SimpleWork_NnState_h__

#include "nn.h"
#include "SNnVariableSolver.h"

SIMPLEWORK_NN_NAMESPACE_ENTER

//
// 抽象变量定义，变量可以表示任何可运算的对象，包括：值/向量/矩阵/张量/单位等等
//
SIMPLEWORK_INTERFACECLASS_ENTER(NnState, "sw.nn.NnState")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.nn.INnState", 211202)

        //
        // 获取变量维度
        //
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

public:
    static int createState(const SDimension& spDimension, SNnState& spVar) {
        return SNnVariableSolver::getSolver()->createState(spDimension, spVar);
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(NnState)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NnState_h__