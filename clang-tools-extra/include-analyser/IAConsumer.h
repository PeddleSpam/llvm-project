#ifndef INCLUDE_ANALYSER_IACONSUMER_H
#define INCLUDE_ANALYSER_IACONSUMER_H

#include "clang/AST/ASTConsumer.h"

#include "IAVisitor.h"

class IAConsumer : public clang::ASTConsumer {
public:
  explicit IAConsumer(clang::ASTContext* context);

  void HandleTranslationUnit(clang::ASTContext& context) override;

  IAVisitor& getVisitor();
  IAVisitor const& getVisitor() const;

private:
  IAVisitor m_visitor;
};

#endif // INCLUDE_ANALYSER_IACONSUMER_H