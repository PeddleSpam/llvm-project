#include "IAOptions.h"

IAOptions::IAOptions(std::string reportPath,
                     std::function<bool(llvm::StringRef)> headerFilter,
                     bool edit, bool disableInsert, bool disableRemove,
                     PrintStyle style)
    : m_reportPath(reportPath), m_headerFilter(headerFilter), m_edit(edit),
      m_disableInsert(disableInsert), m_disableRemove(disableRemove),
      m_style(style) {}
