#ifndef __SimpleWork_IO_CIoBinaryArWriter_H__
#define __SimpleWork_IO_CIoBinaryArWriter_H__

#include "io.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace sw;
using namespace sw;
using namespace std;

class CIoBinaryArWriter : public CObject, public IIoArchive {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IIoArchive)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://IIoArchive
    bool isReading() { return false; }
    int visit(const char* szName, unsigned int idType, int nByte, void* pByte, int nMinVer=0, int nMaxVer=99999999);
    int visitArray(const char* szName, IArrayVisitee* pVisitee, int nMinVer=0, int nMaxVer=99999999);
    int visitObject(const char* szName, SIoArchivable& spVisitee, int nMinVer=0, int nMaxVer=999999999);
    int visitObjectArray(const char* szName, IObjectArrayVisitee* pVisitee, int nMinVer=0, int nMaxVer=999999999);

private://
    void enterElement(SIoArchivable& spEle);
    void leaveElement(SIoArchivable& spEle);
    void saveString(string str);
    int saveEle(SIoArchivable sp);

public:
    static int saveArchive(const char* szFileName, const SIoArchivable& spAr);

public:
    CIoBinaryArWriter(){
        m_nEleVer = -1;
    }

private:
    static SCtx sCtx;
    std::ofstream m_stream;
    int m_nEleVer;
    std::vector<SIoArchivable> m_arrEles;
};

#endif//__SimpleWork_IO_CIoBinaryArWriter_H__