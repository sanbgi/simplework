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

class CIoBinaryArWriter : public CObject, public IArchive {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IArchive)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://IArchive
    bool isReading() { return false; }
    int arBlock(const char* szName, PDATATYPE idType, int nByte, void* pByte, int nMinVer=0, int nMaxVer=99999999);
    int arBlockArray(const char* szName, IArrayVisitee* pVisitee, int nMinVer=0, int nMaxVer=99999999);
    int arObject(const char* szName, SArchivable& spVisitee, int nMinVer=0, int nMaxVer=999999999);
    int arObjectArray(const char* szName, IObjectArrayVisitee* pVisitee, int nMinVer=0, int nMaxVer=999999999);

private://
    void enterElement(SArchivable& spEle);
    void leaveElement(SArchivable& spEle);
    void saveString(string str);
    int saveEle(SArchivable sp);

public:
    static int saveArchive(const char* szFileName, const SArchivable& spAr);

public:
    CIoBinaryArWriter(){
        m_nEleVer = -1;
    }

private:
    static SCtx sCtx;
    std::ofstream m_stream;
    int m_nEleVer;
    std::vector<SArchivable> m_arrEles;
};

#endif//__SimpleWork_IO_CIoBinaryArWriter_H__