//===- RegisterAllPasses.cpp - MLIR Registration ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines a helper to trigger the registration of all passes to the
// system.
//
//===----------------------------------------------------------------------===//

#include "../include/mlir/Conversion/AMDGPUToROCDL/AMDGPUToROCDL.h"
#include "../include/mlir/Conversion/AffineToStandard/AffineToStandard.h"
#include "../include/mlir/Conversion/ArithAndMathToAPFloat/ArithToAPFloat.h"
#include "../include/mlir/Conversion/ArithAndMathToAPFloat/MathToAPFloat.h"
#include "../include/mlir/Conversion/ArithToAMDGPU/ArithToAMDGPU.h"
#include "../include/mlir/Conversion/ArithToArmSME/ArithToArmSME.h"
#include "../include/mlir/Conversion/ArithToEmitC/ArithToEmitCPass.h"
#include "../include/mlir/Conversion/ArithToLLVM/ArithToLLVM.h"
#include "../include/mlir/Conversion/ArithToSPIRV/ArithToSPIRV.h"
#include "../include/mlir/Conversion/ArmNeon2dToIntr/ArmNeon2dToIntr.h"
#include "../include/mlir/Conversion/ArmSMEToLLVM/ArmSMEToLLVM.h"
#include "../include/mlir/Conversion/ArmSMEToSCF/ArmSMEToSCF.h"
#include "../include/mlir/Conversion/AsyncToLLVM/AsyncToLLVM.h"
#include "../include/mlir/Conversion/BufferizationToMemRef/BufferizationToMemRef.h"
#include "../include/mlir/Conversion/ComplexToLLVM/ComplexToLLVM.h"
#include "../include/mlir/Conversion/ComplexToLibm/ComplexToLibm.h"
#include "../include/mlir/Conversion/ComplexToROCDLLibraryCalls/ComplexToROCDLLibraryCalls.h"
#include "../include/mlir/Conversion/ComplexToSPIRV/ComplexToSPIRVPass.h"
#include "../include/mlir/Conversion/ComplexToStandard/ComplexToStandard.h"
#include "../include/mlir/Conversion/ControlFlowToLLVM/ControlFlowToLLVM.h"
#include "../include/mlir/Conversion/ControlFlowToSCF/ControlFlowToSCF.h"
#include "../include/mlir/Conversion/ControlFlowToSPIRV/ControlFlowToSPIRV.h"
#include "../include/mlir/Conversion/ControlFlowToSPIRV/ControlFlowToSPIRVPass.h"
#include "../include/mlir/Conversion/ConvertToEmitC/ConvertToEmitCPass.h"
#include "../include/mlir/Conversion/ConvertToLLVM/ToLLVMPass.h"
#include "../include/mlir/Conversion/FuncToEmitC/FuncToEmitCPass.h"
#include "../include/mlir/Conversion/FuncToLLVM/ConvertFuncToLLVMPass.h"
#include "../include/mlir/Conversion/FuncToSPIRV/FuncToSPIRVPass.h"
#include "../include/mlir/Conversion/GPUCommon/GPUCommonPass.h"
#include "../include/mlir/Conversion/GPUToLLVMSPV/GPUToLLVMSPVPass.h"
#include "../include/mlir/Conversion/GPUToNVVM/GPUToNVVMPass.h"
#include "../include/mlir/Conversion/GPUToROCDL/GPUToROCDLPass.h"
#include "../include/mlir/Conversion/GPUToSPIRV/GPUToSPIRVPass.h"
#include "../include/mlir/Conversion/IndexToLLVM/IndexToLLVM.h"
#include "../include/mlir/Conversion/IndexToSPIRV/IndexToSPIRV.h"
#include "../include/mlir/Conversion/LinalgToStandard/LinalgToStandard.h"
#include "../include/mlir/Conversion/MathToEmitC/MathToEmitCPass.h"
#include "../include/mlir/Conversion/MathToFuncs/MathToFuncs.h"
#include "../include/mlir/Conversion/MathToLLVM/MathToLLVM.h"
#include "../include/mlir/Conversion/MathToLibm/MathToLibm.h"
#include "../include/mlir/Conversion/MathToNVVM/MathToNVVM.h"
#include "../include/mlir/Conversion/MathToROCDL/MathToROCDL.h"
#include "../include/mlir/Conversion/MathToSPIRV/MathToSPIRVPass.h"
#include "../include/mlir/Conversion/MathToXeVM/MathToXeVM.h"
#include "../include/mlir/Conversion/MemRefToEmitC/MemRefToEmitCPass.h"
#include "../include/mlir/Conversion/MemRefToLLVM/MemRefToLLVM.h"
#include "../include/mlir/Conversion/MemRefToSPIRV/MemRefToSPIRVPass.h"
#include "../include/mlir/Conversion/NVGPUToNVVM/NVGPUToNVVM.h"
#include "../include/mlir/Conversion/NVVMToLLVM/NVVMToLLVM.h"
#include "../include/mlir/Conversion/OpenACCToLLVM/ACCToLLVM.h"
#include "../include/mlir/Conversion/OpenACCToSCF/ConvertOpenACCToSCF.h"
#include "../include/mlir/Conversion/OpenMPToLLVM/ConvertOpenMPToLLVM.h"
#include "../include/mlir/Conversion/PDLToPDLInterp/PDLToPDLInterp.h"
#include "../include/mlir/Conversion/RaiseWasm/RaiseWasmMLIR.h"
#include "../include/mlir/Conversion/ReconcileUnrealizedCasts/ReconcileUnrealizedCasts.h"
#include "../include/mlir/Conversion/SCFToAffine/SCFToAffine.h"
#include "../include/mlir/Conversion/SCFToControlFlow/SCFToControlFlow.h"
#include "../include/mlir/Conversion/SCFToEmitC/SCFToEmitC.h"
#include "../include/mlir/Conversion/SCFToGPU/SCFToGPUPass.h"
#include "../include/mlir/Conversion/SCFToOpenMP/SCFToOpenMP.h"
#include "../include/mlir/Conversion/SCFToSPIRV/SCFToSPIRVPass.h"
#include "../include/mlir/Conversion/SPIRVToLLVM/SPIRVToLLVMPass.h"
#include "../include/mlir/Conversion/ShapeToStandard/ShapeToStandard.h"
#include "../include/mlir/Conversion/ShardToMPI/ShardToMPI.h"
#include "../include/mlir/Conversion/TensorToLinalg/TensorToLinalgPass.h"
#include "../include/mlir/Conversion/TensorToSPIRV/TensorToSPIRVPass.h"
#include "../include/mlir/Conversion/TosaToArith/TosaToArith.h"
#include "../include/mlir/Conversion/TosaToLinalg/TosaToLinalg.h"
#include "../include/mlir/Conversion/TosaToMLProgram/TosaToMLProgram.h"
#include "../include/mlir/Conversion/TosaToSCF/TosaToSCF.h"
#include "../include/mlir/Conversion/TosaToSPIRVTosa/TosaToSPIRVTosa.h"
#include "../include/mlir/Conversion/TosaToTensor/TosaToTensor.h"
#include "../include/mlir/Conversion/UBToLLVM/UBToLLVM.h"
#include "../include/mlir/Conversion/UBToSPIRV/UBToSPIRV.h"
#include "../include/mlir/Conversion/VectorToAMX/VectorToAMX.h"
#include "../include/mlir/Conversion/VectorToArmSME/VectorToArmSME.h"
#include "../include/mlir/Conversion/VectorToGPU/VectorToGPU.h"
#include "../include/mlir/Conversion/VectorToSPIRV/VectorToSPIRVPass.h"
#include "../include/mlir/Conversion/VectorToXeGPU/VectorToXeGPU.h"
#include "../include/mlir/Conversion/XeGPUToXeVM/XeGPUToXeVM.h"
#include "../include/mlir/Conversion/XeVMToLLVM/XeVMToLLVM.h"
#include "../include/mlir/Dialect/LLVMIR/Transforms/AddComdats.h"
#include "../include/mlir/Dialect/LLVMIR/Transforms/LegalizeForExport.h"
#include "../include/mlir/Dialect/LLVMIR/Transforms/RequestCWrappers.h"
#include "../include/mlir/Dialect/NVVM/Transforms/OptimizeForNVVM.h"
#include "mlir/InitAllPasses.h"

#include "mlir/Dialect/AMDGPU/Transforms/Passes.h"
#include "mlir/Dialect/Affine/Transforms/Passes.h"
#include "mlir/Dialect/Arith/Transforms/Passes.h"
#include "mlir/Dialect/ArmSVE/Transforms/Passes.h"
#include "mlir/Dialect/Async/Passes.h"
#include "mlir/Dialect/Bufferization/Pipelines/Passes.h"
#include "mlir/Dialect/Bufferization/Transforms/Passes.h"
#include "mlir/Dialect/EmitC/Transforms/Passes.h"
#include "mlir/Dialect/Func/Transforms/Passes.h"
#include "mlir/Dialect/GPU/Pipelines/Passes.h"
#include "mlir/Dialect/GPU/Transforms/Passes.h"
#include "mlir/Dialect/Linalg/Passes.h"
#include "mlir/Dialect/MLProgram/Transforms/Passes.h"
#include "mlir/Dialect/Math/Transforms/Passes.h"
#include "mlir/Dialect/MemRef/Transforms/Passes.h"
#include "mlir/Dialect/NVGPU/Transforms/Passes.h"
#include "mlir/Dialect/OpenACC/Transforms/Passes.h"
#include "mlir/Dialect/OpenMP/Transforms/Passes.h"
#include "mlir/Dialect/Quant/Transforms/Passes.h"
#include "mlir/Dialect/SCF/Transforms/Passes.h"
#include "mlir/Dialect/SPIRV/Transforms/Passes.h"
#include "mlir/Dialect/Shape/Transforms/Passes.h"
#include "mlir/Dialect/Shard/Transforms/Passes.h"
#include "mlir/Dialect/SparseTensor/Pipelines/Passes.h"
#include "mlir/Dialect/Tensor/Transforms/Passes.h"
#include "mlir/Dialect/Transform/Transforms/Passes.h"
#include "mlir/Dialect/Vector/Transforms/Passes.h"
#include "mlir/Dialect/XeGPU/Transforms/Passes.h"
#include "mlir/Target/LLVMIR/Transforms/Passes.h"
#include "mlir/Transforms/Passes.h"

// This function may be called to register the MLIR passes with the
// global registry.
// If you're building a compiler, you likely don't need this: you would build a
// pipeline programmatically without the need to register with the global
// registry, since it would already be calling the creation routine of the
// individual passes.
// The global registry is interesting to interact with the command-line tools.
void mlir::registerAllPasses() {
  // General passes
  registerTransformsPasses();

  // Conversion passes
  registerConversionPasses();

  // Dialect passes
  acc::registerOpenACCPasses();
  affine::registerAffinePasses();
  amdgpu::registerAMDGPUPasses();
  registerAsyncPasses();
  arith::registerArithPasses();
  bufferization::registerBufferizationPasses();
  func::registerFuncPasses();
  registerGPUPasses();
  registerLinalgPasses();
  registerNVGPUPasses();
  NVVM::registerNVVMPasses();
  registerSparseTensorPasses();
  LLVM::registerLLVMPasses();
  LLVM::registerTargetLLVMIRTransformsPasses();
  math::registerMathPasses();
  memref::registerMemRefPasses();
  shard::registerShardPasses();
  ml_program::registerMLProgramPasses();
  omp::registerOpenMPPasses();
  quant::registerQuantPasses();
  registerSCFPasses();
  registerShapePasses();
  spirv::registerSPIRVPasses();
  tensor::registerTensorPasses();
  tosa::registerTosaPasses();
  transform::registerTransformPasses();
  vector::registerVectorPasses();
  arm_sme::registerArmSMEPasses();
  arm_sve::registerArmSVEPasses();
  emitc::registerEmitCPasses();
  xegpu::registerXeGPUPasses();

  // Dialect pipelines
  bufferization::registerBufferizationPipelines();
  sparse_tensor::registerSparseTensorPipelines();
  tosa::registerTosaToLinalgPipelines();
  gpu::registerGPUToNVVMPipeline();
  gpu::registerGPUToROCDLPipeline();
  gpu::registerGPUToXeVMPipeline();
}
