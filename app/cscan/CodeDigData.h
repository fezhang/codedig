#pragma once

#include "CodeDigType.h"
#include <string>

NS_CODE_DIG_BGN

typedef struct SrcInfo_s
{
	SrcInfo_s() :_line(CD_INVALID_UINT) {}
	CUInt32			_line;
	std::string		_src;
	std::string		_stream;
}SrcInfo;

typedef enum {
	TokenType_Unknown = 0,
	TokenType_Type,
	TokenType_TypeDef,
	TokenType_Decl,
	TokenType_Method,
	TokenType_Member,
	TokenType_Reference,
	TokenType_Namespace,
	TokenType_Enum,
	TokenType_Label,
}TokenType;

typedef enum {
	MethodType_Unkonwn = 0,
	MethodType_Global,
	MethodType_Member,
	MethodType_Inherit,
	MethodType_Override,
}MethodType;

typedef struct Instance_s
{
	Instance_s(bool definition = false) :_isDefinition(definition),_methodType(MethodType_Unkonwn) , _tkType(TokenType_Unknown){}
	bool					_isDefinition;
	TokenType		_tkType;

	SrcInfo				_srcInfo;

	std::string		_type;
	std::string		_name;

	MethodType	_methodType;	//for function dec/def
}Instance;

typedef enum {
	AccessType_Default = 0,
	AccessType_Read,
	AccessType_Write,
	AccessType_Reference,
	AccessType_Direct,	//accessing member
	AccessType_Call,
}AccessType;

typedef struct AccessOp_s
{
	AccessOp_s(AccessType at= AccessType_Default) :_accessType(at) {}
	AccessType		_accessType;
	SrcInfo				_srcInfo;
}AccessOp;

NS_CODE_DIG_END