#ifndef __SimpleWork_NNUNIT_SNnUnit_H__
#define __SimpleWork_NNUNIT_SNnUnit_H__

#include "../nn.h"

using namespace sw;

SIMPLEWORK_INTERFACECLASS_ENTER0(NnUnit)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.INnUnit", 220118)

        //
        // 计算
        //
        virtual int eval(unsigned int idType, const void* pIn, void* pOut) = 0;

        //
        // 学习
        //
        virtual int learn(unsigned int idType, const void* pIn, const void* pOut, const void* pOutDev, void* pInDev) = 0;

        //
        // 更新权重
        //
        virtual int updateWeights() = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(NnUnit)

#endif//__SimpleWork_NNUNIT_SNnUnit_H__