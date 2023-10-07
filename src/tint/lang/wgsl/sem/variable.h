// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_TINT_LANG_WGSL_SEM_VARIABLE_H_
#define SRC_TINT_LANG_WGSL_SEM_VARIABLE_H_

#include <optional>
#include <utility>
#include <vector>

#include "src/tint/api/common/override_id.h"

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/core/access.h"
#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/parameter_usage.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/wgsl/ast/parameter.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/utils/containers/unique_vector.h"

// Forward declarations
namespace tint::ast {
class IdentifierExpression;
class Parameter;
class Variable;
}  // namespace tint::ast
namespace tint::sem {
class CallTarget;
class VariableUser;
}  // namespace tint::sem

namespace tint::sem {

/// Variable is the base class for local variables, global variables and
/// parameters.
class Variable : public Castable<Variable, Node> {
  public:
    /// Constructor
    /// @param declaration the AST declaration node
    explicit Variable(const ast::Variable* declaration);

    /// Destructor
    ~Variable() override;

    /// @returns the AST declaration node
    const ast::Variable* Declaration() const { return declaration_; }

    /// @param type the variable type
    void SetType(const core::type::Type* type) { type_ = type; }

    /// @returns the canonical type for the variable
    const core::type::Type* Type() const { return type_; }

    /// @param stage the evaluation stage for an expression of this variable type
    void SetStage(core::EvaluationStage stage) { stage_ = stage; }

    /// @returns the evaluation stage for an expression of this variable type
    core::EvaluationStage Stage() const { return stage_; }

    /// @param space the variable address space
    void SetAddressSpace(core::AddressSpace space) { address_space_ = space; }

    /// @returns the address space for the variable
    core::AddressSpace AddressSpace() const { return address_space_; }

    /// @param access the variable access control type
    void SetAccess(core::Access access) { access_ = access; }

    /// @returns the access control for the variable
    core::Access Access() const { return access_; }

    /// @param value the constant value for the variable. May be null
    void SetConstantValue(const core::constant::Value* value) { constant_value_ = value; }

    /// @return the constant value of this expression
    const core::constant::Value* ConstantValue() const { return constant_value_; }

    /// Sets the variable initializer expression.
    /// @param initializer the initializer expression to assign to this variable.
    void SetInitializer(const ValueExpression* initializer) { initializer_ = initializer; }

    /// @returns the variable initializer expression, or nullptr if the variable
    /// does not have one.
    const ValueExpression* Initializer() const { return initializer_; }

    /// @returns the expressions that use the variable
    VectorRef<const VariableUser*> Users() const { return users_; }

    /// @param user the user to add
    void AddUser(const VariableUser* user) { users_.Push(user); }

  private:
    const ast::Variable* const declaration_ = nullptr;
    const core::type::Type* type_ = nullptr;
    core::EvaluationStage stage_ = core::EvaluationStage::kRuntime;
    core::AddressSpace address_space_ = core::AddressSpace::kUndefined;
    core::Access access_ = core::Access::kUndefined;
    const core::constant::Value* constant_value_ = nullptr;
    const ValueExpression* initializer_ = nullptr;
    tint::Vector<const VariableUser*, 8> users_;
};

/// LocalVariable is a function-scope variable
class LocalVariable final : public Castable<LocalVariable, Variable> {
  public:
    /// Constructor
    /// @param declaration the AST declaration node
    /// @param statement the statement that declared this local variable
    LocalVariable(const ast::Variable* declaration, const sem::Statement* statement);

    /// Destructor
    ~LocalVariable() override;

    /// @returns the statement that declares this local variable
    const sem::Statement* Statement() const { return statement_; }

    /// Sets the Type, Function or Variable that this local variable shadows
    /// @param shadows the Type, Function or Variable that this variable shadows
    void SetShadows(const CastableBase* shadows) { shadows_ = shadows; }

    /// @returns the Type, Function or Variable that this local variable shadows
    const CastableBase* Shadows() const { return shadows_; }

  private:
    const sem::Statement* const statement_;
    const CastableBase* shadows_ = nullptr;
};

/// GlobalVariable is a module-scope variable
class GlobalVariable final : public Castable<GlobalVariable, Variable> {
  public:
    /// Constructor
    /// @param declaration the AST declaration node
    explicit GlobalVariable(const ast::Variable* declaration);

    /// Destructor
    ~GlobalVariable() override;

    /// @param binding_point the resource binding point for the parameter
    void SetBindingPoint(std::optional<tint::BindingPoint> binding_point) {
        binding_point_ = binding_point;
    }

    /// @returns the resource binding point for the variable
    std::optional<tint::BindingPoint> BindingPoint() const { return binding_point_; }

    /// @param id the constant identifier to assign to this variable
    void SetOverrideId(OverrideId id) { override_id_ = id; }

    /// @returns the pipeline constant ID associated with the variable
    tint::OverrideId OverrideId() const { return override_id_; }

    /// @param location the location value for the parameter, if set
    /// @note a GlobalVariable generally doesn't have a `location` in WGSL, as it isn't allowed by
    /// the spec. The location maybe attached by transforms such as CanonicalizeEntryPointIO.
    void SetLocation(std::optional<uint32_t> location) { location_ = location; }

    /// @returns the location value for the parameter, if set
    std::optional<uint32_t> Location() const { return location_; }

    /// @param index the index value for the parameter, if set
    void SetIndex(std::optional<uint32_t> index) { index_ = index; }

    /// @returns the index value for the parameter, if set
    std::optional<uint32_t> Index() const { return index_; }

  private:
    std::optional<tint::BindingPoint> binding_point_;
    tint::OverrideId override_id_;
    std::optional<uint32_t> location_;
    std::optional<uint32_t> index_;
};

/// Parameter is a function parameter
class Parameter final : public Castable<Parameter, Variable> {
  public:
    /// Constructor
    /// @param declaration the AST declaration node
    /// @param index the index of the parameter in the function
    /// @param type the variable type
    /// @param usage the parameter usage
    Parameter(const ast::Parameter* declaration,
              uint32_t index = 0,
              const core::type::Type* type = nullptr,
              core::ParameterUsage usage = core::ParameterUsage::kNone);

    /// Destructor
    ~Parameter() override;

    /// @returns the AST declaration node
    const ast::Parameter* Declaration() const {
        return static_cast<const ast::Parameter*>(Variable::Declaration());
    }

    /// @param index the index value for the parameter, if set
    void SetIndex(uint32_t index) { index_ = index; }

    /// @return the index of the parameter in the function
    uint32_t Index() const { return index_; }

    /// @param usage the semantic usage for the parameter
    void SetUsage(core::ParameterUsage usage) { usage_ = usage; }

    /// @returns the semantic usage for the parameter
    core::ParameterUsage Usage() const { return usage_; }

    /// @param owner the CallTarget owner of this parameter
    void SetOwner(const CallTarget* owner) { owner_ = owner; }

    /// @returns the CallTarget owner of this parameter
    const CallTarget* Owner() const { return owner_; }

    /// Sets the Type, Function or Variable that this local variable shadows
    /// @param shadows the Type, Function or Variable that this variable shadows
    void SetShadows(const CastableBase* shadows) { shadows_ = shadows; }

    /// @returns the Type, Function or Variable that this local variable shadows
    const CastableBase* Shadows() const { return shadows_; }

    /// @param binding_point the resource binding point for the parameter
    void SetBindingPoint(std::optional<tint::BindingPoint> binding_point) {
        binding_point_ = binding_point;
    }

    /// @returns the resource binding point for the parameter
    std::optional<tint::BindingPoint> BindingPoint() const { return binding_point_; }

    /// @param location the location value for the parameter, if set
    void SetLocation(std::optional<uint32_t> location) { location_ = location; }

    /// @returns the location value for the parameter, if set
    std::optional<uint32_t> Location() const { return location_; }

  private:
    uint32_t index_ = 0;
    core::ParameterUsage usage_ = core::ParameterUsage::kNone;
    CallTarget const* owner_ = nullptr;
    const CastableBase* shadows_ = nullptr;
    std::optional<tint::BindingPoint> binding_point_;
    std::optional<uint32_t> location_;
};

/// VariableUser holds the semantic information for an identifier expression
/// node that resolves to a variable.
class VariableUser final : public Castable<VariableUser, ValueExpression> {
  public:
    /// Constructor
    /// @param declaration the AST identifier node
    /// @param stage the evaluation stage for an expression of this variable type
    /// @param statement the statement that owns this expression
    /// @param constant the constant value of the expression. May be null
    /// @param variable the semantic variable
    VariableUser(const ast::IdentifierExpression* declaration,
                 core::EvaluationStage stage,
                 Statement* statement,
                 const core::constant::Value* constant,
                 sem::Variable* variable);
    ~VariableUser() override;

    /// @returns the variable that this expression refers to
    const sem::Variable* Variable() const { return variable_; }

  private:
    const sem::Variable* const variable_;
};

/// A pair of sem::Variables. Can be hashed.
typedef std::pair<const Variable*, const Variable*> VariablePair;

}  // namespace tint::sem

namespace std {

/// Custom std::hash specialization for VariablePair
template <>
class hash<tint::sem::VariablePair> {
  public:
    /// @param i the variable pair to create a hash for
    /// @return the hash value
    inline std::size_t operator()(const tint::sem::VariablePair& i) const {
        return Hash(i.first, i.second);
    }
};

}  // namespace std

#endif  // SRC_TINT_LANG_WGSL_SEM_VARIABLE_H_
