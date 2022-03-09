#ifndef __SimpleWork_Dimension_h__
#define __SimpleWork_Dimension_h__

#include "SMathFactory.h"
#include "SDimensionSolver.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

//
// 维度定义
//
SIMPLEWORK_INTERFACECLASS_ENTER(Dimension, "sw.math.Dimension")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.IDimension", 211202)

        //
        // 获取维度大小
        //
        virtual int getSize() = 0;

        //
        // 获取维度数据
        //
        virtual const int* getData() = 0;

        //
        // 获取维度表示的整体大小
        //
        virtual int getElementSize() = 0;

    SIMPLEWORK_INTERFACE_LEAVE
    
    SDimension(int nDim, const int* pDimSizes) {
        SMathFactory::getFactory()->createDimension(*this, nDim, pDimSizes);
    }

    int size() const {
        IFace* pFace = getPtr();
        return pFace != nullptr ? pFace->getSize() : 0;
    }

    const int* data() const {
        IFace* pFace = getPtr();
        return pFace != nullptr ? pFace->getData() : nullptr;
    }

    int dataSize() const {
        IFace* pFace = getPtr();
        return pFace != nullptr ? pFace->getElementSize() : 0; 
    }

    SDimension upHighDimension(int nDimSize) {
        SDimension spDim;
        SDimensionSolver::getSolver()->upHighDimension(*this, 1, &nDimSize, spDim);
        return spDim;
    }

    SDimension downHighDimension(int nDims = 1) {
        SDimension spDim;
        SDimensionSolver::getSolver()->downHighDimension(*this, nDims, spDim);
        return spDim;
    }

    SDimension upLowDimension(int nDimSize) {
        SDimension spDim;
        SDimensionSolver::getSolver()->upLowDimension(*this, 1, &nDimSize, spDim);
        return spDim;
    }

    SDimension downLowDimension(int nDims = 1) {
        SDimension spDim;
        SDimensionSolver::getSolver()->downLowDimension(*this, nDims, spDim);
        return spDim;
    }

    bool isEqual(const SDimension& spDim) {
        return SDimensionSolver::getSolver()->isEqual(*this, spDim);;
    }

    static int createDimension(SDimension& spDim, int nDims, const int* pDimSizes) {
        return SMathFactory::getFactory()->createDimension(spDim, nDims, pDimSizes);
    }

    static SDimension createDimension(int nDims, const int* pDimSizes) {
        SDimension spDim;
        SMathFactory::getFactory()->createDimension(spDim, nDims, pDimSizes);
        return spDim;
    }

    template<typename Q> static SDimension createDimension(Q v) {
        SDimension spDim;
        SMathFactory::getFactory()->createDimension(spDim, v.size(), v.data());
        return spDim;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(Dimension)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_Dimension_h__