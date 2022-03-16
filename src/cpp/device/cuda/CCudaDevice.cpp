
#include "../device.h"
#include <map>
#include <string>
#include <fstream> 
#include <iostream>
#include <cuda.h>


using namespace sw;
using namespace std;

static SCtx sCtx("CCudaDevice");

class CCudaDevice : public CObject, public IDevice{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IDevice)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

protected://CObject
    int __initialize(const PData* pData){
        if( cuInit(0) != CUDA_SUCCESS ) {
            return sCtx.error("CUDA 初始化失败");
        } 
        
        return sCtx.success();
    }

private://IDevice
    int createKernelMemory(SDeviceMemory& spDeviceMemory, int nSize, void* pInitData = nullptr){
        spDeviceMemory = SObject::createObject("sw.device.CudaMemory", CData<PMemory>({nSize, pInitData}));
        return spDeviceMemory ? sCtx.success() : sCtx.error("创建内存失败");
    }

    int createKernelMemory(SDeviceMemory& spDeviceMemory, const SDeviceMemory& spMemory){
        SDevice spDevice = spMemory.device();
        if(spDevice.getPtr() == this) {
            spDeviceMemory = spMemory;
            return sCtx.success();
        }

        if(spDevice.getPtr() == SDevice::cpu().getPtr()) {
            void* pData = spMemory.data(spDevice);
            if( pData == nullptr ) {
                return sCtx.error("无法获取内存指针");
            }
            return createKernelMemory(spDeviceMemory, spMemory.size(), pData);
        }

        //如果不是CPU内存，则需要内存拷贝到CPU内存，作为中转
        int size = spMemory.size();
        CTaker<char*> spTaker(new char[size], [](char* pMemory){
            delete[] pMemory;
        });

        if( !spMemory || spMemory->readMemory(size, spTaker) != sCtx.success() ) {
            return sCtx.error("创建内存所对应的原始内存无效");
        }
        return createKernelMemory(spDeviceMemory, size, spTaker);
    }

    int runKernel(  const PRuntimeKey& kernelKey, 
                    int nArgs, 
                    PKernalVariable pArgs[], 
                    int nRanges = 0, 
                    int pRanges[]=nullptr) {
        return sCtx.success();
    }

private:
    /*
    static int getKernel(const PRuntimeKey& kernelKey, cl::Kernel& kernelFunc) {

        static std::map<PID,cl::Kernel> sMapKernels;
        static std::map<string,cl::Kernel> sNamedKernels;

        auto it = sMapKernels.find(kernelKey.runtimeId);
        if( it != sMapKernels.end() ) {
            kernelFunc = it->second;
            return sCtx.success();
        }

        if(kernelKey.runtimeKey == nullptr) {
            return sCtx.error("内核参数错误");
        }

        auto itNamedKernel = sNamedKernels.find(kernelKey.runtimeKey);
        if( itNamedKernel != sNamedKernels.end() ) {
            sMapKernels[PRuntimeKey(kernelKey.runtimeKey).runtimeId] = itNamedKernel->second;
            kernelFunc = itNamedKernel->second;
            return sCtx.success();
        }

        string kernalName = kernelKey.runtimeKey;
        auto iProgramName = kernalName.rfind('.');
        if( iProgramName <= 0 && iProgramName >= kernalName.length() - 1) {
            return sCtx.error((string("无效的内核名，")+kernalName).c_str());
        }

        //cl::Program clProgram;
        //if( getProgram(kernalName.substr(0,iProgramName), clProgram) != sCtx.success() ) {
        //    return sCtx.error((string("内核创建错误，内核名：")+kernelKey.runtimeKey).c_str());
        //}
        
        //cl::Kernel kernel = cl::Kernel(clProgram, kernalName.substr(iProgramName+1).c_str());
        //if(kernel.get() == nullptr) {
        //    return sCtx.error((string("创建内核错误，内核名：")+kernelKey.runtimeKey).c_str());
        //}

        //kernelFunc = kernel;
        //sMapKernels[PRuntimeKey(kernelKey.runtimeKey).runtimeId] = kernel;
        //sNamedKernels[kernalName] = kernel;
        return sCtx.success();
    }
    */

    /*
    static int getProgram(string name, cl::Program& program) {
        
        static std::map<string,cl::Program> sMapPrograms;
        auto itProgram = sMapPrograms.find(name);
        if( itProgram != sMapPrograms.end() ) {
            program = itProgram->second;
            return sCtx.success();
        }

        auto ipos = name.rfind(".");
        if( ipos == string::npos || ipos < 1 || ipos >= name.length() - 1) {
            return sCtx.error(("内核程序名错误, 名字："+name).c_str());
        }
        auto filename = "Cuda/"+name.substr(0,ipos)+"/"+name.substr(ipos+1)+".cl";
        ifstream in(filename.c_str(), std::ios_base::binary);
        if(!in.good()) {
            return sCtx.error(("读取内核文件错误，文件名："+filename).c_str());
        }

        // get file length
        in.seekg(0, std::ios_base::end);
        size_t length = in.tellg();
        in.seekg(0, std::ios_base::beg);

        // read program source
        std::vector<char> data(length + 1);
        in.read(data.data(), length);
        data[length] = 0;

        cl::Program clProgram(data.data());
        try {
            cl_int ret = clProgram.build("-cl-std=CL2.0");
            if( ret != CL_SUCCESS) {
                return sCtx.error(("编译内核文件错误，文件名："+filename).c_str());
            }
        }
        catch (...) {
            // Print build info for all devices
            cl_int buildErr = CL_SUCCESS;
            auto buildInfo = clProgram.getBuildInfo<CL_PROGRAM_BUILD_LOG>(&buildErr);
            for (auto &pair : buildInfo) {
                std::cerr << pair.second << std::endl << std::endl;
            }
            return sCtx.error(("编译内核文件错误，文件名："+filename).c_str());
        }
        program = clProgram;
        sMapPrograms[name] = program;
        
        return sCtx.success();
    }*/

public://Factory
    static const char* __getClassKey() { return "sw.device.cuda.CudaDevice"; }

};
SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CCudaDevice, CCudaDevice::__getClassKey())