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

	ArrayType const* bytesType() const noexcept { return &m_bytesType; }
	ArrayType const* bytesMemoryType() const noexcept { return &m_bytesMemoryType; }
	ArrayType const* stringType() const noexcept { return &m_stringType; }
	ArrayType const* stringMemoryType() const noexcept { return &m_stringMemoryType; }

	/// Constructor for a byte array ("bytes") and string.
	ArrayType const* arrayType(DataLocation _location, bool _isString = false);

	/// Constructor for a dynamically sized array type ("type[]")
	ArrayType const* arrayType(DataLocation _location, Type const* _baseType);

	/// Constructor for a fixed-size array type ("type[20]")
	ArrayType const* arrayType(DataLocation _location, Type const* _baseType, u256 const& _length);

	AddressType const* payableAddressType() const noexcept { return addressType(StateMutability::Payable); }
	AddressType const* addressType() const noexcept { return addressType(StateMutability::NonPayable); }
	AddressType const* addressType(StateMutability _mutability) const { return &m_addressType.at(static_cast<size_t>(_mutability)); }

	IntegerType const* intType(unsigned m = 256) const { return &m_intM.at(m - 1); }
	IntegerType const* uintType(unsigned m = 256) const { return &m_uintM.at(m - 1); }
	IntegerType const* integerType(unsigned _bits = 256, IntegerType::Modifier _modifier = IntegerType::Modifier::Unsigned)
	{
		return _modifier == IntegerType::Modifier::Unsigned ? uintType(_bits) : intType(_bits);
	}

	FixedPointType const* fixedType() { return fixedType(128, 18); }
	FixedPointType const* fixedType(unsigned m, unsigned n);

	FixedPointType const* ufixedType() { return ufixedType(128, 18); }
	FixedPointType const* ufixedType(unsigned m, unsigned n);

	FixedPointType const* fixedPointType(unsigned m, unsigned n, FixedPointType::Modifier _modifier)
	{
		return _modifier == FixedPointType::Modifier::Unsigned ? ufixedType(m, n) : fixedType(m, n);
	}

	StringLiteralType const* stringLiteralType(std::string const& literal)
	{
		auto i = m_stringLiteralTypes.find(literal);
		if (i != m_stringLiteralTypes.end())
			return &i->second;
		else
			return &m_stringLiteralTypes.emplace(literal, StringLiteralType{literal}).first->second;
	}

	TupleType const* tupleType(std::vector<Type const*>&& members);
	TupleType const* emptyTupleType() const noexcept { return m_tupleTypes[0].get(); }

	/// @returns a suitably simple type when a type is expected but an error has occured.
	TupleType const* errorType() const noexcept { return emptyTupleType(); }

	ReferenceType const* withLocation(ReferenceType const* _type, DataLocation _location, bool _isPointer);

	/// @returns a copy of @a _type having the same location as this (and is not a pointer type)
	///          if _type is a reference type and an unmodified copy of _type otherwise.
	///          This function is mostly useful to modify inner types appropriately.
	Type const* withLocationIfReference(DataLocation _location, Type const* _type)
	{
		if (auto refType = dynamic_cast<ReferenceType const*>(_type))
			return withLocation(refType, _location, false);

		return _type;
	}

	/// @returns the type of a function.
	FunctionType const* functionType(FunctionDefinition const& _function, bool _isInternal = true);

	/// @returns the accessor function type of a state variable.
	FunctionType const* functionType(VariableDeclaration const& _varDecl);

	/// @returns the function type of an event.
	FunctionType const* functionType(EventDefinition const& _event);

	/// @returns the type of a function type name.
	FunctionType const* functionType(FunctionTypeName const& _typeName);

	/// @returns the function type to be used for a plain type (not derived from a declaration).
	FunctionType const* functionType(
		strings const& _parameterTypes,
		strings const& _returnParameterTypes,
		FunctionType::Kind _kind = FunctionType::Kind::Internal,
		bool _arbitraryParameters = false,
		StateMutability _stateMutability = StateMutability::NonPayable
	);

	/// @returns a highly customized FunctionType, use with care.
	FunctionType const* functionType(
		TypePointers const& _parameterTypes,
		TypePointers const& _returnParameterTypes,
		strings _parameterNames = strings{},
		strings _returnParameterNames = strings{},
		FunctionType::Kind _kind = FunctionType::Kind::Internal,
		bool _arbitraryParameters = false,
		StateMutability _stateMutability = StateMutability::NonPayable,
		Declaration const* _declaration = nullptr,
		bool _gasSet = false,
		bool _valueSet = false,
		bool _bound = false
	);

	RationalNumberType const* rationalNumberType(
		rational const& _value,
		Type const* _compatibleBytesType = nullptr
	);

	ContractType const* contractType(ContractDefinition const& _contract, bool _isSuper = false);

	InaccessibleDynamicType const* inaccessibleDynamicType() const noexcept { return &m_inaccessibleDynamicType; }

	/// @returns the type of an enum instance for given definition, there is one distinct type per enum definition.
	EnumType const* enumType(EnumDefinition const& _enum);

	/// @returns special type for imported modules. These mainly give access to their scope via members.
	ModuleType const* moduleType(SourceUnit const& _source);

	TypeType const* typeType(Type const* _actualType);

	StructType const* structType(StructDefinition const& _struct, DataLocation _location = DataLocation::Storage);

	ModifierType const* modifierType(ModifierDefinition const& _modifierDef);

	MagicType const* magicType(MagicType::Kind _kind);

	MagicType const* metaType(Type const* _type);

	MappingType const* mappingType(Type const* _keyType, Type const* _valueType);

private:
	BoolType m_boolType{};
	InaccessibleDynamicType m_inaccessibleDynamicType{};

	ArrayType m_bytesType{DataLocation::Storage, false};
	ArrayType m_bytesMemoryType{DataLocation::Memory, false};
	ArrayType m_stringType{DataLocation::Storage, true};
	ArrayType m_stringMemoryType{DataLocation::Memory, true};
	std::vector<std::unique_ptr<ArrayType>> m_arrayTypes{};

	std::array<AddressType const, 4> const m_addressType{
		AddressType{StateMutability::Pure},
		AddressType{StateMutability::View},
		AddressType{StateMutability::NonPayable},
		AddressType{StateMutability::Payable},
	};

	std::array<IntegerType, 256> m_intM;        // intM, int
	std::array<IntegerType, 256> m_uintM;       // uintM, uint
	std::array<FixedBytesType, 32> m_bytesM;    // byte, bytesM

	std::map<std::pair<unsigned, unsigned>, std::unique_ptr<FixedPointType>> m_fixedMxN;
	std::map<std::pair<unsigned, unsigned>, std::unique_ptr<FixedPointType>> m_ufixedMxN;

	std::array<MagicType, 4> m_magicTypes{
		MagicType{MagicType::Kind::Block},
		MagicType{MagicType::Kind::Message},
		MagicType{MagicType::Kind::Transaction},
		MagicType{MagicType::Kind::ABI}
		// MetaType is stored seperately
	};

	std::vector<std::unique_ptr<MagicType>> m_metaTypes{};

	std::map<std::string, StringLiteralType> m_stringLiteralTypes{}; // TODO: unique_ptr
	std::vector<std::unique_ptr<TupleType>> m_tupleTypes{};
	std::vector<std::unique_ptr<ReferenceType>> m_referenceTypes{};
	std::vector<std::unique_ptr<FunctionType>> m_functionTypes{};
	std::vector<std::unique_ptr<RationalNumberType>> m_rationalNumberTypes{};
	std::vector<std::unique_ptr<ContractType>> m_contractTypes{};
	std::vector<std::unique_ptr<EnumType>> m_enumTypes{};
	std::vector<std::unique_ptr<ModuleType>> m_moduleTypes{};
	std::vector<std::unique_ptr<TypeType>> m_typeTypes{};
	std::vector<std::unique_ptr<StructType>> m_structTypes{};
	std::vector<std::unique_ptr<ModifierType>> m_modifierTypes{};
	std::vector<std::unique_ptr<MappingType>> m_mappingTypes{};
};

} // namespace solidity
} // namespace dev
