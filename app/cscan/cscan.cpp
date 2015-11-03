// cscan.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "llvm/Support/CommandLine.h"

#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

Rewriter rewriter;
int numFunctions = 0;

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static cl::OptionCategory MyToolCategory("my-tool options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...");

class ExampleVisitor : public RecursiveASTVisitor<ExampleVisitor> {
private:
	ASTContext *astContext; // used for getting additional AST info

public:
	explicit ExampleVisitor(CompilerInstance *CI)
		: astContext(&(CI->getASTContext())) // initialize private members
	{
		rewriter.setSourceMgr(astContext->getSourceManager(),
			astContext->getLangOpts());
	}

	virtual bool VisitFunctionDecl(FunctionDecl *func) {
		numFunctions++;
		string funcName = func->getNameInfo().getName().getAsString();
		if (funcName == "do_math") {
			rewriter.ReplaceText(func->getLocation(), funcName.length(), "add5");
			errs() << "** Rewrote function def: " << funcName << "\n";
		}
		return true;
	}

	virtual bool VisitStmt(Stmt *st) {
		if (ReturnStmt *ret = dyn_cast<ReturnStmt, Stmt>(st)) {
			rewriter.ReplaceText(ret->getRetValue()->getLocStart(), 6, "val");
			errs() << "** Rewrote ReturnStmt\n";
		}
		if (CallExpr *call = dyn_cast<CallExpr, Stmt>(st)) {
			rewriter.ReplaceText(call->getLocStart(), 7, "add5");
			errs() << "** Rewrote function call\n";
		}
		return true;
	}

	// this replaces the VisitStmt function above
	/*virtual bool VisitReturnStmt(ReturnStmt *ret) {
		rewriter.ReplaceText(ret->getLocStart(), 6, "val");
		errs() << "** Rewrote ReturnStmt\n";
		return true;
	}
	virtual bool VisitCallExpr(CallExpr *call) {
		rewriter.ReplaceText(call->getLocStart(), 7, "add5");
		errs() << "** Rewrote function call\n";
		return true;
	}*/
};

class ExampleASTConsumer : public ASTConsumer {
private:
	ExampleVisitor *visitor; // doesn't have to be private

public:
	// override the constructor in order to pass CI
	explicit ExampleASTConsumer(CompilerInstance *CI)
		: visitor(new ExampleVisitor(CI)) // initialize the visitor
	{ }

	// override this to call our ExampleVisitor on each top-level Decl
	/*virtual bool HandleTopLevelDecl(DeclGroupRef DG) {
		// a DeclGroupRef may have multiple Decls, so we iterate through each one
		for (DeclGroupRef::iterator i = DG.begin(), e = DG.end(); i != e; i++) {
			Decl *D = *i;
			visitor->TraverseDecl(D); // recursively visit each AST node in Decl "D"
		}
		return true;
	}*/

	// this replaces "HandleTopLevelDecl"
	// override this to call our ExampleVisitor on the entire source file
	virtual void HandleTranslationUnit(ASTContext &Context) {
		/* we can use ASTContext to get the TranslationUnitDecl, which is
		a single Decl that collectively represents the entire source file */
		visitor->TraverseDecl(Context.getTranslationUnitDecl());
	}
};

class ExampleFrontendAction : public ASTFrontendAction {
public:
	virtual std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) {
		return std::unique_ptr<ASTConsumer>(new ExampleASTConsumer(&CI)); // pass CI pointer to ASTConsumer
	}
};

int main(int argc, const char **argv) {
	CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
	ClangTool Tool(OptionsParser.getCompilations(),
		OptionsParser.getSourcePathList());

	int result = Tool.run(newFrontendActionFactory<ExampleFrontendAction>().get());

	errs() << "\nFound " << numFunctions << " functions.\n\n";
	// print out the rewritten source code ("rewriter" is a global var.)
	rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(errs());
	return result;
}