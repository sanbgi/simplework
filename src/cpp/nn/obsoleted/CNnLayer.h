#ifndef __SimpleWork_NN_CNnLayer_H__
#define __SimpleWork_NN_CNnLayer_H__

#include "nn.h"
#include "COptimizer.h"
#include <vector>
#include <string>

using namespace sw;

class CNnLayer : public CObject, public INnLayer, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnLayer)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "Layer"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.Layer"; }
    static int createLayer(const SNnUnit& spUnit, const char* szMode, SNnLayer& spLayer);

public:
    int getUnit(SNnUnit& spUnit);
    ENnLayerMode getMode();

private:
    ENnLayerMode m_eMode;
    SNnUnit m_spUnit;

public:
    CNnLayer() {
        m_eMode = SNnLayer::EMODE_NONE;
    }
};

#endif//__SimpleWork_NN_CNnLayer_H__
