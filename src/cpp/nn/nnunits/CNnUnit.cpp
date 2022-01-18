
#include "CNnUnit.h"
#include "CBaisUnit.h"
#include "CDenseUnit.h"
#include "CActivatorUnit.h"

int CNnUnit::createBais(SNnUnit& spUnit, int nCells, unsigned int idType, const char* szOptimizer) {
    return CBaisUnit::createUnit(spUnit, nCells, idType, szOptimizer);
}

int CNnUnit::createDense(SNnUnit& spUnit, int nCells, unsigned int idType, const char* szOptimizer, const STensor& spInDim){
    return CDenseUnit::createUnit(spUnit, nCells, idType, szOptimizer, spInDim);
}

int CNnUnit::createActivator(SNnUnit& spUnit, int nCells, unsigned int idType, const char* szActivator){
    return CActivatorUnit::createUnit(spUnit, nCells, idType, szActivator);
}