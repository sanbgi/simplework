#ifndef __SimpleWork_NN_CIdxFileReader_H__
#define __SimpleWork_NN_CIdxFileReader_H__

#include "nn.h"
#include "CActivator.h"

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;
using namespace SIMPLEWORK_NN_NAMESPACE;

class CIdxFileReader : public CObject, IData{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IData)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://IPipe
    const void* getDataPtr(unsigned int idType);
    unsigned int getDataType();

public://Factory
    static int createReader(const char* szFileName, SPipe& spPipe);
    static int readFile(const char* szFileName, SData& spData);
    static void highEndianToCPU(int nData, int nDataByte, unsigned char* pData);

protected:
    CIdxFileReader() {
    }

private:
    CTaker<unsigned char*> m_spBytes;
    PTensor m_tensor;
};

#endif//__SimpleWork_NN_CIdxFileReader_H__
