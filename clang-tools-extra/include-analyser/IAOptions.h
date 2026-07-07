#ifndef INCLUDE_ANALYSER_IAOPTIONS_H
#define INCLUDE_ANALYSER_IAOPTIONS_H

#include <functional>
#include <string>

#include "llvm/ADT/StringRef.h"

struct IAOptions {
  enum class PrintStyle { Changes, Final };

  IAOptions(std::string reportPath,
            std::function<bool(llvm::StringRef)> headerFilter, bool edit,
            bool disableInsert, bool disableRemove, PrintStyle style);

  std::string m_reportPath;
  std::function<bool(llvm::StringRef)> m_headerFilter;
  bool m_edit;
  bool m_disableInsert;
  bool m_disableRemove;
  PrintStyle m_style;
};

#endif // INCLUDE_ANALYSER_IAOPTIONS_H