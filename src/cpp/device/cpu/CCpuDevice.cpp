
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
    int createKernelMemory(SKernelMemory& spKernelMemory, int nSize, void* pData = nullptr){
        spKernelMemory = SObject::createObject("sw.device.CpuMemory", CData<PMemory>({nSize, pData}));
        return spKernelMemory ? sCtx.success() : sCtx.error("创建CPU内存失败");
    }

    int createKernelMemory(SKernelMemory& spKernelMemory, const SKernelMemory& spMemory){
        SDevice spDevice = spMemory.device();
        if(spDevice.getPtr() == this) {
            spKernelMemory = spMemory;
            return sCtx.success();
        }

        //创建CPU内存
        SKernelMemory toMemory;
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

    #define MAX_WRITE_BACK 20
    class CWriteBack{
    public:
        struct CItem{
            SKernelMemory spKernelMemory;
            SDeviceMemory spDeviceMemory;
        }arrWriteBacks[MAX_WRITE_BACK];
        int nWriteBacks = 0;
        int nErr;
    public:
        void pushIn(const SKernelMemory& spKernelMemory) {
            if(nWriteBacks == MAX_WRITE_BACK) {
                nErr = sCtx.error("内核参数过多，超过了限制范围");
            }else{
                arrWriteBacks[nWriteBacks++].spKernelMemory = spKernelMemory;
            }
        }
        void pushIn(const SDeviceMemory& spDeviceMemory, const SKernelMemory& spKernelMemory) {
            if(nWriteBacks == MAX_WRITE_BACK) {
                nErr = sCtx.error("内核参数过多，超过了限制范围");
            }else{
                arrWriteBacks[nWriteBacks].spDeviceMemory = spDeviceMemory;
                arrWriteBacks[nWriteBacks].spKernelMemory = spKernelMemory;
                nWriteBacks++;
            }
        }
        int error(const char* szError) {
            nErr = sCtx.error(szError);
            return nErr;
        }

        static void release(CWriteBack* pWriteBack) {
            CWriteBack::CItem* pItem = pWriteBack->arrWriteBacks;
            while(pWriteBack->nWriteBacks-->0){
                if(pItem->spDeviceMemory) {
                    if( pItem->spDeviceMemory->writeKernelMemory(pItem->spKernelMemory) != sCtx.success() ) {
                        pWriteBack->error("从内核回写内存错误");
                    }
                    pItem->spDeviceMemory.release();
                }
                pItem->spKernelMemory.release();
                pItem++;
            }
        }
    };

    int runKernel(  const PRuntimeKey& kernelKey, 
                int nArgs, 
                PKernelVariable pArgs[], 
                int nRanges = 0, 
                int pRanges[]=nullptr) {
        FKernelFunc func = getKernel(kernelKey);
        if(func == nullptr) {
            return sCtx.error((std::string("创建运算内核失败, 名称:") + kernelKey.runtimeKey).c_str());
        }

        //
        //  回写控制
        //
        static CWriteBack sWriteBack;
        sWriteBack.nErr = sCtx.success();
        CTaker<CWriteBack*> spWriteTaker(&sWriteBack, CWriteBack::release);

        //
        //  支持IKernelPointer和IDeviceMemory类型参数
        //
        const SDevice& spDevice = SDevice::cpu();
        PKernelVariable* pArg = pArgs;
        for(int i=0; i<nArgs; i++, pArg++) {
            switch(pArg->type){
                case PKernelVariable::EDevicePointer:
                {
                    SDeviceMemory& spDeviceMemory = *pArg->pDeviceMemory;
                    switch(pArg->mode) {
                        case PKernelVariable::EReadOnly:{
                            SKernelMemory spKernelMemory;
                            if( spDeviceMemory->createKernelMemory(spDevice, spKernelMemory) != sCtx.success() ) {
                                return sCtx.error("创建内核参数异常");
                            }
                            sWriteBack.pushIn(spKernelMemory);
                            pArg->p = spKernelMemory.data();
                        }break;

                        case PKernelVariable::EReadWrite:{
                            SKernelMemory spKernelMemory;
                            if( spDeviceMemory->createKernelMemory(spDevice, spKernelMemory) != sCtx.success() ) {
                                return sCtx.error("创建内核参数异常");
                            }
                            sWriteBack.pushIn(spDeviceMemory, spKernelMemory);
                            pArg->p = spKernelMemory.data();
                        }
                        break;

                        case PKernelVariable::EWriteOnly:{
                            SKernelMemory spKernelMemory;
                            if( spDevice->createKernelMemory(spKernelMemory, spDeviceMemory.size()) != sCtx.success() ) {
                                return sCtx.error("创建内核参数异常");
                            }
                            sWriteBack.pushIn(spDeviceMemory, spKernelMemory);
                            pArg->p = spKernelMemory.data();
                        }break;
                    }
                    break;
                }
            }
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

        PKernelCtx ctx = {
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

        spWriteTaker.release();
        return sWriteBack.nErr;
    }

private:
    static FKernelFunc getKernel(const PRuntimeKey& kernelKey) {
        static std::map<PID,FKernelFunc> sId2Kernels;
        static std::map<string,FKernelFunc> sName2Kernels;

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

        string kernelName = kernelKey.runtimeKey;
        auto iProgramName = kernelName.rfind('.');
        if( iProgramName <= 0 && iProgramName >= kernelName.length() - 1) {
            return nullptr;
        }
        SKernelOperator spOp = getOperator(kernelName.substr(0,iProgramName));
        if(!spOp) {
            return nullptr;
        }

        FKernelFunc kernelFunc = spOp->getKernelFunc(kernelName.substr(iProgramName+1).c_str());
        if(kernelFunc) {
            sId2Kernels[PRuntimeKey(kernelKey.runtimeKey).runtimeId] = kernelFunc;
            sName2Kernels[kernelName] = kernelFunc;
        }
        return kernelFunc;
    }

    static SKernelOperator getOperator(string szProgramName) {
        static std::map<string,SKernelOperator> sMapOps;
        auto it = sMapOps.find(szProgramName);
        if( it != sMapOps.end() ) {
            return it->second;
        }

        SKernelOperator spOp = SObject::createObject(szProgramName.c_str());
        if(spOp) {
            sMapOps[szProgramName] = spOp;
        }
        return spOp;
    }

public://Factory
    static const char* __getClassKey() { return "sw.device.CpuDevice"; }
};


SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CCpuDevice, CCpuDevice::__getClassKey())