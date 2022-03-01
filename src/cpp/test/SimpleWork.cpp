#include <iostream>
#include <vector>

#include "../inc/SimpleWork.h"
#include <math.h>
#include "CNnNetwork.h"
#include "CAvNetwork.h"

using namespace sw;
using namespace sw;
using namespace sw;
using namespace sw;

void testIdx() {
    SCtx sCtx("TestSimpleWork");

    //std::string strFilename = "D:\\Workspace\\tensorflow\\tensorflow_datas\\downloads\\extracted\\GZIP.cvdf-datasets_mnist_t10k-labels-idx1-ubyte965g-S4A7G3r0jpgiMMdvSNx7KP_oN7677JZkkIErsY.gz";
    std::string strFilename = "D:\\Workspace\\tensorflow\\tensorflow_datas\\downloads\\extracted\\GZIP.cvdf-datasets_mnist_t10k-images-idx3-ubytejUIsewocHHkkWlvPB_6G4z7q_ueSuEWErsJ29aLbxOY.gz";
    //SNnNetwork::getFactory()->readIdxFile("D:\\Workspace\\tensorflow\\tensorflow_datas\\downloads\\extracted\\GZIP.cvdf-datasets_mnist_t10k-images-idx3-ubytejUIsewocHHkkWlvPB_6G4z7q_ueSuEWErsJ29aLbxOY.gz", spTensor);
    //SNnNetwork::getFactory()->readIdxFile("", spTensor);
    //SNnNetwork::getFactory()->readIdxFile("D:\\Workspace\\tensorflow\\tensorflow_datas\\downloads\\extracted\\GZIP.cvdf-datasets_mnist_train-images-idx3-ubyteRA_Kv3PMVG-iFHXoHqNwJlYF9WviEKQCTSyo8gNSNgk.gz", spTensor);
    //SNnNetwork::getFactory()->readIdxFile("D:\\Workspace\\tensorflow\\tensorflow_datas\\downloads\\extracted\\GZIP.cvdf-datasets_mnist_train-labels-idx1-ubyteNVJTSgpVi77WrtMrMMSVzKI9Vn7FLKyL4aBzDoAQJVw.gz", spTensor);
    SNnPipe spReader;
    SNnFactory::getFactory()->openIdxFileReader(strFilename.c_str(), spReader);
    if(spReader) {
        int nRead = 10;
        int nReaded = 0;
        STensor spIn;
        STensor::createVector(spIn, 1, &nRead);
        STensor spOut;
        while(spReader->push(spIn, spOut) == sCtx.success()) {
            nReaded += 10;
            //std::cout << spOut->getDataSize();
        }

        nRead = 0;
    }
}

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200
#include "cl/cl2.hpp"

int testOpenCL()
{
    const int numElements = 32;

    // Filter for a 2.0 platform and set it as the default
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Platform plat;
    for (auto &p : platforms) {
        std::string platver = p.getInfo<CL_PLATFORM_VERSION>();
        if (platver.find("OpenCL 2.") != std::string::npos) {
            plat = p;
        }
    }
    if (plat() == 0)  {
        std::cout << "No OpenCL 2.0 platform found.";
        return -1;
    }

    cl::Platform newP = cl::Platform::setDefault(plat);
    if (newP != plat) {
        std::cout << "Error setting default platform.";
        return -1;
    }

    // Use C++11 raw string literals for kernel source code
    std::string kernel1{R"CLC(
        global int globalA;
        kernel void updateGlobal()
        {
            globalA = 75;
        }
    )CLC"};
    std::string kernel2{R"CLC(
        typedef struct { global int *bar; } Foo;
        kernel void vectorAdd(global const int *inputA,
                                global int *output)
        {
            output[get_global_id(0)] = 1;
        }
    )CLC"};

    // New simpler string interface style
    std::vector<std::string> programStrings {kernel1, kernel2};

    cl::Program vectorAddProgram(programStrings);
    try {
        vectorAddProgram.build("-cl-std=CL2.0");
    }
    catch (...) {
        // Print build info for all devices
        cl_int buildErr = CL_SUCCESS;
        auto buildInfo = vectorAddProgram.getBuildInfo<CL_PROGRAM_BUILD_LOG>(&buildErr);
        for (auto &pair : buildInfo) {
            std::cerr << pair.second << std::endl << std::endl;
        }

        return 1;
    }

    // Get and run kernel that initializes the program-scope global
    // A test for kernels that take no arguments
    auto program2Kernel =
        cl::KernelFunctor<>(vectorAddProgram, "updateGlobal");
    program2Kernel(
        cl::EnqueueArgs(
        cl::NDRange(1)));

    //////////////////
    // SVM allocations
    cl::SVMAllocator<int, cl::SVMTraitCoarse<>> svmAlloc;
    std::vector<int, cl::SVMAllocator<int, cl::SVMTraitCoarse<>>> inputA(numElements, 1, svmAlloc);
    cl::coarse_svm_vector<int> inputB(numElements, 2, svmAlloc);

    //
    //////////////

    // Traditional cl_mem allocations
    std::vector<int> output(numElements, 0xdeadbeef);
    cl::Buffer outputBuffer(begin(output), end(output), false);
    //cl::Pipe aPipe(sizeof(cl_int), numElements / 2);

    // Default command queue, also passed in as a parameter
    //cl::DeviceCommandQueue defaultDeviceQueue = cl::DeviceCommandQueue::makeDefault(
    //    cl::Context::getDefault(), cl::Device::getDefault());

    auto vectorAddKernel =
        cl::KernelFunctor<
            int*,
            cl::Buffer
            >(vectorAddProgram, "vectorAdd");

    // Hand control of coarse allocations to runtime
    //cl::unmapSVM(output);

    cl_int error;
    vectorAddKernel(
        cl::EnqueueArgs(
            cl::NDRange(numElements/2),
            cl::NDRange(numElements/2)),
        inputA.data(),
        outputBuffer,
        error
        );

    cl::copy(outputBuffer, begin(output), end(output));
    // Grab the SVM output vector using a map
    //cl::mapSVM(output);

    std::cout << "Output:\n";
    for (int i = 1; i < numElements; ++i) {
        std::cout << "\t" << output[i] << "\n";
    }
    std::cout << "\n\n";

    return 0;
}

auto getInt(){
    return 1;
}

int main(int argc, char *argv[]){
    //testOpenCL();
    //cl::Buffer bf();
    //cl::copy();
    //FAA a = { 10 };
    //FBB b = { 20 };
    //fun(CData<FAA>(a));
    //fun(CData<FBB>(b));
    //unsigned int la = SData::getBasicTypeIdentifier<int>();
    //unsigned long lb = SData::getTypeIdentifier("aaaaaaaaaaaaaaaafadfasdf");
    //unsigned long lc = SData::getTypeIdentifier("ffffffffffffffffasdfasdfasdfasdfa");
    //unsigned long ld = SData::getTypeIdentifier("ffffffffffffffffffasdfasdfadsfadsfasdf");
    //testWriteFile();
    //testPipe();
    //testTensor();
    //testNN();
    //testIdx();
    //CNnNetwork::runFile();
    CNnNetwork::run();
    //CAvNetwork::runImage();

    return 0;
}
