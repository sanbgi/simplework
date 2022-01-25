#ifndef __SimpleWork_NnLayer_h__
#define __SimpleWork_NnLayer_h__

#include "nn.h"
#include "SNnUnit.h"
#include "SNnFactory.h"

SIMPLEWORK_NN_NAMESPACE_ENTER


//
// 神经网络核心接口定义
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
    static SNnLayer createConvLayer(int nWidth, int nHeight, int nLayers, int nShiftConvs = 1, const char* szPaddingMode = nullptr, const char* szActivator = nullptr) {
        return createLayer(SNnUnit::createConvUnit(nWidth,nHeight,nLayers,nShiftConvs,szPaddingMode,szActivator));
    }
    static SNnLayer createPoolLayer(int nWidth, int nHeight, int nStride, const char* szPaddingMode=nullptr){
        return createLayer(SNnUnit::createPoolUnit(nWidth,nHeight,nStride,szPaddingMode));
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(NnLayer)

typedef SNnLayer::ENnLayerMode ENnLayerMode;

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NnLayer_h__