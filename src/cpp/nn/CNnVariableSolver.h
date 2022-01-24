#ifndef __SimpleWork_NN_CNnVariableSolver_H__
#define __SimpleWork_NN_CNnVariableSolver_H__

#include "nn.h"

using namespace sw;

class CNnVariableSolver : public CObject, public INnVariableSolver{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnVariableSolver)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int createWeightVariable(const SDimension& spDimension, SNnVariable& spVar);
    int createStateVariable(const SDimension& spDimension, SNnVariable& spVar);
    int solveOp(const char* szOp, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar);
};

#endif//__SimpleWork_NN_CNnVariableSolver_H__
