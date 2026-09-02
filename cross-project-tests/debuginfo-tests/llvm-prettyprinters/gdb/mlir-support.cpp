#include "../../../../llvm/include/llvm/ADT/FunctionExtras.h"
#include "../../../../llvm/include/llvm/ADT/Repeated.h"
#include "../../../../llvm/include/llvm/ADT/SmallBitVector.h"
#include "../../../../llvm/include/llvm/ADT/SmallPtrSet.h"
#include "../../../../llvm/include/llvm/ADT/SmallVectorExtras.h"
#include "../../../../llvm/include/llvm/ADT/ilist.h"
#include "../../../../llvm/include/llvm/Support/Allocator.h"
#include "../../../../llvm/include/llvm/Support/TrailingObjects.h"
#include "../../../../llvm/include/llvm/Support/TypeName.h"
#include "../../../../mlir/include/mlir/IR/AffineExpr.h"
#include "../../../../mlir/include/mlir/IR/AttrTypeSubElements.h"
#include "../../../../mlir/include/mlir/IR/AttributeSupport.h"
#include "../../../../mlir/include/mlir/IR/Block.h"
#include "../../../../mlir/include/mlir/IR/BlockSupport.h"
#include "../../../../mlir/include/mlir/IR/BuiltinAttributeInterfaces.h"
#include "../../../../mlir/include/mlir/IR/BuiltinAttributes.h"
#include "../../../../mlir/include/mlir/IR/BuiltinTypeInterfaces.h"
#include "../../../../mlir/include/mlir/IR/BuiltinTypes.h"
#include "../../../../mlir/include/mlir/IR/Diagnostics.h"
#include "../../../../mlir/include/mlir/IR/DialectRegistry.h"
#include "../../../../mlir/include/mlir/IR/Location.h"
#include "../../../../mlir/include/mlir/IR/QuantStorageTypeInterface.h"
#include "../../../../mlir/include/mlir/IR/StorageUniquerSupport.h"
#include "../../../../mlir/include/mlir/IR/TypeRange.h"
#include "../../../../mlir/include/mlir/IR/TypeSupport.h"
#include "../../../../mlir/include/mlir/IR/UseDefLists.h"
#include "../../../../mlir/include/mlir/IR/Value.h"
#include "../../../../mlir/include/mlir/IR/ValueRange.h"
#include "../../../../mlir/include/mlir/IR/Visitors.h"
#include "../../../../mlir/include/mlir/Support/Complex.h"
#include "../../../../mlir/include/mlir/Support/CyclicReplacerCache.h"
#include "../../../../mlir/include/mlir/Support/InterfaceSupport.h"
#include "../../../../mlir/include/mlir/Support/LLVM.h"
#include "../../../../mlir/include/mlir/Support/StorageUniquer.h"
#include "../../../../mlir/include/mlir/Support/WalkResult.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Location.h"
#include "mlir/IR/OperationSupport.h"

mlir::MLIRContext Context;

auto Identifier = mlir::StringAttr::get(&Context, "foo");
mlir::OperationName OperationName("FooOp", &Context);

mlir::Type Type(nullptr);
mlir::Type IndexType = mlir::IndexType::get(&Context);
mlir::Type IntegerType =
    mlir::IntegerType::get(&Context, 3, mlir::IntegerType::Unsigned);
mlir::Type FloatType = mlir::Float32Type::get(&Context);
mlir::Type MemRefType = mlir::MemRefType::get({4, 5}, FloatType);
mlir::Type UnrankedMemRefType = mlir::UnrankedMemRefType::get(IntegerType, 6);
mlir::Type VectorType = mlir::VectorType::get({1, 2}, FloatType);
mlir::Type TupleType =
    mlir::TupleType::get(&Context, mlir::TypeRange({IndexType, FloatType}));


mlir::detail::OutOfLineOpResult Result(FloatType, 42);
mlir::Value Value(&Result);

auto UnknownLoc = mlir::UnknownLoc::get(&Context);
auto FileLineColLoc = mlir::FileLineColLoc::get(&Context, "file", 7, 8);
auto OpaqueLoc = mlir::OpaqueLoc::get<uintptr_t>(9, &Context);
auto NameLoc = mlir::NameLoc::get(Identifier);
auto CallSiteLoc = mlir::CallSiteLoc::get(FileLineColLoc, OpaqueLoc);
auto FusedLoc = mlir::FusedLoc::get(&Context, {FileLineColLoc, NameLoc});

mlir::Attribute UnitAttr = mlir::UnitAttr::get(&Context);
mlir::Attribute FloatAttr = mlir::FloatAttr::get(FloatType, 1.0);
mlir::Attribute IntegerAttr = mlir::IntegerAttr::get(IntegerType, 10);
mlir::Attribute TypeAttr = mlir::TypeAttr::get(IndexType);
mlir::Attribute ArrayAttr = mlir::ArrayAttr::get(&Context, {UnitAttr});
mlir::Attribute StringAttr = mlir::StringAttr::get(&Context, "foo");
mlir::Attribute ElementsAttr = mlir::DenseElementsAttr::get(
    mlir::cast<mlir::ShapedType>(VectorType), llvm::ArrayRef<float>{2.0f, 3.0f});

int main() {
  // Reference symbols that might otherwise be stripped.
  std::uintptr_t result = 0;
  auto dont_strip = [&](const auto &val) {
    result += reinterpret_cast<std::uintptr_t>(&val);
  };
  dont_strip(Value);
  return result; // Non-zero return value is OK.
}
