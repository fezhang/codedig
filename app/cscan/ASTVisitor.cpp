#include "ASTVisitor.h"

NS_CODE_DIG_AST_BGN

/////////////////////////
//class CodeDigVisitor : public RecursiveASTVisitor<CodeDigVisitor> {
CodeDigVisitor::CodeDigVisitor(CompilerInstance *CI, NS_CODE_DIG::IVistorListener* pvc, IVistorHelper* helper)
	: m_astContext(&(CI->getASTContext())) // initialize private members
	, m_listener(pvc)
	, m_declCtx(NULL)
	, m_helper(helper)
{
}

CodeDigVisitor::~CodeDigVisitor()
{
	m_decMap.clear();
}

void CodeDigVisitor::InsFound(DeclContext& dc, NS_CODE_DIG::Instance& ins)
{
	if (!m_helper || !m_listener) return;

	if (NS_CODE_DIG::TokenType_TypeDef == ins._tkType)
		dc._parseFlag = DeclParseFlag_SrcInfo;

	m_helper->parseInstanceInfo(dc, ins);

	std::string id;
	m_listener->InstanceFound(ins, id);
	if (!id.empty())
		m_decMap.insert(std::make_pair(dc._decl, id));
}

bool CodeDigVisitor::VisitTypedefNameDecl(clang::TypedefNameDecl *d) 
{
	errs() << "[CodeDigVisitor::VisitTypedefNameDecl] Name:" << d->getNameAsString() << "\n";

	NS_CODE_DIG::Instance ins;
	ins._tkType = NS_CODE_DIG::TokenType_TypeDef;
	ins._type = m_helper->getTypeRef(d->getUnderlyingType());

	DeclContext dc(d, d->getLocation());
	
	InsFound(dc, ins);

	return true;
}

bool CodeDigVisitor::VisitTypedefTypeLoc(clang::TypedefTypeLoc ttl)
{
	TypedefNameDecl *decl = ttl.getTypedefNameDecl();

	errs() << "[CodeDigVisitor::VisitTypedefTypeLoc] Name:" << decl->getNameAsString() << "\n";

	NS_CODE_DIG::Instance ins;
	ins._tkType = NS_CODE_DIG::TokenType_TypeDef;
	ins._type = m_helper->getTypeRef(decl->getUnderlyingType());

	DeclContext dc(decl, ttl.getSourceRange(), m_declCtx);

	InsFound(dc, ins);

	return true;
}

bool CodeDigVisitor::VisitTagTypeLoc(clang::TagTypeLoc TL)
{
	TagDecl *decl = TL.getDecl();

	errs() << "[CodeDigVisitor::VisitTagTypeLoc] Name:" << decl->getNameAsString() << "\n";

	NS_CODE_DIG::Instance ins;
	ins._tkType = NS_CODE_DIG::TokenType_Type;
	ins._type = decl->getNameAsString();

	DeclContext dc(decl, TL.getSourceRange().getBegin(), m_declCtx);

	InsFound(dc, ins);

	return true;
}

/// TagDecl - Represents the declaration of a struct/union/class/enum
bool CodeDigVisitor::VisitTagDecl(clang::TagDecl *d)
{
	errs() << "[CodeDigVisitor::VisitTagDecl] Name:"<<d->getNameAsString()<<" Kind:"<<d->getKindName ().str()<<" \n";

	NS_CODE_DIG::Instance ins;
	ins._tkType = NS_CODE_DIG::TokenType_Type;
	ins._type = d->getNameAsString();

	DeclContext dc(d, d->getLocation());

	InsFound(dc, ins);

	return true;
}

bool CodeDigVisitor::VisitNamespaceDecl(clang::NamespaceDecl *d) 
{
	errs() << "[CodeDigVisitor::VisitNamespaceDecl] Name:" << d->getNameAsString() << "\n";

	NS_CODE_DIG::Instance ins;
	ins._tkType = NS_CODE_DIG::TokenType_Namespace;
	
	DeclContext dc(d, d->getLocation());

	InsFound(dc, ins);
	return true;
}

///Represents a C++ namespace alias.
bool CodeDigVisitor::VisitNamespaceAliasDecl(clang::NamespaceAliasDecl *d)
{
	errs() << "[CodeDigVisitor::VisitNamespaceAliasDecl] Name:" << d->getNameAsString() << "\n";

	NS_CODE_DIG::Instance ins;
	ins._tkType = NS_CODE_DIG::TokenType_Namespace;

	DeclContext dc(d, d->getLocation());

	InsFound(dc, ins);
	return true;
}

bool CodeDigVisitor::VisitFunctionDecl(clang::FunctionDecl *d) 
{
	if (!shouldProcess(d)) return false;

	bool isDefinition = d->isThisDeclarationADefinition() || d->hasAttr<clang::AliasAttr>();

	NS_CODE_DIG::Instance ins(isDefinition);
	ins._tkType = NS_CODE_DIG::TokenType_Method;
	ins._type = m_helper->getTypeRef(d->getType());

	DeclContext dc(d, d->getNameInfo().getSourceRange());

	InsFound(dc, ins);
	return true;
}

bool CodeDigVisitor::VisitEnumConstantDecl(clang::EnumConstantDecl *d) 
{
	errs() << "[CodeDigVisitor::VisitEnumConstantDecl] Name:" << d->getNameAsString() << "\n";

	NS_CODE_DIG::Instance ins;
	ins._tkType = NS_CODE_DIG::TokenType_Enum;
	ins._type = d->getInitVal().toString(10);

	DeclContext dc(d, d->getLocation());

	InsFound(dc, ins);
	return true;
}

bool CodeDigVisitor::VisitVarDecl(clang::VarDecl *d)
{
	if (!shouldProcess(d)) return false;

	errs() << "[CodeDigVisitor::VisitVarDecl] Name:" << d->getNameAsString() << "\n";

	NS_CODE_DIG::Instance ins;
	ins._tkType = NS_CODE_DIG::TokenType_Decl;
	ins._type = m_helper->getTypeRef(d->getType());

	DeclContext dc(d, d->getLocation());

	InsFound(dc, ins);
	return true;
}

bool CodeDigVisitor::VisitFieldDecl(clang::FieldDecl *d) 
{
	errs() << "[CodeDigVisitor::VisitFieldDecl] Name:" << d->getNameAsString() << "\n";

	NS_CODE_DIG::Instance ins;
	ins._tkType = NS_CODE_DIG::TokenType_Decl;
	ins._type = m_helper->getTypeRef(d->getType());

	DeclContext dc(d, d->getLocation());

	InsFound(dc, ins);
	return true;
}

bool CodeDigVisitor::VisitMemberExpr(clang::MemberExpr *e) 
{
	errs() << "[CodeDigVisitor::VisitMemberExpr] Name:" << e->getMemberDecl()->getNameAsString() << "\n";

	NS_CODE_DIG::Instance ins;
	if (isMember(e->getMemberDecl()))
		ins._tkType = NS_CODE_DIG::TokenType_Member;
	else
		ins._tkType = NS_CODE_DIG::TokenType_Reference;
	ins._type = m_helper->getTypeRef(e->getMemberDecl()->getType());

	DeclContext dc(e->getMemberDecl(), e->getMemberNameInfo().getSourceRange(), m_declCtx);

	InsFound(dc, ins);
	return true;
}

bool CodeDigVisitor::VisitDeclRefExpr(clang::DeclRefExpr *e) 
{
	clang::ValueDecl* decl = e->getDecl();
	errs() << "[CodeDigVisitor::VisitDeclRefExpr] Name:" << decl->getNameAsString() << "\n";

	NS_CODE_DIG::Instance ins;

	if (llvm::isa<clang::EnumConstantDecl>(decl))
		ins._tkType = NS_CODE_DIG::TokenType_Enum;
	else if (isMember(decl))
		ins._tkType = NS_CODE_DIG::TokenType_Member;
	else
		ins._tkType = NS_CODE_DIG::TokenType_Reference;

	ins._type = m_helper->getTypeRef(decl->getType());

	DeclContext dc(decl, e->getNameInfo().getSourceRange(), m_declCtx);

	InsFound(dc, ins);
	return true;
}

///Represents a C99 designated initializer expression.
bool CodeDigVisitor::VisitDesignatedInitExpr(clang::DesignatedInitExpr *e)
{
	errs() << "[CodeDigVisitor::VisitDesignatedInitExpr]\n";
	return true;
}

bool CodeDigVisitor::VisitTemplateSpecializationTypeLoc(clang::TemplateSpecializationTypeLoc TL) {

	clang::TemplateDecl* decl = TL.getTypePtr()->getTemplateName().getAsTemplateDecl();
	if (decl) 
	{
		NS_CODE_DIG::Instance ins;
		ins._tkType = NS_CODE_DIG::TokenType_Type;
		ins._type = decl->getTemplatedDecl()->getNameAsString();

		DeclContext dc(decl, TL.getTemplateNameLoc());

		InsFound(dc, ins);
	}
	else {
		errs() << "\n[CodeDigVistor::VisitTemplateSpecializationTypeLoc] No decl:" << TL.getType().getAsString();
	}
	return true;
}

//A C++ nested-name-specifier augmented with source location
bool CodeDigVisitor::TraverseNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc NNS) 
{
	if (!NNS)	return true;

	switch (NNS.getNestedNameSpecifier()->getKind()) {
	case clang::NestedNameSpecifier::Namespace:
		if (NNS.getNestedNameSpecifier()->getAsNamespace()->isAnonymousNamespace())
			break;
		else
		{
			NS_CODE_DIG::Instance ins;
			ins._tkType = NS_CODE_DIG::TokenType_Namespace;

			DeclContext dc(NNS.getNestedNameSpecifier()->getAsNamespace(), NNS.getSourceRange());

			InsFound(dc, ins);
		}
		return true;
	case clang::NestedNameSpecifier::NamespaceAlias:
		{
			NS_CODE_DIG::Instance ins;
			ins._tkType = NS_CODE_DIG::TokenType_Namespace;

			DeclContext dc(NNS.getNestedNameSpecifier()->getAsNamespaceAlias()->getAliasedNamespace()
				, NNS.getSourceRange());

			InsFound(dc, ins);
		}
		return true; 
	default: break;
	}
	return Base::TraverseNestedNameSpecifierLoc(NNS);
}

bool CodeDigVisitor::TraverseUsingDirectiveDecl(clang::UsingDirectiveDecl *d) 
{
	NS_CODE_DIG::Instance ins;
	ins._tkType = NS_CODE_DIG::TokenType_Namespace;

	DeclContext dc(d->getNominatedNamespace()
		, { d->getQualifierLoc().getBeginLoc(), d->getIdentLocation() });

	InsFound(dc, ins);

	return true;
}

//Represents a C++ base or member initializer.
bool CodeDigVisitor::TraverseConstructorInitializer(clang::CXXCtorInitializer *Init) 
{
	if (Init->isAnyMemberInitializer() && Init->isWritten()) 
	{
		NS_CODE_DIG::Instance ins;
		if (Init->isMemberInitializer())
		{
			ins._tkType = NS_CODE_DIG::TokenType_Member;
		}
		else {
			ins._tkType = NS_CODE_DIG::TokenType_Reference;
		}

		DeclContext dc(Init->getAnyMember(), 
			Init->getMemberLocation(),
			m_declCtx);

		InsFound(dc, ins);
	}
	return Base::TraverseConstructorInitializer(Init);
}

//Represents a call to a C++ constructor.
bool CodeDigVisitor::VisitCXXConstructExpr(clang::CXXConstructExpr *ctr)
{
	//TODO:
	//FIXME: it should be a CALL ACTION!!!!
	clang::CXXConstructorDecl *decl = ctr->getConstructor();
	if (!decl) return true;

	NS_CODE_DIG::Instance ins;
	ins._tkType = NS_CODE_DIG::TokenType_Method;

	DeclContext dc(decl,
		ctr->getParenOrBraceRange().getBegin(),
		m_declCtx);

	InsFound(dc, ins);

	return true;
}

bool CodeDigVisitor::VisitGotoStmt(clang::GotoStmt *stm)
{
	LabelDecl* decl = stm->getLabel();
	if (!decl) return false;

	NS_CODE_DIG::Instance ins;
	ins._tkType = NS_CODE_DIG::TokenType_Label;

	DeclContext dc(decl,
		stm->getLabelLoc(),
		m_declCtx);

	InsFound(dc, ins);

	return true;
}

bool CodeDigVisitor::VisitLabelStmt(clang::LabelStmt *stm) 
{
	LabelDecl* decl = stm->getDecl();
	if (!decl) return false;

	NS_CODE_DIG::Instance ins;
	ins._tkType = NS_CODE_DIG::TokenType_Label;

	DeclContext dc(decl,
		stm->getIdentLoc(),
		m_declCtx);

	InsFound(dc, ins);

	return true;
}

bool CodeDigVisitor::TraverseDecl(clang::Decl *d) 
{
	if (!d) return true;
	clang::NamedDecl	*prev = m_declCtx;
	if (clang::FunctionDecl::classof(d) 
		|| clang::RecordDecl::classof(d)
		|| clang::NamespaceDecl::classof(d) 
		|| clang::TemplateDecl::classof(d)) 
	{
		m_declCtx = llvm::dyn_cast<clang::NamedDecl>(d);
	}
	if (auto v = llvm::dyn_cast_or_null<clang::VarDecl>(d)) 
	{
		if (v->getInit() && !m_exprStack._topExpr)
		{
			m_exprStack._topExpr = v->getInit();
			auto t = v->getType();
			if (t->isReferenceType() && !t.getNonReferenceType().isConstQualified())
				m_exprStack._topAccessType = NS_CODE_DIG::AccessType_Reference;
			else
				m_exprStack._topAccessType = NS_CODE_DIG::AccessType_Read;
		}
	}
	Base::TraverseDecl(d);
	m_declCtx = prev;
	return true;
}

bool CodeDigVisitor::TraverseStmt(clang::Stmt *s)
{
	clang::Expr* exp = llvm::dyn_cast_or_null<clang::Expr>(s);
	CExpressionStack old_stack;
	if (exp) {
		m_exprStack._stack.push_front(exp);
	}
	else {
		std::swap(old_stack, m_exprStack);
		if (auto i = llvm::dyn_cast_or_null<clang::IfStmt>(s)) {
			m_exprStack._topExpr = i->getCond();
			m_exprStack._topAccessType = NS_CODE_DIG::AccessType_Read;
		}
		else if (auto r = llvm::dyn_cast_or_null<clang::ReturnStmt>(s)) {
			m_exprStack._topExpr = r->getRetValue();
			if (auto f = llvm::dyn_cast_or_null<clang::FunctionDecl>(m_declCtx))
			{
				auto t = f->getReturnType();
				if (t->isReferenceType())
					m_exprStack._topAccessType = NS_CODE_DIG::AccessType_Reference; // non const reference
				else
					m_exprStack._topAccessType = NS_CODE_DIG::AccessType_Read; // anything else is considered as read;
			}
		}
		else if (auto sw = llvm::dyn_cast_or_null<clang::SwitchStmt>(s)) 
		{
			m_exprStack._topExpr = sw->getCond();
			m_exprStack._topAccessType = NS_CODE_DIG::AccessType_Read;
		}
		else if (auto d = llvm::dyn_cast_or_null<clang::DoStmt>(s)) 
		{
			m_exprStack._topExpr = d->getCond();
			m_exprStack._topAccessType = NS_CODE_DIG::AccessType_Read;
		}
		else if (auto w = llvm::dyn_cast_or_null<clang::WhileStmt>(s)) 
		{
			m_exprStack._topExpr = w->getCond();
			m_exprStack._topAccessType = NS_CODE_DIG::AccessType_Read;
		}
	}

	auto r = Base::TraverseStmt(s);
	if (exp) {
		m_exprStack._stack.pop_front();
	}
	else {
		std::swap(old_stack, m_exprStack);
	}
	return r;
}

bool CodeDigVisitor::shouldUseDataRecursionFor(clang::Stmt *S)
{
	return false;
}

bool CodeDigVisitor::TraverseDeclarationNameInfo(clang::DeclarationNameInfo NameInfo)
{
	return true;
}

bool CodeDigVisitor::VisitCallExpr(clang::CallExpr *e) 
{
	return true;
}

bool CodeDigVisitor::isMember(clang::NamedDecl *d)
{
	if (!m_declCtx) return false;

	clang::CXXRecordDecl *ctx = llvm::dyn_cast<clang::CXXRecordDecl>(m_declCtx->getDeclContext());
	if (!ctx) return false;
	if (d->getDeclContext() == ctx)
		return true;

	clang::CXXRecordDecl *rec = llvm::dyn_cast<clang::CXXRecordDecl>(d->getDeclContext());
	return rec && ctx->isDerivedFrom(rec);
}

bool CodeDigVisitor::shouldProcess(clang::NamedDecl *d)
{
	if (!m_helper) return true;
	return m_helper->shouldProcess(d);
}

NS_CODE_DIG_AST_END