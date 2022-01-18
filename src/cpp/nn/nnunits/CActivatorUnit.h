#ifndef __SimpleWork_NNUNIT_CNuActivator_H__
#define __SimpleWork_NNUNIT_CNuActivator_H__

#include "CNnUnit.h"

class CActivatorUnit : public CObject, public INnUnit, public IIoArchivable{
    
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnUnit)
        SIMPLEWORK_INTERFACE_ENTRY(IIoArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private:
    CNUUNIT_INNUNIT_IMPLEMENTATION

public:
    template<typename Q> int initT(const STensor& spInDim, STensor& spOutDim);
    template<typename Q> int evalT(const Q* pIn, Q* pOut);
    template<typename Q> int learnT(const Q* pIn, const Q* pOut, const Q* pOutDev, Q* pInDev);
    template<typename Q> int updateWeightsT();


private://IIoArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "ActivatorUnit"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SIoArchive& ar);

public:
    static const char* __getClassKey() { return "sw.nn.ActivatorUnit"; }
    static int createUnit(SNnUnit& spUnit, int nCells, unsigned int idType, const char* szActivator);

private:
    int m_nCells;
    unsigned int m_idType;
    string m_strActivator;
    CActivator* m_pActivator;

public:
    CActivatorUnit(){
        m_nCells = 0;
    }
};

#endif//__SimpleWork_NNUNIT_CNuActivator_H__