//===- Conversion.cpp - C API for Conversion Passes -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../../../include/mlir/Conversion/AMDGPUToROCDL/AMDGPUToROCDL.h"
#include "../../../include/mlir/Conversion/AffineToStandard/AffineToStandard.h"
#include "../../../include/mlir/Conversion/ArithAndMathToAPFloat/ArithToAPFloat.h"
#include "../../../include/mlir/Conversion/ArithAndMathToAPFloat/MathToAPFloat.h"
#include "../../../include/mlir/Conversion/ArithToAMDGPU/ArithToAMDGPU.h"
#include "../../../include/mlir/Conversion/ArithToArmSME/ArithToArmSME.h"
#include "../../../include/mlir/Conversion/ArithToEmitC/ArithToEmitCPass.h"
#include "../../../include/mlir/Conversion/ArithToLLVM/ArithToLLVM.h"
#include "../../../include/mlir/Conversion/ArithToSPIRV/ArithToSPIRV.h"
#include "../../../include/mlir/Conversion/ArmNeon2dToIntr/ArmNeon2dToIntr.h"
#include "../../../include/mlir/Conversion/ArmSMEToLLVM/ArmSMEToLLVM.h"
#include "../../../include/mlir/Conversion/ArmSMEToSCF/ArmSMEToSCF.h"
#include "../../../include/mlir/Conversion/AsyncToLLVM/AsyncToLLVM.h"
#include "../../../include/mlir/Conversion/BufferizationToMemRef/BufferizationToMemRef.h"
#include "../../../include/mlir/Conversion/ComplexToLLVM/ComplexToLLVM.h"
#include "../../../include/mlir/Conversion/ComplexToLibm/ComplexToLibm.h"
#include "../../../include/mlir/Conversion/ComplexToROCDLLibraryCalls/ComplexToROCDLLibraryCalls.h"
#include "../../../include/mlir/Conversion/ComplexToSPIRV/ComplexToSPIRVPass.h"
#include "../../../include/mlir/Conversion/ComplexToStandard/ComplexToStandard.h"
#include "../../../include/mlir/Conversion/ControlFlowToLLVM/ControlFlowToLLVM.h"
#include "../../../include/mlir/Conversion/ControlFlowToSCF/ControlFlowToSCF.h"
#include "../../../include/mlir/Conversion/ControlFlowToSPIRV/ControlFlowToSPIRV.h"
#include "../../../include/mlir/Conversion/ControlFlowToSPIRV/ControlFlowToSPIRVPass.h"
#include "../../../include/mlir/Conversion/ConvertToEmitC/ConvertToEmitCPass.h"
#include "../../../include/mlir/Conversion/ConvertToLLVM/ToLLVMPass.h"
#include "../../../include/mlir/Conversion/FuncToEmitC/FuncToEmitCPass.h"
#include "../../../include/mlir/Conversion/FuncToLLVM/ConvertFuncToLLVMPass.h"
#include "../../../include/mlir/Conversion/FuncToSPIRV/FuncToSPIRVPass.h"
#include "../../../include/mlir/Conversion/GPUCommon/GPUCommonPass.h"
#include "../../../include/mlir/Conversion/GPUToLLVMSPV/GPUToLLVMSPVPass.h"
#include "../../../include/mlir/Conversion/GPUToNVVM/GPUToNVVMPass.h"
#include "../../../include/mlir/Conversion/GPUToROCDL/GPUToROCDLPass.h"
#include "../../../include/mlir/Conversion/GPUToSPIRV/GPUToSPIRVPass.h"
#include "../../../include/mlir/Conversion/IndexToLLVM/IndexToLLVM.h"
#include "../../../include/mlir/Conversion/IndexToSPIRV/IndexToSPIRV.h"
#include "../../../include/mlir/Conversion/LinalgToStandard/LinalgToStandard.h"
#include "../../../include/mlir/Conversion/MathToEmitC/MathToEmitCPass.h"
#include "../../../include/mlir/Conversion/MathToFuncs/MathToFuncs.h"
#include "../../../include/mlir/Conversion/MathToLLVM/MathToLLVM.h"
#include "../../../include/mlir/Conversion/MathToLibm/MathToLibm.h"
#include "../../../include/mlir/Conversion/MathToNVVM/MathToNVVM.h"
#include "../../../include/mlir/Conversion/MathToROCDL/MathToROCDL.h"
#include "../../../include/mlir/Conversion/MathToSPIRV/MathToSPIRVPass.h"
#include "../../../include/mlir/Conversion/MathToXeVM/MathToXeVM.h"
#include "../../../include/mlir/Conversion/MemRefToEmitC/MemRefToEmitCPass.h"
#include "../../../include/mlir/Conversion/MemRefToLLVM/MemRefToLLVM.h"
#include "../../../include/mlir/Conversion/MemRefToSPIRV/MemRefToSPIRVPass.h"
#include "../../../include/mlir/Conversion/NVGPUToNVVM/NVGPUToNVVM.h"
#include "../../../include/mlir/Conversion/NVVMToLLVM/NVVMToLLVM.h"
#include "../../../include/mlir/Conversion/OpenACCToLLVM/ACCToLLVM.h"
#include "../../../include/mlir/Conversion/OpenACCToSCF/ConvertOpenACCToSCF.h"
#include "../../../include/mlir/Conversion/OpenMPToLLVM/ConvertOpenMPToLLVM.h"
#include "../../../include/mlir/Conversion/PDLToPDLInterp/PDLToPDLInterp.h"
#include "../../../include/mlir/Conversion/RaiseWasm/RaiseWasmMLIR.h"
#include "../../../include/mlir/Conversion/ReconcileUnrealizedCasts/ReconcileUnrealizedCasts.h"
#include "../../../include/mlir/Conversion/SCFToAffine/SCFToAffine.h"
#include "../../../include/mlir/Conversion/SCFToControlFlow/SCFToControlFlow.h"
#include "../../../include/mlir/Conversion/SCFToEmitC/SCFToEmitC.h"
#include "../../../include/mlir/Conversion/SCFToGPU/SCFToGPUPass.h"
#include "../../../include/mlir/Conversion/SCFToOpenMP/SCFToOpenMP.h"
#include "../../../include/mlir/Conversion/SCFToSPIRV/SCFToSPIRVPass.h"
#include "../../../include/mlir/Conversion/SPIRVToLLVM/SPIRVToLLVMPass.h"
#include "../../../include/mlir/Conversion/ShapeToStandard/ShapeToStandard.h"
#include "../../../include/mlir/Conversion/ShardToMPI/ShardToMPI.h"
#include "../../../include/mlir/Conversion/TensorToLinalg/TensorToLinalgPass.h"
#include "../../../include/mlir/Conversion/TensorToSPIRV/TensorToSPIRVPass.h"
#include "../../../include/mlir/Conversion/TosaToArith/TosaToArith.h"
#include "../../../include/mlir/Conversion/TosaToLinalg/TosaToLinalg.h"
#include "../../../include/mlir/Conversion/TosaToMLProgram/TosaToMLProgram.h"
#include "../../../include/mlir/Conversion/TosaToSCF/TosaToSCF.h"
#include "../../../include/mlir/Conversion/TosaToSPIRVTosa/TosaToSPIRVTosa.h"
#include "../../../include/mlir/Conversion/TosaToTensor/TosaToTensor.h"
#include "../../../include/mlir/Conversion/UBToLLVM/UBToLLVM.h"
#include "../../../include/mlir/Conversion/UBToSPIRV/UBToSPIRV.h"
#include "../../../include/mlir/Conversion/VectorToAMX/VectorToAMX.h"
#include "../../../include/mlir/Conversion/VectorToArmSME/VectorToArmSME.h"
#include "../../../include/mlir/Conversion/VectorToGPU/VectorToGPU.h"
#include "../../../include/mlir/Conversion/VectorToLLVM/ConvertVectorToLLVMPass.h"
#include "../../../include/mlir/Conversion/VectorToSCF/VectorToSCF.h"
#include "../../../include/mlir/Conversion/VectorToSPIRV/VectorToSPIRVPass.h"
#include "../../../include/mlir/Conversion/VectorToXeGPU/VectorToXeGPU.h"
#include "../../../include/mlir/Conversion/XeGPUToXeVM/XeGPUToXeVM.h"
#include "../../../include/mlir/Conversion/XeVMToLLVM/XeVMToLLVM.h"
#include "mlir/CAPI/Pass.h"

// Must include the declarations as they carry important visibility attributes.
#include "mlir/Conversion/Passes.capi.h.inc"

using namespace mlir;

#ifdef __cplusplus
extern "C" {
#endif

#include "mlir/Conversion/Passes.capi.cpp.inc"

#ifdef __cplusplus
}
#endif
