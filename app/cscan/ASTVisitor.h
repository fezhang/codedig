#pragma once

#include "CodeDigListener.h"
#include "LLVMDepends.h"

NS_CODE_DIG_AST_BGN

typedef enum {
	DeclParseFlag_Default = 0xFFFFFFFF,	//parse ALL.
	DeclParseFlag_SrcInfo = 0x00001,		//parse source information.
}DeclParseFlag;

typedef struct DeclContext_s
{
	DeclContext_s(clang::NamedDecl* decl, clang::SourceRange range, clang::NamedDecl* ctx = NULL)
		:_decl(decl), _range(range), _ctx(ctx), _parseFlag(DeclParseFlag_Default) {}
	clang::NamedDecl*	_decl;
	clang::SourceRange	_range;
	clang::NamedDecl*	_ctx;
	NS_CODE_DIG::CUInt32		_parseFlag;	//REF: DeclParseFlag
private:
	DeclContext_s(); //no def ctor.
}DeclContext;

class IVistorHelper
{
public:
	virtual void initCxt(clang::SourceManager &sm, const clang::LangOptions &lo, clang::MangleContext *m) = 0;
	virtual 	bool parseInstanceInfo(DeclContext& tc, NS_CODE_DIG::Instance& instance) = 0;

	virtual 	std::string getTypeRef(clang::QualType type) = 0;
	virtual bool shouldProcess(clang::NamedDecl *d) = 0;
};

class CodeDigVisitor : public RecursiveASTVisitor<CodeDigVisitor> 
{
	typedef clang::RecursiveASTVisitor<CodeDigVisitor> Base;

public:
	 CodeDigVisitor(CompilerInstance *CI, NS_CODE_DIG::IVistorListener* pvc, IVistorHelper* helper);
	 ~CodeDigVisitor();

public:
	virtual bool VisitTypedefNameDecl(clang::TypedefNameDecl *d);
	virtual bool VisitTagDecl(clang::TagDecl *d);

	virtual bool VisitNamespaceDecl(clang::NamespaceDecl *d);
	virtual bool VisitNamespaceAliasDecl(clang::NamespaceAliasDecl *d);
	virtual bool VisitFunctionDecl(clang::FunctionDecl *d);
	virtual bool VisitEnumConstantDecl(clang::EnumConstantDecl *d);
	virtual bool VisitVarDecl(clang::VarDecl *d);
	virtual bool VisitFieldDecl(clang::FieldDecl *d);
	virtual bool VisitMemberExpr(clang::MemberExpr *e);
	virtual bool VisitDeclRefExpr(clang::DeclRefExpr *e);
	virtual bool VisitDesignatedInitExpr(clang::DesignatedInitExpr *e);

	virtual 	bool VisitTypedefTypeLoc(clang::TypedefTypeLoc TL);

	virtual bool VisitTagTypeLoc(clang::TagTypeLoc TL);

	virtual bool VisitTemplateSpecializationTypeLoc(clang::TemplateSpecializationTypeLoc TL);

	virtual bool TraverseNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc NNS);
	virtual bool TraverseUsingDirectiveDecl(clang::UsingDirectiveDecl *d);

	virtual bool TraverseConstructorInitializer(clang::CXXCtorInitializer *Init);

	virtual bool VisitCXXConstructExpr(clang::CXXConstructExpr *ctr);

	virtual bool VisitGotoStmt(clang::GotoStmt *stm);
	virtual bool VisitLabelStmt(clang::LabelStmt *stm);

	virtual bool TraverseDecl(clang::Decl *d);
	virtual bool TraverseStmt(clang::Stmt *s);

	virtual bool shouldUseDataRecursionFor(clang::Stmt *S);

	virtual bool TraverseDeclarationNameInfo(clang::DeclarationNameInfo NameInfo);
	virtual bool VisitCallExpr(clang::CallExpr *e);

private:
	NS_CODE_DIG::AccessType calcAccessType();
	bool isMember(clang::NamedDecl *d);
	bool shouldProcess(clang::NamedDecl *d);

	void InsFound(DeclContext& dc, NS_CODE_DIG::Instance& ins);

private:
	ASTContext									*m_astContext; // used for getting additional AST info
	NS_CODE_DIG::IVistorListener	*m_listener;
	IVistorHelper								*m_helper;

	class CExpressionStack
	{
	public:
		CExpressionStack() :_topExpr(NULL), _topAccessType(NS_CODE_DIG::AccessType_Default) {}
		std::deque<clang::Expr *>			_stack;
		clang::Expr									*_topExpr;
		NS_CODE_DIG::AccessType		_topAccessType;
	};
	CExpressionStack		m_exprStack;
	clang::NamedDecl		*m_declCtx;

	std::map<Decl*, std::string> m_decMap;

private:
	CodeDigVisitor();
	CodeDigVisitor(const CodeDigVisitor&);
	CodeDigVisitor& operator=(const CodeDigVisitor&);
};

NS_CODE_DIG_AST_END
