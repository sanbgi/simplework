#ifndef __SimpleWork_NN_CConvUnit_H__
#define __SimpleWork_NN_CConvUnit_H__

#include "nn.h"
#include <string>

using namespace sw;
using namespace std;

class CConvUnit : public CObject, public INnUnit, public IIoArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnUnit)
        SIMPLEWORK_INTERFACE_ENTRY(IIoArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://IIoArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "ConvUnit"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SIoArchive& ar);

private://INnUnit
    int eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.ConvUnit"; }
    static int createUnit(int nWidth, int nHeight, int nLayers, int nShiftConvs, const char* szPaddingMode, const char* szActivator, SNnUnit& spUnit);

private:
    //基础参数
    int m_nWidth;
    int m_nHeight;
    int m_nLayers;
    int m_nShiftConvs;
    double m_dDropoutRate;
    string m_strPaddingMode;
    string m_strActivator;
    SNnVariable m_spWeights;
    SNnVariable m_spBais;

public:
    CConvUnit() {
    }
};

#endif//__SimpleWork_NN_CConvUnit_H__
