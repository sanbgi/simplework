#ifndef __SimpleWork_NnLayer_h__
#define __SimpleWork_NnLayer_h__

#include "nn.h"
#include "SNnUnit.h"
#include "SNnFactory.h"

SIMPLEWORK_NN_NAMESPACE_ENTER


//
// 神经网络层
//
SIMPLEWORK_INTERFACECLASS_ENTER0(NnLayer)
    //
    // 层运行模式
    //
    enum ENnLayerMode {
        EMODE_NONE,
        EMODE_BATCH,
        EMODE_SEQUENCE,
    };

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.INnLayer", 211223)

        //
        // 层求解
        //
        virtual int getUnit(SNnUnit& spUnit) = 0;

        //
        // 层运行模式
        //
        virtual ENnLayerMode getMode() = 0;
        
    SIMPLEWORK_INTERFACE_LEAVE

    SNnUnit getUnit() {
        SNnUnit spUnit;
        IFace* pFace = getPtr();
        if(pFace!=nullptr) {
            pFace->getUnit(spUnit);
        }
        return spUnit;
    }

    static SNnLayer createLayer(const SNnUnit& spUnit, const char* szMode=nullptr) {
        SNnLayer spLayer;
        SNnFactory::getFactory()->createLayer(spUnit, szMode, spLayer);
        return spLayer;
    }
    static SNnLayer createDenseLayer(const PNnDense& rDense){
        return createLayer(SNnUnit::createDenseUnit(rDense));
    }
    static SNnLayer createRnnLayer(const PNnRnn& rRnn, const char* szMode=nullptr){
        return createLayer(SNnUnit::createRnnUnit(rRnn), szMode);
    }
    static SNnLayer createGruLayer(const PNnRnn& rRnn, const char* szMode=nullptr){
        return createLayer(SNnUnit::createGruUnit(rRnn), szMode);
    }
    static SNnLayer createConvLayer(const PNnConv& rConv) {
        return createLayer(SNnUnit::createConvUnit(rConv));
    }
    static SNnLayer createPoolLayer(const PNnPool& rPool){
        return createLayer(SNnUnit::createPoolUnit(rPool));
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(NnLayer)

typedef SNnLayer::ENnLayerMode ENnLayerMode;

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NnLayer_h__