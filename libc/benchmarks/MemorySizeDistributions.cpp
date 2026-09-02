#include "MemorySizeDistributions.h"

#ifdef LIBC_BENCHMARKS_HAS_LLVM_SUPPORT
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#else
#include <iostream>
#include <sstream>
#endif

namespace llvm {
namespace libc_benchmarks {

static constexpr double MemmoveGoogleA[] = {
};
static constexpr double MemmoveGoogleB[] = {
};
static constexpr double MemmoveGoogleD[] = {
};
static constexpr double MemmoveGoogleQ[] = {
};
static constexpr double MemmoveGoogleL[] = {
};
static constexpr double MemmoveGoogleM[] = {
};
static constexpr double MemmoveGoogleS[] = {
};
static constexpr double MemmoveGoogleW[] = {
};
static constexpr double MemmoveGoogleU[] = {
};
static constexpr double MemcmpGoogleA[] = {
};
static constexpr double MemcmpGoogleB[] = {
};
static constexpr double MemcmpGoogleD[] = {
};
static constexpr double MemcmpGoogleQ[] = {
};
static constexpr double MemcmpGoogleL[] = {
};
static constexpr double MemcmpGoogleM[] = {
};
static constexpr double MemcmpGoogleS[] = {
};
static constexpr double MemcmpGoogleW[] = {
};
static constexpr double MemcmpGoogleU[] = {
};
static constexpr double MemcpyGoogleA[] = {
};
static constexpr double MemcpyGoogleB[] = {
};
static constexpr double MemcpyGoogleD[] = {
};
static constexpr double MemcpyGoogleQ[] = {
};
static constexpr double MemcpyGoogleL[] = {
};
static constexpr double MemcpyGoogleM[] = {
};
static constexpr double MemcpyGoogleS[] = {
};
static constexpr double MemcpyGoogleW[] = {
};
static constexpr double MemcpyGoogleU[] = {
};
static constexpr double MemsetGoogleA[] = {
};
static constexpr double MemsetGoogleB[] = {
};
static constexpr double MemsetGoogleD[] = {
};
static constexpr double MemsetGoogleQ[] = {
};
static constexpr double MemsetGoogleL[] = {
};
static constexpr double MemsetGoogleM[] = {
};
static constexpr double MemsetGoogleS[] = {
};
static constexpr double MemsetGoogleW[] = {
};
static constexpr double MemsetGoogleU[] = {
};
static constexpr double Uniform384To4096[] = {
};

ArrayRef<MemorySizeDistribution> getMemmoveSizeDistributions() {
  static constexpr MemorySizeDistribution kDistributions[] = {
      {"memmove Google A", MemmoveGoogleA},
      {"memmove Google B", MemmoveGoogleB},
      {"memmove Google D", MemmoveGoogleD},
      {"memmove Google L", MemmoveGoogleL},
      {"memmove Google M", MemmoveGoogleM},
      {"memmove Google Q", MemmoveGoogleQ},
      {"memmove Google S", MemmoveGoogleS},
      {"memmove Google U", MemmoveGoogleU},
      {"memmove Google W", MemmoveGoogleW},
      {"uniform 384 to 4096", Uniform384To4096},
  };
  return kDistributions;
}

ArrayRef<MemorySizeDistribution> getMemcpySizeDistributions() {
  static constexpr MemorySizeDistribution kDistributions[] = {
      {"memcpy Google A", MemcpyGoogleA},
      {"memcpy Google B", MemcpyGoogleB},
      {"memcpy Google D", MemcpyGoogleD},
      {"memcpy Google L", MemcpyGoogleL},
      {"memcpy Google M", MemcpyGoogleM},
      {"memcpy Google Q", MemcpyGoogleQ},
      {"memcpy Google S", MemcpyGoogleS},
      {"memcpy Google U", MemcpyGoogleU},
      {"memcpy Google W", MemcpyGoogleW},
      {"uniform 384 to 4096", Uniform384To4096},
  };
  return kDistributions;
}

ArrayRef<MemorySizeDistribution> getMemsetSizeDistributions() {
  static constexpr MemorySizeDistribution kDistributions[] = {
      {"memset Google A", MemsetGoogleA},
      {"memset Google B", MemsetGoogleB},
      {"memset Google D", MemsetGoogleD},
      {"memset Google L", MemsetGoogleL},
      {"memset Google M", MemsetGoogleM},
      {"memset Google Q", MemsetGoogleQ},
      {"memset Google S", MemsetGoogleS},
      {"memset Google U", MemsetGoogleU},
      {"memset Google W", MemsetGoogleW},
      {"uniform 384 to 4096", Uniform384To4096},
  };
  return kDistributions;
}

ArrayRef<MemorySizeDistribution> getMemcmpSizeDistributions() {
  static constexpr MemorySizeDistribution kDistributions[] = {
      {"memcmp Google A", MemcmpGoogleA},
      {"memcmp Google B", MemcmpGoogleB},
      {"memcmp Google D", MemcmpGoogleD},
      {"memcmp Google L", MemcmpGoogleL},
      {"memcmp Google M", MemcmpGoogleM},
      {"memcmp Google Q", MemcmpGoogleQ},
      {"memcmp Google S", MemcmpGoogleS},
      {"memcmp Google U", MemcmpGoogleU},
      {"memcmp Google W", MemcmpGoogleW},
      {"uniform 384 to 4096", Uniform384To4096},
  };
  return kDistributions;
}

MemorySizeDistribution
getDistributionOrDie(ArrayRef<MemorySizeDistribution> Distributions,
                     StringRef Name) {
  for (const auto &MSD : Distributions)
    if (MSD.name == Name)
      return MSD;

#ifdef LIBC_BENCHMARKS_HAS_LLVM_SUPPORT
  std::string Message;
  raw_string_ostream Stream(Message);
  Stream << "Unknown MemorySizeDistribution '" << Name
         << "', available distributions:\n";
  for (const auto &MSD : Distributions)
    Stream << "'" << MSD.name << "'\n";
  report_fatal_error(Message);
#else
  std::stringstream Stream;
  Stream << "Unknown MemorySizeDistribution '" << std::string(Name)
         << "', available distributions:\n";
  for (const auto &MSD : Distributions)
    Stream << "'" << MSD.name.str() << "'\n";
  report_fatal_error(Stream.str());
#endif
}

} // namespace libc_benchmarks
} // namespace llvm
