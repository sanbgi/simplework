#ifndef __SimpleWork_NN_CIdxFileReader_H__
#define __SimpleWork_NN_CIdxFileReader_H__

#include "nn.h"
#include "CActivator.h"
#include <fstream> 

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;
using namespace SIMPLEWORK_NN_NAMESPACE;

class CIdxFileReader : public CObject, IPipe{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IPipe)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int pushData(const PData& rData, IVisitor<const PData&>* pReceiver);

public://Factory
    static int createReader(const char* szFileName, SPipe& spPipe);
    static int readFile(const char* szFileName, STensor& spData);
    static void highEndianToCPU(int nData, int nDataByte, unsigned char* pData);

protected:
    CIdxFileReader() {
    }

private:
    std::ifstream m_file;
    CTaker<int*> m_spDims;
    PTensor m_tensorTemplate;
    int m_nTensor;
    int m_nEleByte;
};

#endif//__SimpleWork_NN_CIdxFileReader_H__
