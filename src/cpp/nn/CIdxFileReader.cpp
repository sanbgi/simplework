#include <iostream>
#include <fstream> 
#include <string>
#include "CIdxFileReader.h"

using namespace std;

static SCtx sCtx("CIdxFileReader");

const void* CIdxFileReader::getDataPtr(unsigned int idType) {
    if(idType != SData::getStructTypeIdentifier<PTensor>()) {
        return nullptr;
    }
    return &m_tensor;
}

unsigned int CIdxFileReader::getDataType() {
    return SData::getStructTypeIdentifier<PTensor>();
}

void CIdxFileReader::highEndianToCPU(int nData, int nDataByte, unsigned char* pData) {
    typedef union uEndianTest{
        struct
        {
            bool flittle_endian;
            bool fill[3];
        };
        long value;
    }EndianTest;
    static const EndianTest __Endian_Test__ = { (long)1 };
    const bool platform_little_endian = __Endian_Test__.flittle_endian;
    if(platform_little_endian && nDataByte > 1) {
        for(int i=0; i<nData; i++) {
            for(int j=0; j<nDataByte/2; j++) {
                unsigned char c = pData[i*4+j];
                pData[i*4+j] = pData[i*4+nDataByte-j-1];
                pData[i*4+nDataByte-j-1] = c;
            }
        }
    }
}

int CIdxFileReader::readFile(const char* szFileName, SData& spData) {

    ifstream idxFile(szFileName, ios_base::binary);
    if( !idxFile.is_open() ) {
        return sCtx.Error(string(string("读取IDX文件失败，文件名: ") + szFileName).c_str());
    }

    unsigned char headerArray[4];
    if( !idxFile.read((char*)headerArray, 4) ) {
        return sCtx.Error(string(string("无法读取IDX文件头信息，文件名: ") + szFileName).c_str());
    }

    if( headerArray[0] != 0 || headerArray[1] != 0) {
        return sCtx.Error(string(string("IDX文件格式错误，文件名: ") + szFileName).c_str());
    }

    int nEleByte = 0;
    unsigned int idType = 0;
    switch(headerArray[2]) {
        case 0x08:  // unsigned byte
            idType = SData::getBasicTypeIdentifier<unsigned char>();
            nEleByte = 1;
            break;

        case 0x09:  //signed byte
            idType = SData::getBasicTypeIdentifier<char>();
            nEleByte = 1;
            break;

        case 0x0B:  //short (2 bytes)
            idType = SData::getBasicTypeIdentifier<short>();
            nEleByte = 2;
            break;

        case 0x0C:  //int (4 bytes)
            idType = SData::getBasicTypeIdentifier<int>();
            nEleByte = 4;
            break;

        case 0x0D:  //float (4 bytes)
            idType = SData::getBasicTypeIdentifier<float>();
            nEleByte = 4;
            break;

        case 0x0E:  //double (8 bytes)
            idType = SData::getBasicTypeIdentifier<double>();
            nEleByte = 8;
            break;

        default:
            return sCtx.Error(string(string("IDX文件数据类型未知，文件名: ") + szFileName).c_str());
    }

    int nDims = headerArray[3];
    int pDimSize[nDims];
    if( !idxFile.read((char*)pDimSize, sizeof(int)*nDims) ) {
        return sCtx.Error(string(string("IDX文件数据数据不完整，读取维度信息失败，文件名: ") + szFileName).c_str());
    }
    highEndianToCPU(nDims, sizeof(int), (unsigned char*)pDimSize);

    int nData = 1;
    for(int i=0; i<nDims; i++) {
        nData *= pDimSize[i];
    }

    int nSize = nDims*sizeof(int) + nData*nEleByte;
    CPointer<CIdxFileReader> spFileReader;
    CObject::createObject(spFileReader);
    spFileReader->m_spBytes.take(new unsigned char[nSize], [](unsigned char* ptr) {
        delete[] ptr;
    });

    unsigned char* pBuf = spFileReader->m_spBytes;
    unsigned char* pData = pBuf+sizeof(int)*nDims;
    if( !idxFile.read((char*)pData, nData*nEleByte) ){
        return sCtx.Error(string(string("IDX文件数据数据不完整，读取数据信息失败，文件名: ") + szFileName).c_str());
    }
    highEndianToCPU(nData, nEleByte, pData);

    memcpy(pBuf, pDimSize, sizeof(int)*nDims);
    PTensor& tensor = spFileReader->m_tensor;
    tensor.idType = idType;
    tensor.nDims = nDims;
    tensor.pDimSizes = (int*)pBuf;
    tensor.nData = nData;
    tensor.pData = pData;
    spData.setPtr(spFileReader.getPtr());
    return sCtx.Success();
}