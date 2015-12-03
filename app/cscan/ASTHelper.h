#pragma once

#include "ASTVisitor.h"
#include <unordered_map>

NS_CODE_DIG_AST_BGN

class CASTHelper: public IVistorHelper
{
public:
	////////////////////////// APIs from IVistorHelper
	virtual void initCxt(clang::SourceManager &sm, const clang::LangOptions &lo, clang::MangleContext *m);
	virtual 	bool parseInstanceInfo(DeclContext& tc, NS_CODE_DIG::Instance& instance);

	virtual 	std::string getTypeRef(clang::QualType type);
	virtual bool shouldProcess(clang::NamedDecl *d);

public:
	CASTHelper() :m_sourceManager(NULL), m_langOption(NULL) {};
	~CASTHelper() {};

private:
	std::string getFuncString(clang::NamedDecl* decl);

private:
	clang::SourceManager		*m_sourceManager;
	const clang::LangOptions	*m_langOption;

	std::unordered_map<void *, std::string> m_funcNameCache;	//decl-->func-str
	std::unique_ptr<clang::MangleContext> m_mangle;

private:
	CASTHelper(const CASTHelper&);
	CASTHelper& operator=(const CASTHelper&);
};

NS_CODE_DIG_AST_END
