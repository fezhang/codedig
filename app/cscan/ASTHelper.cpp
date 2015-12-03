#include "ASTHelper.h"

NS_CODE_DIG_AST_BGN

/////////////////////////////////////////////////////////
//class CASTHelper

void CASTHelper::initCxt(clang::SourceManager &sm, const clang::LangOptions &lo, clang::MangleContext *m)
{
	m_sourceManager = &sm;
	m_langOption = &lo;
	m_mangle.reset(m);
}

std::string CASTHelper::getTypeRef(clang::QualType type)
{
	return type.getAsString(*m_langOption);
};

bool CASTHelper::shouldProcess(clang::NamedDecl *d)
{
	if (!d) return false;

	FileID fid = clang::FullSourceLoc(d->getLocation(), *m_sourceManager).getExpansionLoc().getFileID();
	//verify fid here.
	return true;
}

bool CASTHelper::parseInstanceInfo(DeclContext& tc, NS_CODE_DIG::Instance& instance)
{
	if (!tc._decl) return false;

	//annonymouse namespace, anonymous struct, or unnamed argument.
	if (tc._decl->getDeclName().isIdentifier() && tc._decl->getName().empty())
		return true;

	instance._name = getFuncString(tc._decl);

	return true;
};

std::string CASTHelper::getFuncString(clang::NamedDecl* decl)
{
	clang::Decl* canonDecl = decl->getCanonicalDecl();
	auto &cached = m_funcNameCache[canonDecl];
	if (!cached.empty()) return cached;

	std::string qualName = decl->getQualifiedNameAsString();
	if ((llvm::isa<clang::FunctionDecl>(decl) || llvm::isa<clang::VarDecl>(decl))
		&& m_mangle->shouldMangleDeclName(decl)
		&& !llvm::StringRef(qualName).startswith("__"))  //make clang happy
	{
		llvm::raw_string_ostream s(cached);
		if (llvm::isa<clang::CXXDestructorDecl>(decl)) {
			m_mangle->mangleCXXDtor(llvm::cast<clang::CXXDestructorDecl>(decl), clang::Dtor_Complete, s);
		}
		else if (llvm::isa<clang::CXXConstructorDecl>(decl)) {
			m_mangle->mangleCXXCtor(llvm::cast<clang::CXXConstructorDecl>(decl), clang::Ctor_Complete, s);
		}
		else {
			m_mangle->mangleName(decl, s);
		}
	}
	else if (clang::FieldDecl *d = llvm::dyn_cast<clang::FieldDecl>(decl))
	{
		cached = getFuncString(d->getParent()) + "::" + decl->getName().str();
	}
	else
	{
		cached = qualName;
	}

	return cached;
};

NS_CODE_DIG_AST_END