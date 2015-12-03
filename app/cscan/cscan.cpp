// cscan.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ASTConsumer.h"

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static cl::OptionCategory MyToolCategory("my-tool options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...");

NS_CODE_DIG_AST_BGN

class CCDVistorContext: public NS_CODE_DIG::IVistorListener
{
public:
	void SetCurrSrcFile(const std::string& src)
	{
		m_currSrcFile = src;
	}

	virtual void InstanceFound(const NS_CODE_DIG::Instance& ins, std::string& id)
	{
		errs() << " \n[CCDVistorContext::InstanceFound] IsDefinition:" << ins._isDefinition
			<< " TokenType:" << ins._tkType
			<< " MethodType:" << ins._methodType
			<< " Src:" << ins._srcInfo._src
			<< " Line:" << ins._srcInfo._line
			<<"  Stream:"<< ins._srcInfo._stream
			<< " Type:" << ins._type
			<< " Name:" << ins._name << "\n";

		if (ins._tkType == NS_CODE_DIG::TokenType_Method)
		{
			++m_numFunctions; //find func.
		}

		//we'd generate id and return back.
	};

	virtual void OpFound(const NS_CODE_DIG::AccessOp& op, const std::string& caller, const std::string& callee)
	{
		errs() << "\n[CCDVistorContext::OpFound] Access type:" << op._accessType
			<<" Caller:"<<caller
			<<" Callee:" <<callee
			<< " Src:" << op._srcInfo._src
			<< " Line:" << op._srcInfo._line
			<< "  Stream:" << op._srcInfo._stream
			<< "\n";
	};

public:
	CCDVistorContext() :m_numFunctions(0) {}
	~CCDVistorContext() {}

	int GetFuncCount() { return m_numFunctions; }

private:
	int m_numFunctions;
	std::string m_currSrcFile;
};

CCDVistorContext g_vistorCtx;
NS_CODE_DIG_AST::CASTHelper g_vistorHelper;

class CDFrontendAction : public ASTFrontendAction {
public:
	virtual std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) 
	{
		g_vistorCtx.SetCurrSrcFile(file.str());
		return std::unique_ptr<ASTConsumer>(new CDASTConsumer(&CI, &g_vistorCtx, &g_vistorHelper)); // pass CI pointer to ASTConsumer
	}
};

NS_CODE_DIG_AST_END

int main(int argc, const char **argv)
{
	CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
	ClangTool Tool(OptionsParser.getCompilations(),
		OptionsParser.getSourcePathList());

	int result = Tool.run(newFrontendActionFactory<NS_CODE_DIG_AST::CDFrontendAction>().get());

	errs() << "\nFound " << NS_CODE_DIG_AST::g_vistorCtx.GetFuncCount()<< " functions.\n\n";

	return result;
}