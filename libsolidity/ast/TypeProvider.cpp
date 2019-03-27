#include <libsolidity/ast/TypeProvider.h>
#include "../../../../sandbox/TypeTraits.h"

using namespace std;
using namespace dev;
using namespace solidity;

template <size_t... N>
constexpr std::array<IntegerType, sizeof...(N)> createIntegerTypes(IntegerType::Modifier _modifier, std::index_sequence<N...>)
{
	return dev::make_array<IntegerType>(IntegerType(static_cast<unsigned>(N) + 1, _modifier)...);
}

template <size_t... N>
constexpr std::array<FixedBytesType, sizeof...(N)> createFixedBytesTypes(std::index_sequence<N...>)
{
	return dev::make_array<FixedBytesType>(FixedBytesType(static_cast<unsigned>(N) + 1)...);
}

TypeProvider::TypeProvider():
	m_intM{createIntegerTypes(IntegerType::Modifier::Signed, std::make_index_sequence<256>{})},
	m_uintM{createIntegerTypes(IntegerType::Modifier::Unsigned, std::make_index_sequence<256>{})},
	m_bytesM{createFixedBytesTypes(std::make_index_sequence<32>{})}
{
}

Type const* TypeProvider::fromElementaryTypeName(ElementaryTypeNameToken const& _type)
{
	solAssert(TokenTraits::isElementaryTypeName(_type.token()),
		"Expected an elementary type name but got " + _type.toString()
	);

	unsigned const m = _type.firstNumber();
	unsigned const n = _type.secondNumber();

	switch (_type.token())
	{
	case Token::IntM:
		return intType(m);
	case Token::UIntM:
		return uintType(m);
	case Token::Byte:
		return byteType();
	case Token::BytesM:
		return fixedBytesType(m);
	case Token::FixedMxN:
		return fixedType(m, n);
	case Token::UFixedMxN:
		return ufixedType(m, n);
	case Token::Int:
		return intType();
	case Token::UInt:
		return uintType();
	case Token::Fixed:
		return fixedType();
	case Token::UFixed:
		return ufixedType();
	case Token::Address:
		return addressType();
	case Token::Bool:
		return boolType();
	case Token::Bytes:
		return bytesType();
	case Token::String:
		return stringType();
	default:
		solAssert(
			false,
			"Unable to convert elementary typename " + _type.toString() + " to type."
		);
	}
}

