#include <fstream>

#include "clang/Tooling/Tooling.h"

#include "IAFrontendAction.h"

int main(int argc, char **argv) {
  if (argc < 2)
    return -1;

  auto inFile = std::ifstream();
  inFile.open(argv[1], std::ios::binary | std::ios::ate);
  if (!inFile.is_open()) {
    llvm::outs() << "Failed to open input file: " << argv[1] << "\n";
    return -1;
  }

  auto const fileSize = std::size_t(inFile.tellg());
  auto buffer = std::string();
  buffer.resize(fileSize + 1u);
  inFile.seekg(0u);
  inFile.read(buffer.data(), fileSize);

  auto readCount = size_t(inFile.gcount());
  if (readCount < fileSize) {
    llvm::outs() << "Failed to read input file: " << readCount << " out of ";
    llvm::outs() << fileSize << "\n";
    return -1;
  }
  inFile.close();

  auto const numArgs = size_t(argc);
  auto args = std::vector<std::string>();
  args.reserve(numArgs);
  for (auto i = 2u; i < numArgs; ++i) {
    args.push_back(argv[i]);
  }

  auto outFileName = std::string("log.txt");
  auto errCode = std::error_code();
  auto outFile = llvm::raw_fd_ostream(
    outFileName, errCode, llvm::sys::fs::OF_None);
  auto* outStream = static_cast<llvm::raw_ostream*>(&outFile);
  
  if (errCode) {
    llvm::outs() << "Failed to open output file: " << outFileName << "\n";
    outStream = &llvm::outs();
  }

  clang::tooling::runToolOnCodeWithArgs(
    std::make_unique<IAFrontendAction>(*outStream), buffer, args, argv[1]);

  return 0;
}
