#include "IAVisitor.h"

IAVisitor::IAVisitor(clang::ASTContext* context) : 
  m_context(context)
{}

bool IAVisitor::VisitDecl(clang::Decl* decl) {
    
  if (auto* funcDecl = decl->getAsFunction()) {
    m_funcDecls.push_back(funcDecl);
  }

  return true;
}

bool IAVisitor::VisitStmt(clang::Stmt* stmt) {

  if (auto* expr = llvm::dyn_cast<clang::Expr>(stmt)) {
    m_exprs.push_back(expr);

    if (auto* callExpr = llvm::dyn_cast<clang::CallExpr>(expr)) {
      m_callExprs.push_back(callExpr);
    }

    if (auto* tmpObj = llvm::dyn_cast<clang::CXXTemporaryObjectExpr>(expr)) {
      auto* srcInfo = tmpObj->getTypeSourceInfo();
      m_types.push_back(srcInfo->getType().getTypePtr());
      m_typeLocs.push_back(srcInfo->getTypeLoc());
    }
  }

  return true;
}

bool IAVisitor::VisitType(clang::Type* type) {
  //m_types.push_back(type);
  return true;
}

bool IAVisitor::TraverseTypeLoc(
    clang::TypeLoc typeLoc, bool traverseQualifier) {
  //m_typeLocs.push_back(typeLoc);
  return true;
}

bool IAVisitor::TraverseTemplateArgumentLoc(
    clang::TemplateArgumentLoc const& argLoc) {
  if (argLoc.getArgument().getKind() == clang::TemplateArgument::Type)
    m_typeLocs.push_back(argLoc.getTypeSourceInfo()->getTypeLoc());
  
  return true;
}

std::list<clang::Expr*>& IAVisitor::getExprs() {
  return m_exprs;
}

std::list<clang::Expr*> const& IAVisitor::getExprs() const {
  return m_exprs;
}

std::list<clang::Type const*>& IAVisitor::getTypes() {
  return m_types;
}

std::list<clang::Type const*> const& IAVisitor::getTypes() const {
  return m_types;
}

std::list<clang::TypeLoc>& IAVisitor::getTypeLocs() {
  return m_typeLocs;
}

std::list<clang::TypeLoc> const& IAVisitor::getTypeLocs() const {
  return m_typeLocs;
}

std::list<clang::CallExpr*>& IAVisitor::getCallExprs() {
  return m_callExprs;
}

std::list<clang::CallExpr*> const& IAVisitor::getCallExprs() const {
  return m_callExprs;
}

std::list<clang::FunctionDecl*>& IAVisitor::getFuncDecls() {
  return m_funcDecls;
}

std::list<clang::FunctionDecl*> const& IAVisitor::getFuncDecls() const {
  return m_funcDecls;
}
