
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
    int createMemory(const PMemory& cpuMemory, SDeviceMemory& spDeviceMemory){
        spDeviceMemory = SObject::createObject("sw.device.CpuMemory", CData<PMemory>(cpuMemory));
        return spDeviceMemory ? sCtx.success() : sCtx.error("创建CPU内存失败");
    }

    int createMemory(const SDeviceMemory& spMemory, SDeviceMemory& spDeviceMemory){
        SDevice spDevice = spMemory.device();
        if(spDevice.getPtr() == this) {
            spDeviceMemory = spMemory;
            return sCtx.success();
        }

        //创建CPU内存
        SDeviceMemory toMemory;
        if( createMemory({spMemory.size(), nullptr}, toMemory) != sCtx.success() ) {
            return sCtx.error("创建内存失败");
        }

        //获取内存指针
        PMemory sMemory;
        if( toMemory->getMemoryInDevice(toMemory.device(), sMemory) != sCtx.success() ) {
            return sCtx.error("未知异常错误");
        }

        //拷贝内存值
        if( spMemory->getCpuMemory(sMemory) != sCtx.success() ) {
            return sCtx.error("从指定的设备内存，拷贝值到CPU内存失败");
        }

        spDeviceMemory = toMemory;
        return sCtx.success();
    }

    int runKernel(  const PKernalKey& kernelKey, 
                int nArgs, 
                PKernalVariable pArgs[], 
                int nRanges = 0, 
                int pRanges[]=nullptr) {
        FKernalFunc func = getKernel(kernelKey);
        if(func == nullptr) {
            return sCtx.error((std::string("创建运算内核失败, 名称:") + kernelKey.szKernalName).c_str());
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
        static std::map<PID,FKernalFunc> sId2Kernels;
        static std::map<string,FKernalFunc> sName2Kernels;
        if(kernelKey.pKernalId) {
            if( *kernelKey.pKernalId > 0 ) {
                auto it = sId2Kernels.find(*kernelKey.pKernalId);
                if( it != sId2Kernels.end() ) {
                    return it->second;
                }
                return nullptr;
            }
        }

        if( kernelKey.szKernalName == nullptr ) {
            return nullptr;
        }

        auto it = sName2Kernels.find(kernelKey.szKernalName);
        if( it != sName2Kernels.end() ) {
            if(kernelKey.pKernalId != nullptr ) {
                PRuntimeKey rKey(kernelKey.szKernalName);
                sId2Kernels[rKey.runtimeId] = it->second;
                *kernelKey.pKernalId = rKey.runtimeId;
            }
            return it->second;
        }

        string kernalName = kernelKey.szKernalName;
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
           if(kernelKey.pKernalId != nullptr) {
                PRuntimeKey rKey(kernalName.c_str());
                sId2Kernels[rKey.runtimeId] = kernelFunc;
                 *kernelKey.pKernalId = rKey.runtimeId;
            }
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