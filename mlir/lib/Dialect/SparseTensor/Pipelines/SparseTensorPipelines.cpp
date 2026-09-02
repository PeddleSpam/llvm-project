//===- SparseTensorPipelines.cpp - Pipelines for sparse tensor code -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../../../../include/mlir/Conversion/AMDGPUToROCDL/AMDGPUToROCDL.h"
#include "../../../../include/mlir/Conversion/AffineToStandard/AffineToStandard.h"
#include "../../../../include/mlir/Conversion/ArithAndMathToAPFloat/ArithToAPFloat.h"
#include "../../../../include/mlir/Conversion/ArithAndMathToAPFloat/MathToAPFloat.h"
#include "../../../../include/mlir/Conversion/ArithToAMDGPU/ArithToAMDGPU.h"
#include "../../../../include/mlir/Conversion/ArithToArmSME/ArithToArmSME.h"
#include "../../../../include/mlir/Conversion/ArithToEmitC/ArithToEmitCPass.h"
#include "../../../../include/mlir/Conversion/ArithToLLVM/ArithToLLVM.h"
#include "../../../../include/mlir/Conversion/ArithToSPIRV/ArithToSPIRV.h"
#include "../../../../include/mlir/Conversion/ArmNeon2dToIntr/ArmNeon2dToIntr.h"
#include "../../../../include/mlir/Conversion/ArmSMEToLLVM/ArmSMEToLLVM.h"
#include "../../../../include/mlir/Conversion/ArmSMEToSCF/ArmSMEToSCF.h"
#include "../../../../include/mlir/Conversion/AsyncToLLVM/AsyncToLLVM.h"
#include "../../../../include/mlir/Conversion/BufferizationToMemRef/BufferizationToMemRef.h"
#include "../../../../include/mlir/Conversion/ComplexToLLVM/ComplexToLLVM.h"
#include "../../../../include/mlir/Conversion/ComplexToLibm/ComplexToLibm.h"
#include "../../../../include/mlir/Conversion/ComplexToROCDLLibraryCalls/ComplexToROCDLLibraryCalls.h"
#include "../../../../include/mlir/Conversion/ComplexToSPIRV/ComplexToSPIRVPass.h"
#include "../../../../include/mlir/Conversion/ComplexToStandard/ComplexToStandard.h"
#include "../../../../include/mlir/Conversion/ControlFlowToLLVM/ControlFlowToLLVM.h"
#include "../../../../include/mlir/Conversion/ControlFlowToSCF/ControlFlowToSCF.h"
#include "../../../../include/mlir/Conversion/ControlFlowToSPIRV/ControlFlowToSPIRV.h"
#include "../../../../include/mlir/Conversion/ControlFlowToSPIRV/ControlFlowToSPIRVPass.h"
#include "../../../../include/mlir/Conversion/ConvertToEmitC/ConvertToEmitCPass.h"
#include "../../../../include/mlir/Conversion/ConvertToLLVM/ToLLVMPass.h"
#include "../../../../include/mlir/Conversion/FuncToEmitC/FuncToEmitCPass.h"
#include "../../../../include/mlir/Conversion/FuncToLLVM/ConvertFuncToLLVMPass.h"
#include "../../../../include/mlir/Conversion/FuncToSPIRV/FuncToSPIRVPass.h"
#include "../../../../include/mlir/Conversion/GPUCommon/GPUCommonPass.h"
#include "../../../../include/mlir/Conversion/GPUToLLVMSPV/GPUToLLVMSPVPass.h"
#include "../../../../include/mlir/Conversion/GPUToNVVM/GPUToNVVMPass.h"
#include "../../../../include/mlir/Conversion/GPUToROCDL/GPUToROCDLPass.h"
#include "../../../../include/mlir/Conversion/GPUToSPIRV/GPUToSPIRVPass.h"
#include "../../../../include/mlir/Conversion/IndexToLLVM/IndexToLLVM.h"
#include "../../../../include/mlir/Conversion/IndexToSPIRV/IndexToSPIRV.h"
#include "../../../../include/mlir/Conversion/LinalgToStandard/LinalgToStandard.h"
#include "../../../../include/mlir/Conversion/MathToEmitC/MathToEmitCPass.h"
#include "../../../../include/mlir/Conversion/MathToFuncs/MathToFuncs.h"
#include "../../../../include/mlir/Conversion/MathToLLVM/MathToLLVM.h"
#include "../../../../include/mlir/Conversion/MathToLibm/MathToLibm.h"
#include "../../../../include/mlir/Conversion/MathToNVVM/MathToNVVM.h"
#include "../../../../include/mlir/Conversion/MathToROCDL/MathToROCDL.h"
#include "../../../../include/mlir/Conversion/MathToSPIRV/MathToSPIRVPass.h"
#include "../../../../include/mlir/Conversion/MathToXeVM/MathToXeVM.h"
#include "../../../../include/mlir/Conversion/MemRefToEmitC/MemRefToEmitCPass.h"
#include "../../../../include/mlir/Conversion/MemRefToLLVM/MemRefToLLVM.h"
#include "../../../../include/mlir/Conversion/MemRefToSPIRV/MemRefToSPIRVPass.h"
#include "../../../../include/mlir/Conversion/NVGPUToNVVM/NVGPUToNVVM.h"
#include "../../../../include/mlir/Conversion/NVVMToLLVM/NVVMToLLVM.h"
#include "../../../../include/mlir/Conversion/OpenACCToLLVM/ACCToLLVM.h"
#include "../../../../include/mlir/Conversion/OpenACCToSCF/ConvertOpenACCToSCF.h"
#include "../../../../include/mlir/Conversion/OpenMPToLLVM/ConvertOpenMPToLLVM.h"
#include "../../../../include/mlir/Conversion/PDLToPDLInterp/PDLToPDLInterp.h"
#include "../../../../include/mlir/Conversion/RaiseWasm/RaiseWasmMLIR.h"
#include "../../../../include/mlir/Conversion/ReconcileUnrealizedCasts/ReconcileUnrealizedCasts.h"
#include "../../../../include/mlir/Conversion/SCFToAffine/SCFToAffine.h"
#include "../../../../include/mlir/Conversion/SCFToControlFlow/SCFToControlFlow.h"
#include "../../../../include/mlir/Conversion/SCFToEmitC/SCFToEmitC.h"
#include "../../../../include/mlir/Conversion/SCFToGPU/SCFToGPUPass.h"
#include "../../../../include/mlir/Conversion/SCFToOpenMP/SCFToOpenMP.h"
#include "../../../../include/mlir/Conversion/SCFToSPIRV/SCFToSPIRVPass.h"
#include "../../../../include/mlir/Conversion/SPIRVToLLVM/SPIRVToLLVMPass.h"
#include "../../../../include/mlir/Conversion/ShapeToStandard/ShapeToStandard.h"
#include "../../../../include/mlir/Conversion/ShardToMPI/ShardToMPI.h"
#include "../../../../include/mlir/Conversion/TensorToLinalg/TensorToLinalgPass.h"
#include "../../../../include/mlir/Conversion/TensorToSPIRV/TensorToSPIRVPass.h"
#include "../../../../include/mlir/Conversion/TosaToArith/TosaToArith.h"
#include "../../../../include/mlir/Conversion/TosaToLinalg/TosaToLinalg.h"
#include "../../../../include/mlir/Conversion/TosaToMLProgram/TosaToMLProgram.h"
#include "../../../../include/mlir/Conversion/TosaToSCF/TosaToSCF.h"
#include "../../../../include/mlir/Conversion/TosaToSPIRVTosa/TosaToSPIRVTosa.h"
#include "../../../../include/mlir/Conversion/TosaToTensor/TosaToTensor.h"
#include "../../../../include/mlir/Conversion/UBToLLVM/UBToLLVM.h"
#include "../../../../include/mlir/Conversion/UBToSPIRV/UBToSPIRV.h"
#include "../../../../include/mlir/Conversion/VectorToAMX/VectorToAMX.h"
#include "../../../../include/mlir/Conversion/VectorToArmSME/VectorToArmSME.h"
#include "../../../../include/mlir/Conversion/VectorToGPU/VectorToGPU.h"
#include "../../../../include/mlir/Conversion/VectorToSPIRV/VectorToSPIRVPass.h"
#include "../../../../include/mlir/Conversion/VectorToXeGPU/VectorToXeGPU.h"
#include "../../../../include/mlir/Conversion/XeGPUToXeVM/XeGPUToXeVM.h"
#include "../../../../include/mlir/Conversion/XeVMToLLVM/XeVMToLLVM.h"
#include "mlir/Dialect/Arith/Transforms/Passes.h"
#include "mlir/Dialect/Bufferization/Transforms/Passes.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/GPU/Transforms/Passes.h"
#include "mlir/Dialect/Linalg/Passes.h"
#include "mlir/Dialect/MemRef/Transforms/Passes.h"
#include "mlir/Dialect/SparseTensor/Pipelines/Passes.h"
#include "mlir/Transforms/Passes.h"

//===----------------------------------------------------------------------===//
// Pipeline implementation.
//===----------------------------------------------------------------------===//

void mlir::sparse_tensor::buildSparsifier(OpPassManager &pm,
                                          const SparsifierOptions &options) {
  // Rewrite named linalg ops into generic ops and apply fusion.
  pm.addNestedPass<func::FuncOp>(createLinalgGeneralizeNamedOpsPass());
  pm.addNestedPass<func::FuncOp>(createLinalgElementwiseOpFusionPass());

  // Sparsification and bufferization mini-pipeline.
  pm.addPass(createSparsificationAndBufferizationPass(
      getBufferizationOptionsForSparsification(
          options.testBufferizationAnalysisOnly),
      options.sparsificationOptions(), options.createSparseDeallocs,
      options.enableRuntimeLibrary, options.enableBufferInitialization,
      options.vectorLength,
      /*enableVLAVectorization=*/options.armSVE,
      /*enableSIMDIndex32=*/options.force32BitVectorIndices,
      options.enableGPULibgen,
      options.sparsificationOptions().sparseEmitStrategy,
      options.sparsificationOptions().parallelizationStrategy));

  // Bail-early for test setup.
  if (options.testBufferizationAnalysisOnly)
    return;

  // Storage specifier lowering and bufferization wrap-up.
  pm.addPass(createStorageSpecifierToLLVMPass());
  pm.addNestedPass<func::FuncOp>(createCanonicalizerPass());

  // GPU code generation.
  const bool gpuCodegen = options.gpuTriple.hasValue();
  if (gpuCodegen) {
    pm.addPass(createSparseGPUCodegenPass(options.gpuNumThreads,
                                          options.enableRuntimeLibrary));
    pm.addNestedPass<gpu::GPUModuleOp>(createStripDebugInfoPass());
    pm.addNestedPass<gpu::GPUModuleOp>(createSCFToControlFlowPass());
    pm.addNestedPass<gpu::GPUModuleOp>(createConvertGpuOpsToNVVMOps());
  }

  // Progressively lower to LLVM. Note that the convert-vector-to-llvm
  // pass is repeated on purpose.
  // TODO(springerm): Add sparse support to the BufferDeallocation pass and add
  // it to this pipeline.
  pm.addNestedPass<func::FuncOp>(createConvertLinalgToLoopsPass());
  pm.addNestedPass<func::FuncOp>(createConvertVectorToSCFPass());
  pm.addNestedPass<func::FuncOp>(memref::createExpandReallocPass());
  pm.addNestedPass<func::FuncOp>(createSCFToControlFlowPass());
  pm.addPass(memref::createExpandStridedMetadataPass());
  pm.addPass(createLowerAffinePass());
  pm.addPass(
      createConvertVectorToLLVMPass(options.convertVectorToLLVMOptions()));
  pm.addNestedPass<func::FuncOp>(createConvertComplexToStandardPass());
  pm.addNestedPass<func::FuncOp>(arith::createArithExpandOpsPass());
  pm.addNestedPass<func::FuncOp>(createConvertMathToLLVMPass());
  pm.addPass(createConvertMathToLibmPass());
  pm.addPass(createConvertComplexToLibm());
  pm.addPass(
      createConvertVectorToLLVMPass(options.convertVectorToLLVMOptions()));

  // Finalize GPU code generation.
  if (gpuCodegen) {
    GpuNVVMAttachTargetOptions nvvmTargetOptions;
    nvvmTargetOptions.triple = options.gpuTriple;
    nvvmTargetOptions.chip = options.gpuChip;
    nvvmTargetOptions.features = options.gpuFeatures;
    pm.addPass(createGpuNVVMAttachTarget(nvvmTargetOptions));
    pm.addPass(createGpuToLLVMConversionPass());
    GpuModuleToBinaryPassOptions gpuModuleToBinaryPassOptions;
    gpuModuleToBinaryPassOptions.compilationTarget = options.gpuFormat;
    pm.addPass(createGpuModuleToBinaryPass(gpuModuleToBinaryPassOptions));
  }

  // Convert to LLVM.
  pm.addPass(createConvertToLLVMPass());

  // Ensure all casts are realized.
  pm.addPass(createReconcileUnrealizedCastsPass());
}

//===----------------------------------------------------------------------===//
// Pipeline registration.
//===----------------------------------------------------------------------===//

void mlir::sparse_tensor::registerSparseTensorPipelines() {
  PassPipelineRegistration<SparsifierOptions>(
      "sparsifier",
      "The standard pipeline for taking sparsity-agnostic IR using the"
      " sparse-tensor type, and lowering it to LLVM IR with concrete"
      " representations and algorithms for sparse tensors.",
      buildSparsifier);
}
