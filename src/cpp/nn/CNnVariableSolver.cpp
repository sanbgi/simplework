
#include "nn.h"
#include "CNnWeightVariable.h"
#include "CNnStateVariable.h"
#include "CNnOperator.h"
#include "CNnVariableSolver.h"

static SCtx sCtx("CNnVariableSolver");
int CNnVariableSolver::createWeightVariable(const SDimension& spDimension, SNnVariable& spVar) {
    return CNnWeightVariable::createWeightVariable(spDimension, spVar);
}

int CNnVariableSolver::createStateVariable(const SDimension& spDimension, SNnVariable& spVar){
    return CNnStateVariable::createStateVariable(spDimension, spVar);
}

int CNnVariableSolver::solveOp(const char* szOp, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) {
    return CNnOperator::createOperator(szOp, nInVars, pInVars, spOutVar);
}

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CNnVariableSolver, SNnVariableSolver::__getClassKey())