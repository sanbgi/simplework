
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

    int runKernel(const PRuntimeKey& opKey, int nArgs, PMemory pArgs[], int nRanges=0, int pRanges[]=nullptr, SDeviceEvent* pEvent=nullptr) {
        IKernalOperator* pOp = getOperator(opKey);
        if(pOp == nullptr) {
            return sCtx.error((std::string("创建运算对象失败, 名称:") + opKey.runtimeKey).c_str());
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
                pOp->process(&ctx, nArgs, pArgs);
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
            pOp->process(&ctx, nArgs, pArgs);
        }
        return sCtx.success();
    }

private:
    static IKernalOperator* getOperator(const PRuntimeKey& opKey) {
        static std::map<PID,SKernalOperator> sMapOps;
        auto it = sMapOps.find(opKey.runtimeId);
        if( it != sMapOps.end() ) {
            return it->second.getPtr();
        }

        string classKey = string(opKey.runtimeKey) + "Kernel";
        SKernalOperator spOp = SObject::createObject(classKey.c_str());
        if(spOp) {
            sMapOps[opKey.runtimeId] = spOp;
        }

        return spOp.getPtr();
    }

public://Factory
    static const char* __getClassKey() { return "sw.device.CpuDevice"; }
};


SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CCpuDevice, CCpuDevice::__getClassKey())