#ifndef __SimpleWork_NnModule_h__
#define __SimpleWork_NnModule_h__

#include "nn.h"

SIMPLEWORK_NN_NAMESPACE_ENTER

class SNnModule;

//
// 神经网络单元
//
SIMPLEWORK_INTERFACECLASS_ENTER0(NnModule)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.INnModule", 211202)

        //
        // 单元求解
        //
        virtual int eval(int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    static SNnModule createDenseModule(const PNnDense& rData){
        return SObject::createObject("sw.nn.DenseModule", CData<PNnDense>(rData));
    }
    static SNnModule createConvModule(const PNnConv& rData) {
        return SObject::createObject("sw.nn.ConvModule", CData<PNnConv>(rData));
    }
    static SNnModule createPoolModule(const PNnPool& rData){
        return SObject::createObject("sw.nn.PoolModule", CData<PNnPool>(rData));
    }
    static SNnModule createRNnModule(const PNnRnn& rData) {
        return SObject::createObject("sw.nn.RnnModule", CData<PNnRnn>(rData));
    }
    static SNnModule createGruModule(const PNnRnn& rData) {
        return SObject::createObject("sw.nn.GruModule", CData<PNnRnn>(rData));
    }
    static SNnModule createBatchNormalizeModule(const PNnBatchNormalize& rData) {
        return SObject::createObject("sw.nn.BatchNormalizeModule", CData<PNnBatchNormalize>(rData));
    }
    static SNnModule createGvpModule() {
        return SObject::createObject("sw.nn.GvpModule", nullptr);
    }

    struct PNnCompositeModule {
        SIMPLEWORK_PDATAKEY(PNnCompositeModule, "sw.nn.NnCompositeModule")

        int nModules;
        SNnModule* pModules;
    };
    static SModule createCompositeModule(const PNnCompositeModule& rData) {
        return SObject::createObject("sw.nn.CompositeModule", CData<PNnCompositeModule>(rData));
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(NnModule)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NnModule_h__