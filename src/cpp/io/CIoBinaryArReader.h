#ifndef __SimpleWork_IO_CIoBinaryArReader_H__
#define __SimpleWork_IO_CIoBinaryArReader_H__

#include "io.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace sw;
using namespace sw;
using namespace std;

class CIoBinaryArReader : public CObject, public IIoArchive {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IIoArchive)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://IIoArchive
    bool isReading() { return true; }
    int visit(const char* szName, unsigned int idType, int nByte, void* pByte, int nMinVer=0, int nMaxVer=99999999);
    int visit(const char* szName, IArrayVisitee* pVisitee, int nMinVer=0, int nMaxVer=99999999);
    int visit(const char* szName, SIoArchivable& spVisitee, int nMinVer=0, int nMaxVer=999999999);
    int visit(const char* szName, IObjectArrayVisitee* pVisitee, int nMinVer=0, int nMaxVer=999999999);

public:
    void enterElement(int nVer);
    void leaveElement();
    int loadString(string& str);
    int loadEle(SIoArchivable& sp);

public:
    static int loadArchive(const char* szFileName, SIoArchivable& spAr);

public:
    CIoBinaryArReader(){
        m_nEleVer = -1;
    }

private:
    static SCtx sCtx;
    std::ifstream m_stream;
    int m_nEleVer;
    std::vector<int> m_arrVers;
};

#endif//__SimpleWork_IO_CIoBinaryArReader_H__