#ifndef __SimpleWork_NnUnit_h__
#define __SimpleWork_NnUnit_h__

#include "nn.h"
#include "SNnFactory.h"

SIMPLEWORK_NN_NAMESPACE_ENTER

class SNnVariable;

//
// 神经网络单元
//
SIMPLEWORK_INTERFACECLASS_ENTER0(NnUnit)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.INnUnit", 211202)

        //
        // 单元求解
        //
        virtual int eval(int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    static SNnUnit createDenseUnit(const PNnDense& rData){
        return SObject::createObject("sw.nn.DenseUnit", CData<PNnDense>(rData));
    }
    static SNnUnit createConvUnit(const PNnConv& rData) {
        return SObject::createObject("sw.nn.ConvUnit", CData<PNnConv>(rData));
    }
    static SNnUnit createPoolUnit(const PNnPool& rData){
        return SObject::createObject("sw.nn.PoolUnit", CData<PNnPool>(rData));
    }
    static SNnUnit createRnnUnit(const PNnRnn& rData) {
        return SObject::createObject("sw.nn.RnnUnit", CData<PNnRnn>(rData));
    }
    static SNnUnit createGruUnit(const PNnRnn& rData) {
        return SObject::createObject("sw.nn.GruUnit", CData<PNnRnn>(rData));
    }
    static SNnUnit createBatchNormalizeUnit(const PNnBatchNormalize& rData) {
        return SObject::createObject("sw.nn.BatchNormalizeUnit", CData<PNnBatchNormalize>(rData));
    }
    static SNnUnit createGvpUnit() {
        return SObject::createObject("sw.nn.GvpUnit", nullptr);
    }
    static SNnUnit createSequenceUnit(int nUnits, const SNnUnit pUnits[]) {
        SNnUnit spUnit;
        SNnFactory::getFactory()->createSequenceUnit(nUnits, pUnits, spUnit);
        return spUnit;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(NnUnit)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NnUnit_h__