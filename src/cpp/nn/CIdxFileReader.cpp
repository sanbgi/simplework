#include <iostream>
#include <fstream> 
#include <string>
#include "CIdxFileReader.h"

using namespace std;

static SCtx sCtx("CIdxFileReader");

int CIdxFileReader::pushData(const PData& rData, IVisitor<const PData&>* pReceiver) {
    //
    // 如果张量数据已经读完，则返回一个空张量
    //
    if(m_nTensor == 0) {
        return pReceiver->visit(CData<PTensor>(nullptr));
    }

    int nData = m_tensorTemplate.nData;
    int nSize = m_nEleByte;
    unsigned char pData[nSize];
    if( !m_file.read((char*)pData, nSize) ){
        return sCtx.Error(string("IDX文件数据数据不完整，读取失败").c_str());
    }
    highEndianToCPU(nData, m_nEleByte, pData);

    m_nTensor--;
    PTensor tensor = m_tensorTemplate;
    tensor.pData = pData;
    return pReceiver->visit(CData<PTensor>(tensor));
}

int CIdxFileReader::createReader(const char* szFileName, SPipe& spPipe) {

    CPointer<CIdxFileReader> spReader;
    CObject::createObject(spReader);
    
    //
    // 读取文件头信息
    //
    ifstream& idxFile = spReader->m_file;
    idxFile.open(szFileName, ios_base::binary);
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
            idType = SData::getTypeIdentifier<CBasicType<unsigned char>>();
            nEleByte = 1;
            break;

        case 0x09:  //signed byte
            idType = SData::getTypeIdentifier<CBasicType<char>>();
            nEleByte = 1;
            break;

        case 0x0B:  //short (2 bytes)
            idType = SData::getTypeIdentifier<CBasicType<short>>();
            nEleByte = 2;
            break;

        case 0x0C:  //int (4 bytes)
            idType = SData::getTypeIdentifier<CBasicType<int>>();
            nEleByte = 4;
            break;

        case 0x0D:  //float (4 bytes)
            idType = SData::getTypeIdentifier<CBasicType<float>>();
            nEleByte = 4;
            break;

        case 0x0E:  //double (8 bytes)
            idType = SData::getTypeIdentifier<CBasicType<double>>();
            nEleByte = 8;
            break;

        default:
            return sCtx.Error(string(string("IDX文件数据类型未知，文件名: ") + szFileName).c_str());
    }

    int nDims = headerArray[3];
    spReader->m_spDims.take(new int[nDims], [](int* ptr) {
        delete[] ptr;
    });
    int* pDimSize = spReader->m_spDims;
    if( !idxFile.read((char*)pDimSize, sizeof(int)*(nDims+1)) ) {
        return sCtx.Error(string(string("IDX文件数据数据不完整，读取维度信息失败，文件名: ") + szFileName).c_str());
    }
    highEndianToCPU(nDims, sizeof(int), (unsigned char*)pDimSize);

    int nData = 1;
    for(int i=1; i<nDims; i++) {
        nData *= pDimSize[i];
    }
    pDimSize[nDims] = 1;

    //
    //  更新每次读取的张量模板参数。读取数据降维，第一个维度是张量数据的个数
    //
    PTensor& tensorTemplate = spReader->m_tensorTemplate;
    tensorTemplate.idType = idType;
    tensorTemplate.nDims = nDims>1?nDims-1:1;
    tensorTemplate.pDimSizes = pDimSize+1;  
    tensorTemplate.nData = nData;
    spReader->m_nTensor = pDimSize[0];
    spReader->m_nEleByte = nEleByte;
    spPipe.setPtr(spReader.getPtr());
    return sCtx.Success();
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

    //
    // 读取文件头信息
    //
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
            idType = SData::getTypeIdentifier<CBasicType<unsigned char>>();
            nEleByte = 1;
            break;

        case 0x09:  //signed byte
            idType = SData::getTypeIdentifier<CBasicType<char>>();
            nEleByte = 1;
            break;

        case 0x0B:  //short (2 bytes)
            idType = SData::getTypeIdentifier<CBasicType<short>>();
            nEleByte = 2;
            break;

        case 0x0C:  //int (4 bytes)
            idType = SData::getTypeIdentifier<CBasicType<int>>();
            nEleByte = 4;
            break;

        case 0x0D:  //float (4 bytes)
            idType = SData::getTypeIdentifier<CBasicType<float>>();
            nEleByte = 4;
            break;

        case 0x0E:  //double (8 bytes)
            idType = SData::getTypeIdentifier<CBasicType<double>>();
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

    //
    // 读取张量数据，并保存为CIdxFileData
    //
    class CIdxFileData : public CObject, public IData{
        SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
            SIMPLEWORK_INTERFACE_ENTRY(IData)
        SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)
    private://IPipe
        const void* getDataPtr(unsigned int idType){
            if(idType != SData::getTypeIdentifier<PTensor>()) {
                return nullptr;
            }
            return &m_tensor;
        }
        unsigned int getDataType() {
            return SData::getTypeIdentifier<PTensor>();
        }
    public:
        CTaker<unsigned char*> m_spBytes;
        PTensor m_tensor;
    };

    int nData = 1;
    for(int i=0; i<nDims; i++) {
        nData *= pDimSize[i];
    }

    int nSize = nDims*sizeof(int) + nData*nEleByte;
    CPointer<CIdxFileData> spIdxData;
    CObject::createObject(spIdxData);
    spIdxData->m_spBytes.take(new unsigned char[nSize], [](unsigned char* ptr) {
        delete[] ptr;
    });

    unsigned char* pBuf = spIdxData->m_spBytes;
    unsigned char* pData = pBuf+sizeof(int)*nDims;
    if( !idxFile.read((char*)pData, nData*nEleByte) ){
        return sCtx.Error(string(string("IDX文件数据数据不完整，读取数据信息失败，文件名: ") + szFileName).c_str());
    }
    highEndianToCPU(nData, nEleByte, pData);

    memcpy(pBuf, pDimSize, sizeof(int)*nDims);
    PTensor& tensor = spIdxData->m_tensor;
    tensor.idType = idType;
    tensor.nDims = nDims;
    tensor.pDimSizes = (int*)pBuf;
    tensor.nData = nData;
    tensor.pData = pData;
    spData.setPtr(spIdxData.getPtr());
    return sCtx.Success();
}