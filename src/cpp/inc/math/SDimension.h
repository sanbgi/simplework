#ifndef __SimpleWork_Dimension_h__
#define __SimpleWork_Dimension_h__

#include "SMathFactory.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

class STensor;

/**
 * 张量类定义
 */
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

        //
        // 获取表示维度信息的张量
        //
        virtual int getVector(STensor& spDimVector) = 0;

    SIMPLEWORK_INTERFACE_LEAVE
    
    int size() {
        IFace* pFace = getPtr();
        return pFace != nullptr ? pFace->getSize() : 0;
    }

    const int* data() const {
        IFace* pFace = getPtr();
        return pFace != nullptr ? pFace->getData() : nullptr;
    }

    int dataSize() {
        IFace* pFace = getPtr();
        return pFace != nullptr ? pFace->getElementSize() : 0; 
    }

    static int createDimension(SDimension& spDim, int nDims, const int* pDimSizes) {
        return SMathFactory::getFactory()->createDimension(spDim, nDims, pDimSizes);
    }

    static SDimension createDimension(int nDims, const int* pDimSizes) {
        SDimension spDim;
        SMathFactory::getFactory()->createDimension(spDim, nDims, pDimSizes);
        return spDim;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(Dimension)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_Dimension_h__