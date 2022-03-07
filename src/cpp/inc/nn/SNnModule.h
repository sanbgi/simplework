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
        return SObject::createObject("sw.nn.DenseModule", CArg<PNnDense>(rData));
    }
    static SNnModule createConvModule(const PNnConv& rData) {
        return SObject::createObject("sw.nn.ConvModule", CArg<PNnConv>(rData));
    }
    static SNnModule createPoolModule(const PNnPool& rData){
        return SObject::createObject("sw.nn.PoolModule", CArg<PNnPool>(rData));
    }
    static SNnModule createRNnModule(const PNnRnn& rData) {
        return SObject::createObject("sw.nn.RnnModule", CArg<PNnRnn>(rData));
    }
    static SNnModule createGruModule(const PNnRnn& rData) {
        return SObject::createObject("sw.nn.GruModule", CArg<PNnRnn>(rData));
    }
    static SNnModule createBatchNormalizeModule(const PNnBatchNormalize& rData) {
        return SObject::createObject("sw.nn.BatchNormalizeModule", CArg<PNnBatchNormalize>(rData));
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
        return SObject::createObject("sw.nn.CompositeModule", CArg<PNnCompositeModule>(rData));
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(NnModule)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NnModule_h__