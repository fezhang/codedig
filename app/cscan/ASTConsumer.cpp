#include "ASTConsumer.h"

NS_CODE_DIG_AST_BGN

///////////////////////
//class CDASTConsumer : public ASTConsumer {
bool CDASTConsumer::HandleTopLevelDecl(DeclGroupRef DG) 
{
	return true;
}

void CDASTConsumer::HandleTranslationUnit(ASTContext &Context)
{
	if (m_visitor)
		m_visitor->TraverseDecl(Context.getTranslationUnitDecl());
};

void CDASTConsumer::HandleDependentLibrary(llvm::StringRef Lib)
{
	errs()<<"\n[CDASTConsumer::HandleDependentLibrary]" << Lib.str() << "\n";
};

NS_CODE_DIG_AST_END