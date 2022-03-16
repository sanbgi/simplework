
#include "../device.h"
#include <map>
#include <string>
#include <fstream> 
#include <iostream>
#include <cuda.h>


using namespace sw;
using namespace std;

static SCtx sCtx("CCudaMemory");

class CCudaMemory : public CObject, IDeviceMemory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IDeviceMemory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

protected://CObject
    int __initialize(const PData* pData){
        return sCtx.success();
    }

private://IDeviceMemory
    int getSize() {
        return m_nSize;
    }

    int getDevice(SDevice& spDevice){
        spDevice = SDevice::cuda();
        return sCtx.success();
    }

    void* getData(const SDevice& spDevice){
        if( spDevice.getPtr() != SDevice::cuda().getPtr() ) {
            return nullptr;
        }
        //return m_sBuffer.get();
        return nullptr;
    }

    int getKernelMemory(SDeviceMemory& spKernelMemory){
        spKernelMemory.setPtr(this);
        return sCtx.success();
    }

    int toDevice(const SDevice& spDevice, SDeviceMemory& spMemory) {
        return sCtx.success();
    }

    int writeMemory(const SDeviceMemory& spMemory) {
        if(spMemory.getPtr() == this) {
            return sCtx.success();
        }

        /*
        if(spMemory.size() != m_nSize) {
            return sCtx.error("不能写入大小不一样的内存");
        }

        SDevice spDevice = spMemory.device();
        if( spDevice.getPtr() == SDevice::cpu().getPtr() ) {
            return writeMemory(m_nSize, spMemory.data(spDevice));
        }else if( spDevice.getPtr() == SDevice::Cuda().getPtr() ) {
            void* pSrc = spMemory.data(spDevice);
            if( pSrc == m_sBuffer.get() ) {
                return sCtx.success();
            }
            //
            //  如果都是Cuda设备，数据拷贝是不是可以不通过内存中转，直接拷贝？这里面涉
            //  及到需要内核函数执行，依赖性较强
            //
            return spDevice.memoryCopy(m_sBuffer.get(), 0, pSrc, 0, m_nSize);
        }
        
        CTaker<char*> spTaker(new char[m_nSize], [](char* pMemory){
            delete[] pMemory;
        });
        if( spMemory->readMemory(m_nSize, spTaker) != sCtx.success() ){
            return sCtx.error("读取数据源数据异常");
        }
        return writeMemory(m_nSize, spTaker);
        */
        return sCtx.success();
    }

    int writeMemory(int nSize, void* pData, int iOffset=0){
        /*
        if(m_sBuffer.get() == nullptr) {
            return sCtx.error("设备内存无效");
        }
        if( iOffset != 0 || nSize + iOffset > m_nSize) {
            return sCtx.error("设置内存超出了范围");
        }
        if( cl::copy(((unsigned char*)pData), ((unsigned char*)pData)+nSize, m_sBuffer) != CL_SUCCESS ) {
            return sCtx.error("Cuda内存拷贝错误");
        }
        */
        return sCtx.success();
    }

    int readMemory(int nSize, void* pData, int iOffset=0){
        /*
        if(m_sBuffer.get() == nullptr) {
            return sCtx.error("设备内存无效");
        }
        if( iOffset != 0 || nSize + iOffset > m_nSize) {
            return sCtx.error("设置内存超出了范围");
        }
        cl_int err;
        if( (err = cl::copy(m_sBuffer, ((unsigned char*)pData), ((unsigned char*)pData)+nSize )) != CL_SUCCESS ) {
            return sCtx.error("Cuda内存拷贝错误");
        }*/
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.device.CudaMemory"; }

private:
    int m_nSize;
    //cl::Buffer m_sBuffer;
};
SIMPLEWORK_FACTORY_AUTO_REGISTER(CCudaMemory, CCudaMemory::__getClassKey())
