#include "CIoBinaryArWriter.h"

using std::string;

SCtx CIoBinaryArWriter::sCtx("CIoBinaryArWriter");
int CIoBinaryArWriter::visit(const char* szName, unsigned int idType, int nByte, void* pByte, int nMinVer, int nMaxVer){
    if(m_nEleVer >= nMinVer && m_nEleVer <= nMaxVer ) {
        m_stream.write((const char*)pByte, nByte);
    }
    return sCtx.success();
}

int CIoBinaryArWriter::visit(const char* szName, IArrayVisitee* pVisitee, int nMinVer, int nMaxVer) {
    if(m_nEleVer >= nMinVer && m_nEleVer <= nMaxVer ) {
        int nEle = pVisitee->size();
        m_stream.write((const char*)&nEle, sizeof(int));
        if(nEle > 0)
            m_stream.write((const char*)pVisitee->data(), nEle*pVisitee->getElementBytes());
    }
    return sCtx.success();
}

int CIoBinaryArWriter::visit(const char* szName, SIoArchivable& spVisitee, int nMinVer, int nMaxVer){
    if(m_nEleVer >= nMinVer && m_nEleVer <= nMaxVer ) {
        return saveEle(spVisitee);
    }
    return sCtx.success();
}

int CIoBinaryArWriter::visit(const char* szName, IObjectArrayVisitee* pVisitee, int nMinVer, int nMaxVer) {
    if(m_nEleVer >= nMinVer && m_nEleVer <= nMaxVer ) {
        int nEles = pVisitee->size();
        m_stream.write((const char*)&nEles, sizeof(int));
        for(int i=0; i<nEles; i++) {
            SIoArchivable spVisitee;
            pVisitee->getEleAt(i, spVisitee);
            saveEle(spVisitee);
        }
    }
    return sCtx.success();
}

int CIoBinaryArWriter::saveArchive(const char* szFileName, const SIoArchivable& spAr) {
    CPointer<CIoBinaryArWriter> spOut;
    CObject::createObject(spOut);
    spOut->m_stream.open(szFileName, ios_base::binary);
    if( !spOut->m_stream.is_open() ) {
        return sCtx.error(string(string("无法打开保存文件，文件名: ") + szFileName).c_str());
    }

    int nFileVer = 20220112;
    spOut->m_stream.write((const char*)&nFileVer, sizeof(int));

    if( spOut->saveEle(spAr) != sCtx.success() ) {
        return sCtx.error("序列化文件失败");
    }

    spOut->m_stream.close();
    return sCtx.success();
}

void CIoBinaryArWriter::enterElement(SIoArchivable& spEle) {
    m_arrEles.push_back(spEle);
    m_nEleVer = spEle->getVer();
}

void CIoBinaryArWriter::leaveElement(SIoArchivable& spEle) {
    m_arrEles.pop_back();
    if(m_arrEles.size() > 0) {
        m_nEleVer = (*m_arrEles.rend())->getVer();
    }else{
        m_nEleVer = -1;
    }
}

void CIoBinaryArWriter::saveString(string str) {
    //+1的目的时把末尾的0也一并序列化
    int nChar = str.length()+1;
    const char* pChar = str.c_str();
    m_stream.write((const char*)&nChar, sizeof(int));
    //如果字符串为空，则无需序列化内容了
    if(nChar>1)
        m_stream.write(pChar, nChar);
}

int CIoBinaryArWriter::saveEle(SIoArchivable spVisitee) {
    string classKey;
    if( !spVisitee ) {
        saveString(classKey);
        return sCtx.success();
    }

    classKey = spVisitee->getClassKey();
    saveString(classKey);

    int nVer = spVisitee->getVer();
    m_stream.write((const char*)&nVer, sizeof(int));

    int retCode = sCtx.success();
    enterElement(spVisitee);
    retCode = spVisitee->toVisit(SIoArchive((IIoArchive*)this));
    leaveElement(spVisitee);
    return retCode;
}