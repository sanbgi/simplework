#ifndef __SimpleWork_NN_COptmizer_H__
#define __SimpleWork_NN_COptmizer_H__

#include "nn.h"

using namespace sw;

//
// 神经网络核心接口定义
//
SIMPLEWORK_INTERFACECLASS_ENTER0(Optimizer)

    SIMPLEWORK_INTERFACE_ENTER(sw::IObject, "sw.nn.IOptimizer", 220107)

        //
        // 获取用于保存输入偏导数的地址
        //
        virtual void* getDeviationPtr(int nDeviations) = 0;

        //
        // 更新偏差值
        //
        virtual int updateDeviation(int nBatchSize) = 0;

        //
        // 更新偏差值
        //
        virtual int updateDeviation(PDATATYPE idType, int nBatchs, const SDevice& spDevice, int nDeviations, void* pDeviations ) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(Optimizer)

class COptimizer {
public:
    static int getOptimizer(const char* szOptimizer, PDATATYPE idType, SOptimizer& spOptimizer); 
};

#endif//__SimpleWork_NN_COptmizer_H__
