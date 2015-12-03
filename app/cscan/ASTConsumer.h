#pragma once

#include "ASTHelper.h"

NS_CODE_DIG_AST_BGN

class CDASTConsumer : public ASTConsumer {
private:
	CodeDigVisitor	*m_visitor; // doesn't have to be private
	IVistorHelper		*m_vistorHelper;

public:
	explicit CDASTConsumer(CompilerInstance *CI, NS_CODE_DIG::IVistorListener* pvc, IVistorHelper* pHelper)
		: m_visitor(new CodeDigVisitor(CI, pvc, pHelper)) // initialize the visitor
		, m_vistorHelper(pHelper)
	{ }

	virtual bool HandleTopLevelDecl(DeclGroupRef DG);
	virtual void HandleTranslationUnit(ASTContext &Context);
	virtual void HandleDependentLibrary(llvm::StringRef Lib);
	virtual void Initialize(clang::ASTContext& ctx) override 
	{
		if (m_vistorHelper)
			m_vistorHelper->initCxt(ctx.getSourceManager(), ctx.getLangOpts(), ctx.createMangleContext());
	}
};

NS_CODE_DIG_AST_END