#ifndef __SimpleWork_NNUNIT_CBaisUnit_H__
#define __SimpleWork_NNUNIT_CBaisUnit_H__

#include "CNnUnit.h"

class CBaisUnit : public CObject, public INnUnit, public IIoArchivable{
    
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnUnit)
        SIMPLEWORK_INTERFACE_ENTRY(IIoArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private:
    CNUUNIT_INNUNIT_IMPLEMENTATION

public:
    template<typename Q> int initT(int nCells);
    template<typename Q> int evalT(const Q* pIn, Q* pOut);
    template<typename Q> int learnT(const Q* pIn, const Q* pOut, const Q* pOutDev, Q* pInDev);
    template<typename Q> int updateWeightsT();

private:
    int m_nCells;
    unsigned int m_idType;
    CTaker<char*> m_spBais;
    string m_strOptimizer;

    int m_nBatchs;
    SOptimizer m_spOptimizer;

private://IIoArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "BaisUnit"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SIoArchive& ar);

public:
    static const char* __getClassKey() { return "sw.nn.BaisUnit"; }
    static int createUnit(SNnUnit& spUnit, int nCells, unsigned int idType, const char* szOptimizer);

public:
    CBaisUnit(){
        m_nCells = 0;
        m_nBatchs = 0;
    }
};

#endif//__SimpleWork_NNUNIT_CBaisUnit_H__