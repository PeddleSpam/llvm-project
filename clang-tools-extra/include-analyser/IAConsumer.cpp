#include "IAConsumer.h"

IAConsumer::IAConsumer(clang::ASTContext* context) : 
  m_visitor(context)
{}

void IAConsumer::HandleTranslationUnit(clang::ASTContext& context) {
  m_visitor.TraverseDecl(context.getTranslationUnitDecl());
}

IAVisitor& IAConsumer::getVisitor() {
  return m_visitor;
}

IAVisitor const& IAConsumer::getVisitor() const {
  return m_visitor;
}