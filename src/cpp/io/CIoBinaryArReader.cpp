#include "CIoBinaryArReader.h"

SCtx CIoBinaryArReader::sCtx("CIoBinaryArReader");
int CIoBinaryArReader::visit(const char* szName, unsigned int idType, int nByte, void* pByte, int nMinVer, int nMaxVer){
    if(m_nEleVer >= nMinVer && m_nEleVer <= nMaxVer ) {
        m_stream.read((char*)pByte, nByte);
    }
    return sCtx.success();
}

int CIoBinaryArReader::visitArray(const char* szName, IArrayVisitee* pVisitee, int nMinVer, int nMaxVer) {
    if(m_nEleVer >= nMinVer && m_nEleVer <= nMaxVer ) {
        int nBuffer;
        m_stream.read((char*)&nBuffer, sizeof(int));
        if(nBuffer) {
            char sBuffer[nBuffer];
            m_stream.read(sBuffer, nBuffer);
            pVisitee->setArray(nBuffer / pVisitee->getElementBytes(), (const void*)sBuffer);
        }
    }
    return sCtx.success();
}

int CIoBinaryArReader::visitObject(const char* szName, SIoArchivable& spVisitee, int nMinVer, int nMaxVer){
    if(m_nEleVer >= nMinVer && m_nEleVer <= nMaxVer ) {
        return loadEle(spVisitee);
    }
    return sCtx.success();
}

int CIoBinaryArReader::visitObjectArray(const char* szName, IObjectArrayVisitee* pVisitee, int nMinVer, int nMaxVer) {
    if(m_nEleVer >= nMinVer && m_nEleVer <= nMaxVer ) {
        int nEles;
        m_stream.read((char*)&nEles, sizeof(int));
        for(int i=0; i<nEles; i++) {
            SIoArchivable spVisitee;
            if( loadEle(spVisitee) != sCtx.success() ) {
                return sCtx.error("读取对象数据失败");
            }
            pVisitee->addEle(spVisitee);
        }
    }
    return sCtx.success();
}

int CIoBinaryArReader::loadArchive(const char* szFileName, SIoArchivable& spAr){
    CPointer<CIoBinaryArReader> spIn;
    CObject::createObject(spIn);
    spIn->m_stream.open(szFileName, ios_base::binary);
    if( !spIn->m_stream.is_open() ) {
        return sCtx.error(string(string("无法打开保存文件，文件名: ") + szFileName).c_str());
    }

    int nFileVer;
    spIn->m_stream.read((char*)&nFileVer, sizeof(int));
    if(nFileVer != 20220112) {
        return sCtx.error(string(string("文件格式错误，无法打开，文件名: ") + szFileName).c_str());
    }

    if( spIn->loadEle(spAr) != sCtx.success() ) {
        return sCtx.error("序列化文件失败");
    }

    spIn->m_stream.close();
    return sCtx.success();
}

void CIoBinaryArReader::enterElement(int nVer) {
    m_arrVers.push_back(nVer);
    m_nEleVer = nVer;
}

void CIoBinaryArReader::leaveElement() {
    m_arrVers.pop_back();
    if(m_arrVers.size() > 0) {
        m_nEleVer = *m_arrVers.rend();
    }else{
        m_nEleVer = -1;
    }
}

int CIoBinaryArReader::loadString(string& str) {
    //+1的目的时把末尾的0也一并序列化
    int nChar;
    m_stream.read((char*)&nChar, sizeof(int));

    //如果字符串为空，则无需序列化内容了
    if(nChar>1) {
        char szStr[nChar];
        m_stream.read(szStr, nChar);
        str = szStr;
    }
    return sCtx.success();
}

int CIoBinaryArReader::loadEle(SIoArchivable& spVisitee) {
    string classKey;
    if( loadString(classKey) != sCtx.success() ){
        return sCtx.error();
    }

    if(classKey.length() == 0) {
        return sCtx.success();
    }

    SObject spObj;
    SObject::createObject(classKey.c_str(), spObj);
    if( !spObj ) {
        return sCtx.error(string("创建对象失败，类型:" + classKey).c_str());
    }

    spVisitee = spObj;
    if( !spVisitee ) {
        return sCtx.error(string("类型没有实现IIoArchivable接口，无法序列化，类型:" + classKey).c_str());
    }

    int nVer;
    m_stream.read((char*)&nVer, sizeof(int));

    int retCode = sCtx.success();
    enterElement(nVer);
    retCode = spVisitee->toArchive(SIoArchive((IIoArchive*)this));
    leaveElement();
    return retCode;
}