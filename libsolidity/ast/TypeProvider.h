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

/**
 * API for accessing the Solidity Type System.
 *
 * This is the Solidity Compiler's type provider. Use it to request for types. The caller does
 * <b>not</b> own the types.
 *
 * It is not recommended to explicitly instanciate types unless you really know what and why
 * you are doing it.
 */
class TypeProvider
{
public:
	TypeProvider();
	TypeProvider(TypeProvider&&) = default;
	TypeProvider(TypeProvider const&) = delete;
	TypeProvider& operator=(TypeProvider&&) = default;
	TypeProvider& operator=(TypeProvider const&) = delete;
	~TypeProvider() = default;

	/**
	 * Global TypeProvider instance.
	 *
	 * This is only available initially and is going to be factored out in future PRs (TODO),
	 * so we keep the initial PR lean and clean.
	 */
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

	AddressType const* payableAddressType() const noexcept { return &m_payableAddressType; }
	AddressType const* addressType() const noexcept { return &m_addressType; }

	IntegerType const* intType(unsigned m = 256) const
	{
		solAssert((m % 8) == 0, "");
		return &m_intM.at(m / 8 - 1);
	}

	IntegerType const* uintType(unsigned m = 256) const
	{
		solAssert((m % 8) == 0, "");
		return &m_uintM.at(m / 8 - 1);
	}

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

	StringLiteralType const* stringLiteralType(std::string const& literal);

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
	static BoolType const m_boolType;
	static InaccessibleDynamicType const m_inaccessibleDynamicType;

	static ArrayType const m_bytesType;
	static ArrayType const m_bytesMemoryType;
	static ArrayType const m_stringType;
	static ArrayType const m_stringMemoryType;

	static AddressType const m_payableAddressType;
	static AddressType const m_addressType;
	static std::array<IntegerType, 32> const m_intM;
	static std::array<IntegerType, 32> const m_uintM;
	static std::array<FixedBytesType, 32> const m_bytesM;
	static std::array<MagicType, 4> const m_magicTypes;     ///< MagicType's except MetaType

	std::map<std::pair<unsigned, unsigned>, std::unique_ptr<FixedPointType>> m_fixedMxN{};
	std::map<std::pair<unsigned, unsigned>, std::unique_ptr<FixedPointType>> m_ufixedMxN{};
	std::map<std::string, std::unique_ptr<StringLiteralType>> m_stringLiteralTypes{};
	std::vector<std::unique_ptr<ArrayType>> m_arrayTypes{};
	std::vector<std::unique_ptr<MagicType>> m_metaTypes{};
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
