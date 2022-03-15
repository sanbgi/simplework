
#include "../device.h"
#include <map>
#include <string>

using namespace sw;
using namespace std;

static SCtx sCtx("CCpuDevice");
class CCpuDevice : public CObject, public IDevice{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IDevice)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://IDevice
    int createKernelMemory(SDeviceMemory& spKernelMemory, int nSize, void* pData = nullptr){
        spKernelMemory = SObject::createObject("sw.device.CpuMemory", CData<PMemory>({nSize, pData}));
        return spKernelMemory ? sCtx.success() : sCtx.error("创建CPU内存失败");
    }

    int createKernelMemory(SDeviceMemory& spKernelMemory, const SDeviceMemory& spMemory){
        SDevice spDevice = spMemory.device();
        if(spDevice.getPtr() == this) {
            spKernelMemory = spMemory;
            return sCtx.success();
        }

        //创建CPU内存
        SDeviceMemory toMemory;
        if( createKernelMemory(toMemory, spMemory.size()) != sCtx.success() ) {
            return sCtx.error("创建内存失败");
        }

        //拷贝内存值
        if( spMemory->readMemory(toMemory.size(), toMemory.data()) != sCtx.success() ) {
            return sCtx.error("从指定的设备内存，拷贝值到CPU内存失败");
        }

        spKernelMemory = toMemory;
        return sCtx.success();
    }

    int runKernel(  const PRuntimeKey& kernelKey, 
                int nArgs, 
                PKernalVariable pArgs[], 
                int nRanges = 0, 
                int pRanges[]=nullptr) {
        FKernalFunc func = getKernel(kernelKey);
        if(func == nullptr) {
            return sCtx.error((std::string("创建运算内核失败, 名称:") + kernelKey.runtimeKey).c_str());
        }

        #define MAX_RANGE_SIZE 10
        if(nRanges > MAX_RANGE_SIZE) {
            return sCtx.error("并行计算的RANGE维度超过了限制范围");
        }

        int pLocalRange[MAX_RANGE_SIZE];
        int* pLocalRangeEnd = pLocalRange+nRanges;
        int* pItLocalRange = pLocalRange;
        int* pItRange = pRanges;
        while(pItLocalRange<pLocalRangeEnd) {
            *pItLocalRange = 0;
            if(*pItRange < 1) {
                if(*pItRange == 0) {
                    return sCtx.success();
                }else{
                    return sCtx.error("并行计算的RANGE维度值无效(必须大于0)");
                }
            }
            pItRange++, pItLocalRange++;
        }

        PKernalCtx ctx = {
            nRanges,
            pLocalRange,
            pRanges
        };

        //
        // 这个地方如果有一个维度值为0或者小于0，则下面的程序执行会错误
        //
        int iRangeLayer=0;
        if(nRanges>0) {
            while(iRangeLayer>=0) {
                func(&ctx,nArgs,pArgs);
                iRangeLayer = nRanges-1;
                while(iRangeLayer>=0){
                    pLocalRange[iRangeLayer]++;
                    if(pLocalRange[iRangeLayer] < pRanges[iRangeLayer]) {
                        break;
                    }
                    pLocalRange[iRangeLayer--] = 0;
                }
            }
        }else{
            func(&ctx,nArgs,pArgs);
        }
        return sCtx.success();
    }

private:
    static FKernalFunc getKernel(const PRuntimeKey& kernelKey) {
        static std::map<PID,FKernalFunc> sId2Kernels;
        static std::map<string,FKernalFunc> sName2Kernels;

        auto it = sId2Kernels.find(kernelKey.runtimeId);
        if( it != sId2Kernels.end() ) {
            return it->second;
        }

        if( kernelKey.runtimeKey == nullptr ) {
            return nullptr;
        }

        auto itNamedKernel = sName2Kernels.find(kernelKey.runtimeKey);
        if( itNamedKernel != sName2Kernels.end() ) {
            sId2Kernels[PRuntimeKey(kernelKey.runtimeKey).runtimeId] = itNamedKernel->second;
            return itNamedKernel->second;
        }

        string kernalName = kernelKey.runtimeKey;
        auto iProgramName = kernalName.rfind('.');
        if( iProgramName <= 0 && iProgramName >= kernalName.length() - 1) {
            return nullptr;
        }
        SKernalOperator spOp = getOperator(kernalName.substr(0,iProgramName));
        if(!spOp) {
            return nullptr;
        }

        FKernalFunc kernelFunc = spOp->getKernalFunc(kernalName.substr(iProgramName+1).c_str());
        if(kernelFunc) {
            sId2Kernels[PRuntimeKey(kernelKey.runtimeKey).runtimeId] = kernelFunc;
            sName2Kernels[kernalName] = kernelFunc;
        }
        return kernelFunc;
    }

    static SKernalOperator getOperator(string szProgramName) {
        static std::map<string,SKernalOperator> sMapOps;
        auto it = sMapOps.find(szProgramName);
        if( it != sMapOps.end() ) {
            return it->second;
        }

        SKernalOperator spOp = SObject::createObject(szProgramName.c_str());
        if(spOp) {
            sMapOps[szProgramName] = spOp;
        }
        return spOp;
    }

public://Factory
    static const char* __getClassKey() { return "sw.device.CpuDevice"; }
};


SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CCpuDevice, CCpuDevice::__getClassKey())