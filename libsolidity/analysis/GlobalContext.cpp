/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @author Gav Wood <g@ethdev.com>
 * @date 2014
 * Container of the (implicit and explicit) global objects.
 */

#include <libsolidity/analysis/GlobalContext.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/ast/Types.h>
#include <memory>

using namespace std;

namespace dev
{
namespace solidity
{

std::vector<std::shared_ptr<MagicVariableDeclaration const>> constructMagicVariables()
{
	static auto const magicVariableDecl = [](string const& _name, Type const* _type) {
		return make_shared<MagicVariableDeclaration>(_name, _type);
	};

	TypeProvider& typeProvider = TypeProvider::get();

	return {
		magicVariableDecl("abi", typeProvider.magicType(MagicType::Kind::ABI)),
		magicVariableDecl("addmod", typeProvider.functionType(strings{"uint256", "uint256", "uint256"}, strings{"uint256"}, FunctionType::Kind::AddMod, false, StateMutability::Pure)),
		magicVariableDecl("assert", typeProvider.functionType(strings{"bool"}, strings{}, FunctionType::Kind::Assert, false, StateMutability::Pure)),
		magicVariableDecl("block", typeProvider.magicType(MagicType::Kind::Block)),
		magicVariableDecl("blockhash", typeProvider.functionType(strings{"uint256"}, strings{"bytes32"}, FunctionType::Kind::BlockHash, false, StateMutability::View)),
		magicVariableDecl("ecrecover", typeProvider.functionType(strings{"bytes32", "uint8", "bytes32", "bytes32"}, strings{"address"}, FunctionType::Kind::ECRecover, false, StateMutability::Pure)),
		magicVariableDecl("gasleft", typeProvider.functionType(strings(), strings{"uint256"}, FunctionType::Kind::GasLeft, false, StateMutability::View)),
		magicVariableDecl("keccak256", typeProvider.functionType(strings{"bytes memory"}, strings{"bytes32"}, FunctionType::Kind::KECCAK256, false, StateMutability::Pure)),
		magicVariableDecl("log0", typeProvider.functionType(strings{"bytes32"}, strings{}, FunctionType::Kind::Log0)),
		magicVariableDecl("log1", typeProvider.functionType(strings{"bytes32", "bytes32"}, strings{}, FunctionType::Kind::Log1)),
		magicVariableDecl("log2", typeProvider.functionType(strings{"bytes32", "bytes32", "bytes32"}, strings{}, FunctionType::Kind::Log2)),
		magicVariableDecl("log3", typeProvider.functionType(strings{"bytes32", "bytes32", "bytes32", "bytes32"}, strings{}, FunctionType::Kind::Log3)),
		magicVariableDecl("log4", typeProvider.functionType(strings{"bytes32", "bytes32", "bytes32", "bytes32", "bytes32"}, strings{}, FunctionType::Kind::Log4)),
		magicVariableDecl("msg", typeProvider.magicType(MagicType::Kind::Message)),
		magicVariableDecl("mulmod", typeProvider.functionType(strings{"uint256", "uint256", "uint256"}, strings{"uint256"}, FunctionType::Kind::MulMod, false, StateMutability::Pure)),
		magicVariableDecl("now", typeProvider.integerType(256)),
		magicVariableDecl("require", typeProvider.functionType(strings{"bool"}, strings{}, FunctionType::Kind::Require, false, StateMutability::Pure)),
		magicVariableDecl("require", typeProvider.functionType(strings{"bool", "string memory"}, strings{}, FunctionType::Kind::Require, false, StateMutability::Pure)),
		magicVariableDecl("revert", typeProvider.functionType(strings(), strings(), FunctionType::Kind::Revert, false, StateMutability::Pure)),
		magicVariableDecl("revert", typeProvider.functionType(strings{"string memory"}, strings(), FunctionType::Kind::Revert, false, StateMutability::Pure)),
		magicVariableDecl("ripemd160", typeProvider.functionType(strings{"bytes memory"}, strings{"bytes20"}, FunctionType::Kind::RIPEMD160, false, StateMutability::Pure)),
		magicVariableDecl("selfdestruct", typeProvider.functionType(strings{"address payable"}, strings{}, FunctionType::Kind::Selfdestruct)),
		magicVariableDecl("sha256", typeProvider.functionType(strings{"bytes memory"}, strings{"bytes32"}, FunctionType::Kind::SHA256, false, StateMutability::Pure)),
		magicVariableDecl("sha3", typeProvider.functionType(strings{"bytes memory"}, strings{"bytes32"}, FunctionType::Kind::KECCAK256, false, StateMutability::Pure)),
		magicVariableDecl("suicide", typeProvider.functionType(strings{"address payable"}, strings{}, FunctionType::Kind::Selfdestruct)),
		magicVariableDecl("tx", typeProvider.magicType(MagicType::Kind::Transaction)),
		magicVariableDecl("type", typeProvider.functionType(
			strings{"address"} /* accepts any contract type, handled by the type checker */,
			strings{} /* returns a MagicType, handled by the type checker */,
			FunctionType::Kind::MetaType,
			false,
			StateMutability::Pure
		)),
	};
}

GlobalContext::GlobalContext(): m_magicVariables{constructMagicVariables()}
{
}

void GlobalContext::setCurrentContract(ContractDefinition const& _contract)
{
	m_currentContract = &_contract;
}

vector<Declaration const*> GlobalContext::declarations() const
{
	vector<Declaration const*> declarations;
	declarations.reserve(m_magicVariables.size());
	for (ASTPointer<Declaration const> const& variable: m_magicVariables)
		declarations.push_back(variable.get());
	return declarations;
}

MagicVariableDeclaration const* GlobalContext::currentThis() const
{
	if (!m_thisPointer[m_currentContract])
		m_thisPointer[m_currentContract] = make_shared<MagicVariableDeclaration>("this", TypeProvider::get().contractType(*m_currentContract));
	return m_thisPointer[m_currentContract].get();

}

MagicVariableDeclaration const* GlobalContext::currentSuper() const
{
	if (!m_superPointer[m_currentContract])
		m_superPointer[m_currentContract] = make_shared<MagicVariableDeclaration>("super", TypeProvider::get().contractType(*m_currentContract, true));
	return m_superPointer[m_currentContract].get();
}

}
}
