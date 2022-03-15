#ifndef __SimpleWork_STensorSolver_h__
#define __SimpleWork_STensorSolver_h__

#include "math.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

class STensorHooker;
struct PTensorOperator {
    enum {
        plus,
        minus,
        multiply,
        divide,
        product,

        square,
        sqrt,
        sum,
        avg,

        toFloat,
        toDouble,
    }id;
};

//
// 向量求解器
//
SIMPLEWORK_INTERFACECLASS_ENTER(TensorSolver, "sw.math.TensorSolver")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.ITensorSolver", 220309)
        //
        // 解析求解（部分简单的张量求解辅助函数）
        //
        virtual int solve( const PTensorOperator& rOp, int nVars, STensor pVars[]) = 0;

        //
        // 内核求解
        //  说明：所有张量解析求解需要调用的统一内核求解函数，系统可以HOOK这个函数的执行，便于
        //  记录下张量求解步骤。
        //
        //  @kernelKey (同runEvalKernel)
        //  @kernalRange (同runEvalKernel)
        //  @kernalParameter (同runEvalKernel)
        //  @pVars 求解涉及的张量，每个张量必须是一个合法的张量，内核求解时，不会检查张量是否合法
        //
        virtual int solve(
                        PRuntimeKey kernalKey, 
                        PVector kernalRange, 
                        PMemory kernalParameter,
                        int nVars, STensor pVars[]) = 0;

        //
        // 设置钩子，只有最后一个钩子有效
        //
        virtual int pushHooker(const STensorHooker& spHooker) = 0;

        //
        // 获取钩子
        //
        virtual int getHooker(STensorHooker& spHooker) = 0;

        //
        // 取消钩子
        //
        virtual int popHooker() = 0;

        //
        // 设备上正向运行
        //  
        //  @kernelKey 内核标识，其中，为了提升性能，做了如下设计（较复杂）
        //      kernelKey.pKernelId 指向内核ID的指针（既是输入，也是输出，可为空），内核会在调
        //  用后，确保指针直指向的值与实际内核ID相同。这么设计的目的是，允许调用着在第一次调用后，
        //  缓存内核ID，便于下次调用时，内核可以根据这个ID，快速找到内核对象。
        //
        //  @kernalParameter：
        //          1，如果size == 0 字节，则不会传递这个参数给内核
        //          2，如果size > 0 && size <= 8字节，则指针指向的内容，会作为参数传递给内核；
        //          3，如果size > 8字节，则指针值会作为参数，传递给内核(大小为指针大小)
        //  @pVars，
        //          1，每一个pVar会传递两个参数给内核；
        //          2，参数一：为size
        //          3，参数二：数据指针值（所以，其指针务必为设备内存指针）
        //
        //virtual int runEvalKernel(
        //                const SDevice& spDevice,
        //                PRuntimeKey kernelKey,
        //                PVector kernalRange,
        //                PMemory kernalParameter,
        //                int nVars,
        //                const PVector pVars[] ) = 0;

        //
        // 设备上反向偏导（累加模式）
        //
        //  @kernelKey，内核标识（同runEvalKernel)
        //
        //  @kernalParameter，内核参数（同runEvalKernel)
        //
        //  @pVars，求解需要的张量
        //      1，每一个pVar会传递三个参数给内核；
        //      2，参数一：为size
        //      3，参数二：数据指针值（所以，其指针务必为设备内存指针）
        //      4，参数三：偏导指针值（所以，其指针务必为设备内存指针）
        //
        //virtual int runDeviaKernel(
        //                const SDevice& spDevice,
        //                PRuntimeKey kernelKey,
        //                PVector kernalRange,
        //                PMemory kernalParameter,
        //                int nVars,
        //                const PDeviaVector pVars[] ) = 0;


    SIMPLEWORK_INTERFACE_LEAVE

    static STensorSolver getSolver(){
        static STensorSolver g_solver = SObject::createObject(STensorSolver::__getClassKey());
        return g_solver;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(TensorSolver)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_STensorSolver_h__