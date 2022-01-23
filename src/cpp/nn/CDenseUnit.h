#ifndef __SimpleWork_NN_CDenseUnit_H__
#define __SimpleWork_NN_CDenseUnit_H__

#include "nn.h"
#include <string>

using namespace sw;
using namespace std;

//
//  全连接神经网络，注意数据格式：
//
//      输入数据：nBatchs(第一个维度) * nCells(余下所有维度)
//      输出数据：nBatchs(第一个维度) * nCells(第二个维度)
//
//  比如：输入维度[10, 3, 1]，表示一共有十个输入数据，每一个数据数据包含3 X 1个输入神经元
//
class CDenseUnit : public CObject, public INnUnit, public IIoArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnUnit)
        SIMPLEWORK_INTERFACE_ENTRY(IIoArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://IIoArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "DenseUnit"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SIoArchive& ar);

private://INnUnit
    int eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.DenseUnit"; }
    static int createUnit(int nCells, double dDropoutRate, const char* szActivator, SNnUnit& spUnit);

private:
    //基础参数
    int m_nCells;
    double m_dDropoutRate;
    string m_strActivator;
    SNnVariable m_spWeights;
    SNnVariable m_spBais;

public:
    CDenseUnit() {
        m_nCells = 0;
    }
};

#endif//__SimpleWork_NN_CDenseUnit_H__
