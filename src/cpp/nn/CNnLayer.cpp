#include "CNnLayer.h"
#include <string>
static SCtx sCtx("CNnLayer");
int CNnLayer::createLayer(const SNnUnit& spUnit, const char* szMode, SNnLayer& spLayer) {
    if(!spUnit) {
        return sCtx.error("层的运行单元不允许为空");
    }

    CPointer<CNnLayer> sp;
    CObject::createObject(sp);
    sp->m_eMode = ENnLayerMode::EMODE_NONE;
    if(szMode) {
        std::string strMode(szMode);
        if( strMode == "batch" ) {
            sp->m_eMode = ENnLayerMode::EMODE_BATCH;
        }else if(strMode == "sequence") {
            sp->m_eMode = ENnLayerMode::EMODE_SEQUENCE;
        }else if( strMode == "rbatch" ) {
            sp->m_eMode = ENnLayerMode::EMODE_RBATCH;
        }else if(strMode == "rsequence") {
            sp->m_eMode = ENnLayerMode::EMODE_RSEQUENCE;
        }else{
            return sCtx.error("无效的层模式");
        }
    }

    sp->m_spUnit = spUnit;
    spLayer.setPtr(sp.getPtr());
    return sCtx.success();
}

int CNnLayer::getUnit(SNnUnit& spUnit) {
    spUnit = m_spUnit;
    return sCtx.success();
}

ENnLayerMode CNnLayer::getMode() {
    return m_eMode;
}


int CNnLayer::toArchive(const SArchive& ar) {
    //基础参数
    ar.visit("mode", m_eMode);
    ar.visitObject("unit", m_spUnit);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CNnLayer, CNnLayer::__getClassKey())