#ifndef __SimpleWork_NnUnit_h__
#define __SimpleWork_NnUnit_h__

#include "nn.h"
#include "SNnFactory.h"

SIMPLEWORK_NN_NAMESPACE_ENTER

class SNnVariable;

//
// 神经网络单元
//
SIMPLEWORK_INTERFACECLASS_ENTER(NnUnit, "sw.nn.NnUnit")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.INnUnit", 211202)

        //
        // 获取变量维度
        //
        virtual int eval(int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    static SNnUnit createDenseUnit(int nCells, double dDropoutRate=0, const char* szActivator = nullptr){
        SNnUnit spUnit;
        SNnFactory::getFactory()->createDenseUnit(nCells, dDropoutRate, szActivator, spUnit);
        return spUnit;
    }
    static SNnUnit createConvUnit(int nWidth, int nHeight, int nLayers, int nShiftConvs = 1, const char* szPaddingMode = nullptr, const char* szActivator = nullptr) {
        SNnUnit spUnit;
        SNnFactory::getFactory()->createConvUnit(nWidth, nHeight, nLayers, nShiftConvs, szPaddingMode, szActivator, spUnit);
        return spUnit;
    }
    static SNnUnit createPoolUnit(int nWidth, int nHeight, int nStride, const char* szPaddingMode=nullptr){
        SNnUnit spUnit;
        SNnFactory::getFactory()->createPoolUnit(nWidth, nHeight, nStride, szPaddingMode, spUnit);
        return spUnit;
    }
    static SNnUnit createRnnUnit(int nCells, const char* szActivator=nullptr) {
        SNnUnit spUnit;
        SNnFactory::getFactory()->createRnnUnit(nCells, szActivator, spUnit);
        return spUnit;
    }
    static SNnUnit createSequenceUnit(int nUnits, const SNnUnit pUnits[]) {
        SNnUnit spUnit;
        SNnFactory::getFactory()->createSequenceUnit(nUnits, pUnits, spUnit);
        return spUnit;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(NnUnit)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NnUnit_h__