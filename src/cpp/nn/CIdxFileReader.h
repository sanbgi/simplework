#ifndef __SimpleWork_NN_CIdxFileReader_H__
#define __SimpleWork_NN_CIdxFileReader_H__

#include "nn.h"
#include "CActivator.h"
#include <fstream> 

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;
using namespace SIMPLEWORK_NN_NAMESPACE;

class CIdxFileReader : public CObject, INeuralPipe{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INeuralPipe)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int push(const STensor& spIn, STensor& spOut);

public://Factory
    static int createReader(const char* szFileName, SNeuralPipe& spPipe);
    static int readFile(const char* szFileName, STensor& spData);
    static void highEndianToCPU(int nData, int nDataByte, unsigned char* pData);

private:
    std::ifstream m_file;
    int m_nDims;
    int m_nEleSize;
    int m_nEleByte;
    unsigned int m_iEleType;
    CTaker<int*> m_spDims;
    STensor m_spDimVector;
    int m_nTensor;

};

#endif//__SimpleWork_NN_CIdxFileReader_H__
