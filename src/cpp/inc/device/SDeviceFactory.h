#ifndef __SimpleWork_Device_DeviceFactory_h__
#define __SimpleWork_Device_DeviceFactory_h__

#include "device.h"

SIMPLEWORK_DEVICE_NAMESPACE_ENTER

class SDevice;

//
// 计算设备
//
SIMPLEWORK_INTERFACECLASS_ENTER(DeviceFactory, "sw.device.DeviceFactory")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.device.IDeviceFactory", 220307)

        //
        // 获取可用设备
        //
        //virtual int getAvaiableDevices(const char* szName, IVisitor<const SDevice&>& cbVisitor) = 0;

        //
        // 获得Cpu设备
        //
        virtual int getCpuDevice(SDevice& spDevice) = 0;

        //
        // 获得Cuda设备
        //
        virtual int getCudaDevice(SDevice& spDevice) = 0;

        //
        // 获得Opencl设备
        //
        virtual int getOpenclDevice(SDevice& spDevice) = 0;

        //
        // 获得当前默认计算设备
        //
        virtual int getDefaultKernelDevic(SDevice& spDevice) = 0;

        //
        // 设置默认计算设备
        //
        virtual int setDefaultKernelDevic(const SDevice& spDevice) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    static SDeviceFactory getFactory() {
        static SDeviceFactory g_factory = SObject::createObject(SDeviceFactory::__getClassKey());
        return g_factory;
    }
 
SIMPLEWORK_INTERFACECLASS_LEAVE(DeviceFactory)

SIMPLEWORK_DEVICE_NAMESPACE_LEAVE

#endif//__SimpleWork_Device_DeviceFactory_h__