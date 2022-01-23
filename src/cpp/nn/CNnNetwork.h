#ifndef __SimpleWork_NN_CNnNetwork_H__
#define __SimpleWork_NN_CNnNetwork_H__

#include "nn.h"
#include "COptimizer.h"
#include <vector>
#include <string>

using namespace sw;
using namespace std;

class CNnOperator;
class CNnVariable;

//
// 计算函数
//
typedef void (*FEval)(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar);

//
// 计算变量
//
struct PSolveVar {
    int type;//ENnVariableType
    int size;
    void* data; //存储状态及权重值的指针
    void* devia;//存储偏倒数的指针
    CNnVariable* pVar;
};

struct PSolveInstruct {
    //
    // 输入参数个数及位置
    //
    int nInVar;
    int pInVarIndex[4];

    //
    // 输出参数位置
    //
    int iOutVarIndex;

    //
    // 操作对象指针
    //
    CNnOperator* pOperator;

    //
    // 计算值和偏导数的函数指针
    //
    FEval pFunEval;
    FEval pFunDevia;
}; 

class CNnNetwork : public CObject, public INnNetwork{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    static int saveNetwork(const char* szFileName, const SNnNetwork& spNet);
    static int loadNetwork(const char* szFileName, SNnNetwork& spNet);
    static int createNetwork(const SNnUnit& spUnit, const SDimension spInDimVector, SNnNetwork& spNet);

public://INnNetwork
    int eval(const STensor& spBatchIn, STensor& spBatchOut);
    int learn(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation);

private:
    template<typename Q>
    int evalT(const STensor& spBatchIn, STensor& spBatchOut);

    template<typename Q>
    int learnT(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation);


private:
    SNnUnit m_spUnit;
    SDimension m_spInDimVector;
    string m_strOptimizer;

    bool m_bInitialized;

    SNnVariable m_spOutVar;

    //单元求解后的变量
    int m_nStateSize;
    int m_nWeightSize;
    int m_nOpVarSize;
    int m_iInputVarIndex;
    int m_nInputTensorSize;
    int m_nOutputTensorSize;

    //解算参数列表
    vector<PSolveVar> m_arrVars;
    //解算步骤列表
    vector<PSolveInstruct> m_arrSolvers;

    int m_nBatchIns;
    int m_nBatchInSize;
    unsigned int m_idType;
    SOptimizer m_spOptimizer;

    STensor m_spSolveBuffer;
    STensor m_spDeviaBuffer;
    STensor m_spBatchIn;
    STensor m_spBatchOut;
    STensor m_spBatchInDeviation;
    int m_nBatchOutVer;

public:
    int initNetwork();
    int prepareNetwork(const STensor& spBatchIn);

    CNnNetwork() {
        m_bInitialized = false;
        m_nBatchInSize = 0;
        m_idType = 0;
    }
};

#endif//__SimpleWork_NN_CNnNetwork_H__
