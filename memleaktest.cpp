#include <libsolidity/interface/CompilerStack.h>

using namespace std;
using namespace dev;

int main(void)
{
	string sol{
		"contract C {\n"
		"	function f(C c) pure public returns (C) { return c; }\n"
		//"	function g() pure public returns (bytes4) { return this.g; }\n"
		//"	function h() pure public returns (bytes4) { C self = f(this); return self.f; }\n"
		"	function h() pure public returns (bytes4) { return f(this).f; }\n"
		"}\n"
	};
	string sourceCode = "pragma solidity >=0.0;\n" + sol;

	solidity::CompilerStack compiler;
	compiler.reset(false);
	compiler.addSource("<input>", sourceCode);
	compiler.compile();

	return 0;
}
