
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
    int createMemory(const PMemory& cpuMemory, SMemory& spDeviceMemory){
        spDeviceMemory = SObject::createObject("sw.device.CpuMemory", CData<PMemory>(cpuMemory));
        return spDeviceMemory ? sCtx.success() : sCtx.error("创建CPU内存失败");
    }

    int createMemory(const SMemory& spSrcMemory, SMemory& spDeviceMemory){
        SDevice spDevice = spSrcMemory.device();
        if(spDevice.getPtr() == this) {
            spDeviceMemory = spSrcMemory;
            return sCtx.success();
        }
        PMemory sMemory;
        if( !spSrcMemory || spSrcMemory->getMemory(sMemory) != sCtx.success() ) {
            return sCtx.error("创建内存所对应的原始内存无效");
        }

        SMemory spMemory;
        if(createMemory({sMemory.size, nullptr}, spMemory) != sCtx.success()) {
            return sCtx.error();
        }
        spMemory->getMemory(sMemory);
        return spSrcMemory->getMemory(sMemory);
    }

    int runKernel(const PKernalKey& kernelKey, int nArgs, PMemory pArgs[], int nRanges=0, int pRanges[]=nullptr, SDeviceEvent* pEvent=nullptr) {
        FKernalFunc func = getKernel(kernelKey);
        if(func == nullptr) {
            return sCtx.error((std::string("创建运算内核失败, 名称:") + kernelKey.szProgramName).c_str());
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
    static FKernalFunc getKernel(const PKernalKey& kernelKey) {
        static std::map<PID,FKernalFunc> sMapKernels;
        static std::map<string,SKernalOperator> sMapOps;
        if(kernelKey.pKernalId && *kernelKey.pKernalId > 0) {
            auto it = sMapKernels.find(*kernelKey.pKernalId);
            if( it != sMapKernels.end() ) {
                return it->second;
            }
        }

        if( kernelKey.szKernalName == nullptr || kernelKey.szProgramName == nullptr) {
            return nullptr;
        }

        SKernalOperator spOp = SObject::createObject(kernelKey.szProgramName);
        if(!spOp) {
            return nullptr;
        }
        FKernalFunc kernelFunc = spOp->getKernalFunc(kernelKey.szKernalName);
        if(kernelFunc) {
            string keyName = string(kernelKey.szProgramName)+"."+kernelKey.szKernalName;
            PRuntimeKey rKey(keyName.c_str());
            sMapOps[kernelKey.szProgramName] = spOp;
            sMapKernels[rKey.runtimeId] = kernelFunc;
            if(kernelKey.pKernalId != nullptr && *kernelKey.pKernalId != rKey.runtimeId) {
                *kernelKey.pKernalId = rKey.runtimeId;
            }
        }
        return kernelFunc;
    }

public://Factory
    static const char* __getClassKey() { return "sw.device.CpuDevice"; }
};


SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CCpuDevice, CCpuDevice::__getClassKey())