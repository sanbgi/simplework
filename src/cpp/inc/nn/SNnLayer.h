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
    static SNnLayer createDenseLayer(int nCells, double dDropoutRate=0, const char* szActivator = nullptr){
        return createLayer(SNnUnit::createDenseUnit(nCells,dDropoutRate,szActivator));
    }
    static SNnLayer createRnnLayer(int nCells, const char* szMode=nullptr, const char* szActivator = nullptr){
        return createLayer(SNnUnit::createRnnUnit(nCells,szActivator), szMode);
    }
    static SNnLayer createGruLayer(int nCells, const char* szMode=nullptr){
        return createLayer(SNnUnit::createGruUnit(nCells), szMode);
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