#pragma once

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/Types.h>

#include <array>
#include <map>
#include <memory>
#include <utility>

namespace dev
{
namespace solidity
{

class TypeProvider
{
public:
	TypeProvider();
	TypeProvider(TypeProvider&&) = default;
	TypeProvider(TypeProvider const&) = delete;
	TypeProvider& operator=(TypeProvider&&) = default;
	TypeProvider& operator=(TypeProvider const&) = delete;
	~TypeProvider() = default;

	/// TODO This is just a quick'n'dirty workaround to get code changing. Properly pass a non-global instance ASAP.
	static TypeProvider& get()
	{
		static TypeProvider _provider;
		return _provider;
	}

	Type const* fromElementaryTypeName(ElementaryTypeNameToken const& _type);

	BoolType const* boolType() const noexcept { return &m_boolType; }

	FixedBytesType const* byteType() { return fixedBytesType(1); }
	FixedBytesType const* fixedBytesType(unsigned m) { return &m_bytesM.at(m - 1); }
	ArrayType const* bytesType() { return &m_bytesType; }
	ArrayType const* stringType() const noexcept { return &m_stringType; }

	AddressType const* payableAddressType() const noexcept { return &m_payableAddressType; }
	AddressType const* addressType() const noexcept { return &m_addressType; }
	IntegerType const* intType(unsigned m = 256) const { return &m_intM.at(m - 1); }
	IntegerType const* uintType(unsigned m = 256) const { return &m_uintM.at(m - 1); }

	FixedPointType const* fixedType() { return fixedType(128, 18); }

	FixedPointType const* fixedType(unsigned m, unsigned n) {
		auto i = m_fixedMxN.find(std::make_pair(m, n));
		if (i != m_fixedMxN.end())
			return i->second.get();

		return m_fixedMxN.emplace(
			std::make_pair(m, n),
			std::make_unique<FixedPointType>(
				m,
				n,
				FixedPointType::Modifier::Signed
			)
		).first->second.get();
	}

	FixedPointType const* ufixedType() { return ufixedType(128, 18); }

	FixedPointType const* ufixedType(unsigned m, unsigned n) {
		auto i = m_fixedMxN.find(std::make_pair(m, n));
		if (i != m_fixedMxN.end())
			return i->second.get();

		return m_fixedMxN.emplace(
			std::make_pair(m, n),
			std::make_unique<FixedPointType>(
				m,
				n,
				FixedPointType::Modifier::Unsigned
			)
		).first->second.get();
	}

	StringLiteralType const* stringLiteralType(std::string const& literal)
	{
		auto i = m_stringLiteralTypes.find(literal);
		if (i != m_stringLiteralTypes.end())
			return &i->second;
		else
			return &m_stringLiteralTypes.emplace(literal, StringLiteralType{literal}).first->second;
	}

	TupleType const* emptyTupleType() const noexcept { return &m_emptyTupleType; }

	ReferenceType const* withLocation(ReferenceType const* _type, DataLocation _location, bool _isPointer)
	{
		if (_type->location() == _location && _type->isPointer() == _isPointer)
			return _type;

		// TODO: re-use existing objects if present (requires deep equality check)

		m_referenceTypes.emplace_back(_type->copyForLocation(_location, _isPointer));
		return m_referenceTypes.back().get();
	}

	/// @returns a copy of @a _type having the same location as this (and is not a pointer type)
	///          if _type is a reference type and an unmodified copy of _type otherwise.
	///          This function is mostly useful to modify inner types appropriately.
	Type const* withLocationIfReference(Type const* _type, DataLocation _location)
	{
		if (auto rt = dynamic_cast<ReferenceType const*>(_type))
			return withLocation(rt, _location, false);

		return _type;
	}

private:
	BoolType m_boolType{};
	ArrayType m_bytesType{DataLocation::Storage};
	ArrayType m_stringType{DataLocation::Storage, true};
	AddressType m_payableAddressType{StateMutability::Payable};
	AddressType m_addressType{StateMutability::NonPayable};
	std::array<IntegerType, 256> m_intM;        // intM, int
	std::array<IntegerType, 256> m_uintM;       // uintM, uint
	std::array<FixedBytesType, 32> m_bytesM;    // byte, bytesM
	std::map<std::pair<unsigned, unsigned>, std::unique_ptr<FixedPointType>> m_fixedMxN;
	std::map<std::pair<unsigned, unsigned>, std::unique_ptr<FixedPointType>> m_ufixedMxN;
	std::map<std::string, StringLiteralType> m_stringLiteralTypes;
	TupleType m_emptyTupleType{};
	std::vector<std::unique_ptr<ReferenceType>> m_referenceTypes;

	// more to come ...
};

} // namespace solidity
} // namespace dev
