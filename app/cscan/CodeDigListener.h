#pragma once

#include "CodeDigData.h"

NS_CODE_DIG_BGN

class IVistorListener
{
public:
	virtual void InstanceFound(const Instance& stmt, std::string& id) = 0;
	virtual void OpFound(const AccessOp& op, const std::string& caller, const std::string& callee) = 0;
};

NS_CODE_DIG_END