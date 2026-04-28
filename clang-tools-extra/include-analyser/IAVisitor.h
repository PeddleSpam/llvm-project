#ifndef INCLUDE_ANALYSER_IAVISITOR_H
#define INCLUDE_ANALYSER_IAVISITOR_H

#include "clang/AST/RecursiveASTVisitor.h"

class IAVisitor : public clang::RecursiveASTVisitor<IAVisitor> {
public:
  explicit IAVisitor(clang::ASTContext* context);

  bool VisitDecl(clang::Decl* decl);
  bool VisitStmt(clang::Stmt* stmt);
  bool VisitType(clang::Type* type);

  bool TraverseTypeLoc(clang::TypeLoc typeLoc, bool traverseQualifier = true);
  bool TraverseTemplateArgumentLoc(clang::TemplateArgumentLoc const& argLoc);

  std::list<clang::Expr*>& getExprs();
  std::list<clang::Expr*> const& getExprs() const;

  std::list<clang::Type const*>& getTypes();
  std::list<clang::Type const*> const& getTypes() const;

  std::list<clang::TypeLoc>& getTypeLocs();
  std::list<clang::TypeLoc> const& getTypeLocs() const;

  std::list<clang::CallExpr*>& getCallExprs();
  std::list<clang::CallExpr*> const& getCallExprs() const;

  std::list<clang::FunctionDecl*>& getFuncDecls();
  std::list<clang::FunctionDecl*> const& getFuncDecls() const;

private:
  clang::ASTContext* m_context;
  std::list<clang::Expr*> m_exprs;
  std::list<clang::Type const*> m_types;
  std::list<clang::TypeLoc> m_typeLocs;
  std::list<clang::CallExpr*> m_callExprs;
  std::list<clang::FunctionDecl*> m_funcDecls;
};

#endif // INCLUDE_ANALYSER_IAVISITOR_H