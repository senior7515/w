#pragma once
#ifndef WAYWARD_SUPPORT_TYPES_HPP_INCLUDED
#define WAYWARD_SUPPORT_TYPES_HPP_INCLUDED

#include <string>
#include <cstdint>
#include <sstream>

#include <wayward/support/type.hpp>
#include <wayward/support/maybe.hpp>
#include <wayward/support/data_franca/mutator.hpp>
#include <wayward/support/data_franca/spectator.hpp>
#include <wayward/support/datetime/type.hpp>

namespace wayward {
  struct NothingTypeType : IDataTypeFor<NothingType> {
    bool is_nullable() const final { return true; }
    std::string name() const final { return "NothingType"; }
    bool has_value(const NothingType&) const final { return false; }
    bool deserialize_value(NothingType&, const data_franca::ScalarSpectator&) const override { return true; }
    bool serialize_value(const NothingType&, data_franca::ScalarMutator& target) const override { return target << Nothing; }
  };

  const NothingTypeType* build_type(const TypeIdentifier<NothingType>*);

  struct StringType : IDataTypeFor<std::string> {
    bool is_nullable() const final { return false; }
    std::string name() const final { return "std::string"; }

    bool has_value(const std::string& value) const final {
      return true;
    }

    bool deserialize_value(std::string& value, const data_franca::ScalarSpectator& input) const override {
      return input >> value;
    }

    bool serialize_value(const std::string& value, data_franca::ScalarMutator& target) const override {
      target << value;
      return true;
    }
  };

  const StringType* build_type(const TypeIdentifier<std::string>*);

  template <typename T>
  struct NumericType : IDataTypeFor<T> {
    NumericType(std::string name) : name_(std::move(name)) {}
    bool is_nullable() const final { return false; }
    std::string name() const final { return name_; }
    size_t bits() const { return sizeof(T)*8; }
    bool is_signed() const { return std::is_signed<T>::value; }
    bool is_float() const { return std::is_floating_point<T>::value; }

    bool has_value(const T& value) const final { return true; }

    bool deserialize_value(T& value, const data_franca::ScalarSpectator& source) const override {
      if (is_float()) {
        data_franca::Real r;
        if (source >> r) {
          value = static_cast<T>(r);
          return true;
        }
      } else {
        data_franca::Integer n;
        if (source >> n) {
          value = static_cast<T>(n);
          return true;
        }
      }
      return false;
    }

    bool serialize_value(const T& value, data_franca::ScalarMutator& target) const override {
      if (is_float()) {
        target << static_cast<data_franca::Real>(value);
      } else {
        target << static_cast<data_franca::Integer>(value);
      }
      return true;
    }
  private:
    std::string name_;
  };

  const NumericType<std::int32_t>* build_type(const TypeIdentifier<std::int32_t>*);
  const NumericType<std::int64_t>* build_type(const TypeIdentifier<std::int64_t>*);
  const NumericType<std::uint32_t>* build_type(const TypeIdentifier<std::uint32_t>*);
  const NumericType<std::uint64_t>* build_type(const TypeIdentifier<std::uint64_t>*);
  const NumericType<float>* build_type(const TypeIdentifier<float>*);
  const NumericType<double>* build_type(const TypeIdentifier<double>*);

  namespace detail {
    std::string maybe_type_name(const IType* inner);
  }

  template <typename T>
  struct MaybeType : IDataTypeFor<Maybe<T>> {
    MaybeType(const IDataTypeFor<T>* inner_type) : inner_type_(inner_type) {}
    std::string name() const final { return detail::maybe_type_name(inner_type_); }
    bool is_nullable() const { return true; }

    bool has_value(const Maybe<T>& value) const final {
      return static_cast<bool>(value);
    }

    bool deserialize_value(Maybe<T>& value, const data_franca::ScalarSpectator& source) const final {
      if (source) {
        T val;
        inner_type_->deserialize_value(val, source);
        value = std::move(val);
      } else {
        value = Nothing;
      }
      return true;
    }

    bool serialize_value(const Maybe<T>& value, data_franca::ScalarMutator& target) const final {
      if (value) {
        inner_type_->serialize_value(*value, target);
      } else {
        target << Nothing;
      }
      return true;
    }
  private:
    const IDataTypeFor<T>* inner_type_ = nullptr;
  };

  template <typename T>
  const MaybeType<T>* build_type(const TypeIdentifier<Maybe<T>>*) {
    static const MaybeType<T>* p = new MaybeType<T>{get_type<T>()};
    return p;
  }
}

#endif // WAYWARD_SUPPORT_TYPES_HPP_INCLUDED