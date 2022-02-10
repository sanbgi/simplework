#ifndef __SimpleWork_NN_PNnNetwork_H__
#define __SimpleWork_NN_PNnNetwork_H__

#include "nn.h"

SIMPLEWORK_NN_NAMESPACE_ENTER 

class SNnVariable;

//
// 网络求解器定义
//
struct INnNetworkSolver {
    //
    // 求解函数
    //
    virtual int solve(const SNnVariable& spIn, SNnVariable& spOut) = 0;
};

//
// 网络参数
//
struct PNnNetwork {
    SIMPLEWORK_PDATAKEY(PNnNetwork, "sw.nn.NnNetwork")

    //
    // 输入维度
    //
    SDimension spInDimension;

    //
    // 求解函数
    //
    INnNetworkSolver* pSolver;

    //
    // 求解器指针构造函数
    //
    PNnNetwork(const SDimension& spInDimension, INnNetworkSolver* pSolver) {
        this->spInDimension = spInDimension;
        this->pSolver = pSolver;
    }

    //
    // 网络求解函数定义
    //
    typedef int (*FNnNetworkSolver)(const SNnVariable& spIn, SNnVariable& spOut);
    struct CNnNetworkSolver : INnNetworkSolver {
        int solve(const SNnVariable& spIn, SNnVariable& spOut) {
            return (*funSolver)(spIn, spOut);
        }

    public:
        FNnNetworkSolver funSolver;
    }fSolver;

    //
    // 求解函数构造函数
    //
    PNnNetwork(const SDimension& spInDimension, FNnNetworkSolver funSolver) {
        this->spInDimension = spInDimension;
        this->fSolver.funSolver = funSolver;
        this->pSolver = &this->fSolver;
    }
};

SIMPLEWORK_NN_NAMESPACE_LEAVE


#endif//__SimpleWork_NN_PNnNetwork_H__