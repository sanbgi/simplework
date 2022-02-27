#ifndef __SimpleWork_NN_CTensorSolver_H__
#define __SimpleWork_NN_CTensorSolver_H__

#include "nn.h"

using namespace sw;

class CTensorSolver {
public:
    //
    // 归一化
    //
    static int normalize(const STensor& spIn, STensor& spOut);

    //
    // 分类化张量
    //
    //      将张量中的每一个值，扩展为一个长度为nClassify，值域[0,1]的向量，
    // 其向量中对应位置的值为1.0，其它位置值为0.0
    //
    static int classify(int nClassify, const STensor& spIn, STensor& spOut);
};

#endif//__SimpleWork_NN_CTensorSolver_H__
