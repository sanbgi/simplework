#include <iostream>
#include <fstream> 
#include <string>
#include "CIdxFileReader.h"

using namespace std;

static SCtx sCtx("CIdxFileReader");

int CIdxFileReader::push(const STensor& spIn, STensor& spOut) {
    if(m_nTensor == 0) {
        return sCtx.error("文件读取完毕");
    }

    //
    // 如果张量数据已经读完，则返回一个空张量
    //
    if(spIn->getDataSize() != 1 || spIn->getDataType() != CBasicData<int>::getStaticType() ) {
        return sCtx.error("IDX文件读取的输入张量需要是一个整数，代表要读取多少张量");
    }

    int nRead = spIn.data<int>()[0];
    if( m_spDimension ) {
        if( *m_spDimension.data() != nRead ) {
            m_spDimension.release();
        }
    }

    if( !m_spDimension ) {
        int* pDimSizes = m_spDims;
        pDimSizes[0] = nRead;
        if( SDimension::createDimension(m_spDimension, m_nDims, pDimSizes) != sCtx.success() ) {
            return sCtx.error("创建输出维度向量失败");
        }
    }

    if( STensor::createTensor(spOut, m_spDimension, m_iEleType, m_nEleSize*nRead) != sCtx.success() ) {
        return sCtx.error("创建输出维度向量失败");
    }

    int nData = m_nEleSize*nRead;
    int nSize = m_nEleByte*nData;
    unsigned char* pData = (unsigned char*)spOut.data();
    if( !m_file.read((char*)pData, nSize) ){
        return sCtx.error(string("IDX文件数据数据不完整，读取失败").c_str());
    }
    highEndianToCPU(nData, m_nEleByte, pData);

    m_nTensor-=nRead;
    return sCtx.success();
}

int CIdxFileReader::createReader(const char* szFileName, SNnPipe& spPipe) {

    CPointer<CIdxFileReader> spReader;
    CObject::createObject(spReader);
    
    //
    // 读取文件头信息
    //
    ifstream& idxFile = spReader->m_file;
    idxFile.open(szFileName, ios_base::binary);
    if( !idxFile.is_open() ) {
        return sCtx.error(string(string("读取IDX文件失败，文件名: ") + szFileName).c_str());
    }

    unsigned char headerArray[4];
    if( !idxFile.read((char*)headerArray, 4) ) {
        return sCtx.error(string(string("无法读取IDX文件头信息，文件名: ") + szFileName).c_str());
    }

    if( headerArray[0] != 0 || headerArray[1] != 0) {
        return sCtx.error(string(string("IDX文件格式错误，文件名: ") + szFileName).c_str());
    }

    int nEleByte = 0;
    PDATATYPE idType = 0;
    switch(headerArray[2]) {
        case 0x08:  // unsigned byte
            idType = CBasicData<unsigned char>::getStaticType();
            nEleByte = 1;
            break;

        case 0x09:  //signed byte
            idType = CBasicData<char>::getStaticType();
            nEleByte = 1;
            break;

        case 0x0B:  //short (2 bytes)
            idType = CBasicData<short>::getStaticType();
            nEleByte = 2;
            break;

        case 0x0C:  //int (4 bytes)
            idType = CBasicData<int>::getStaticType();
            nEleByte = 4;
            break;

        case 0x0D:  //float (4 bytes)
            idType = CBasicData<float>::getStaticType();
            nEleByte = 4;
            break;

        case 0x0E:  //double (8 bytes)
            idType = CBasicData<double>::getStaticType();
            nEleByte = 8;
            break;

        default:
            return sCtx.error(string(string("IDX文件数据类型未知，文件名: ") + szFileName).c_str());
    }

    int nDims = headerArray[3];
    spReader->m_spDims.take(new int[nDims], [](int* ptr) {
        delete[] ptr;
    });
    int* pDimSize = spReader->m_spDims;
    if( !idxFile.read((char*)pDimSize, sizeof(int)*nDims) ) {
        return sCtx.error(string(string("IDX文件数据数据不完整，读取维度信息失败，文件名: ") + szFileName).c_str());
    }
    highEndianToCPU(nDims, sizeof(int), (unsigned char*)pDimSize);

    int nData = 1;
    for(int i=1; i<nDims; i++) {
        nData *= pDimSize[i];
    }
    spReader->m_nTensor = pDimSize[0];
    spReader->m_nEleByte = nEleByte;
    spReader->m_nEleSize = nData;
    spReader->m_nDims = nDims;
    spReader->m_iEleType = idType;
    spPipe.setPtr(spReader.getPtr());
    return sCtx.success();
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

int CIdxFileReader::readFile(const char* szFileName, STensor& spData) {

    //
    // 读取文件头信息
    //
    ifstream idxFile(szFileName, ios_base::binary);
    if( !idxFile.is_open() ) {
        return sCtx.error(string(string("读取IDX文件失败，文件名: ") + szFileName).c_str());
    }

    unsigned char headerArray[4];
    if( !idxFile.read((char*)headerArray, 4) ) {
        return sCtx.error(string(string("无法读取IDX文件头信息，文件名: ") + szFileName).c_str());
    }

    if( headerArray[0] != 0 || headerArray[1] != 0) {
        return sCtx.error(string(string("IDX文件格式错误，文件名: ") + szFileName).c_str());
    }

    int nEleByte = 0;
    PDATATYPE idType = 0;
    switch(headerArray[2]) {
        case 0x08:  // unsigned byte
            idType = CBasicData<unsigned char>::getStaticType();
            nEleByte = 1;
            break;

        case 0x09:  //signed byte
            idType = CBasicData<char>::getStaticType();
            nEleByte = 1;
            break;

        case 0x0B:  //short (2 bytes)
            idType = CBasicData<short>::getStaticType();
            nEleByte = 2;
            break;

        case 0x0C:  //int (4 bytes)
            idType = CBasicData<int>::getStaticType();
            nEleByte = 4;
            break;

        case 0x0D:  //float (4 bytes)
            idType = CBasicData<float>::getStaticType();
            nEleByte = 4;
            break;

        case 0x0E:  //double (8 bytes)
            idType = CBasicData<double>::getStaticType();
            nEleByte = 8;
            break;

        default:
            return sCtx.error(string(string("IDX文件数据类型未知，文件名: ") + szFileName).c_str());
    }

    int nDims = headerArray[3];
    vector<int> pDimSize(nDims);
    if( !idxFile.read((char*)pDimSize.data(), sizeof(int)*nDims) ) {
        return sCtx.error(string(string("IDX文件数据数据不完整，读取维度信息失败，文件名: ") + szFileName).c_str());
    }
    highEndianToCPU(nDims, sizeof(int), (unsigned char*)pDimSize.data());

    int nData = 1;
    for(int i=0; i<nDims; i++) {
        nData *= pDimSize[i];
    }

    SDimension spDimension;
    if( SDimension::createDimension(spDimension, nDims, pDimSize.data()) != sCtx.success() ) {
        return sCtx.error("创建张量的维度向量失败");
    }
    STensor spTensor;
    if( STensor::createTensor(spTensor, spDimension, idType, nData) != sCtx.success() ) {
        return sCtx.error("创建张量失败");
    }

    int nSize = nDims*sizeof(int) + nData*nEleByte;
    unsigned char* pData = spTensor.data<unsigned char>();
    if( !idxFile.read((char*)pData, nData*nEleByte) ){
        return sCtx.error(string(string("IDX文件数据数据不完整，读取数据信息失败，文件名: ") + szFileName).c_str());
    }
    highEndianToCPU(nData, nEleByte, pData);
    spData = spTensor;
    return sCtx.success();
}