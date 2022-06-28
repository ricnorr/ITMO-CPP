#pragma once

#include <utility>
struct nullopt_t {};

struct in_place_t {};

template <class T, class Enabled = void>
union optional_storage {
  nullopt_t dummy;
  T value;

  constexpr optional_storage() : dummy(nullopt_t()) {}

  template <class... Args>
  constexpr explicit optional_storage(in_place_t, Args&&... args)
      : value(T(std::forward<Args>(args)...)) {}
  constexpr optional_storage(T&& value) : value(std::move(value)) {}
  ~optional_storage() {}
};

template <class T>
union optional_storage<
    T, typename std::enable_if_t<std::is_trivially_destructible_v<T>>> {
  nullopt_t dummy;
  T value;

  template <class... Args>
  constexpr explicit optional_storage(in_place_t, Args&&... args)
      : value(T(std::forward<Args>(args)...)) {}
  constexpr optional_storage() : dummy(nullopt_t()) {}
  constexpr optional_storage(T&& value) : value(std::move(value)) {}
};

template <class T>
struct optional_base_impl {
  optional_storage<T> storage;
  bool init;

  constexpr optional_base_impl() noexcept : init(false) {}

  constexpr optional_base_impl(T&& value)
      : storage(std::move(value)), init(true) {}

  template <class... Args>
  constexpr optional_base_impl(in_place_t tag, Args&&... args)
      : init(true), storage(tag, std::forward<Args>(args)...) {}

  optional_base_impl(const optional_base_impl&) = default;
  optional_base_impl(optional_base_impl&&) = default;
  optional_base_impl& operator=(optional_base_impl&&) = default;
  optional_base_impl& operator=(const optional_base_impl&) = default;
};

template <class T, class Enabled = void>
struct optional_base_trivially_copyable : optional_base_impl<T> {
  using optional_base_impl<T>::optional_base_impl;

  constexpr optional_base_trivially_copyable(
      optional_base_trivially_copyable&& rhs) {
    if (rhs.init) {
      new (&this->storage.value) T(std::move(rhs.storage.value));
      rhs.storage.value.~T();
    }
    this->init = rhs.init;
    rhs.init = false;
  }

  constexpr optional_base_trivially_copyable(
      const optional_base_trivially_copyable& rhs) {
    if (rhs.init) {
      new (&this->storage.value) T(rhs.storage.value);
      this->init = rhs.init;
    }
  }

  optional_base_trivially_copyable&
  operator=(optional_base_trivially_copyable&& rhs) {
    if (this->init) {
      this->storage.value.~T();
    }
    if (rhs.init) {
      new (&this->storage.value) T(std::move(rhs.storage.value));
    }
    this->init = rhs.init;
    return *this;
  }

  optional_base_trivially_copyable&
  operator=(const optional_base_trivially_copyable& rhs) {
    if (this->init) {
      this->storage.value.~T();
    }
    if (rhs.init) {
      new (&this->storage.value) T(rhs.storage.value);
    }
    this->init = rhs.init;
    return *this;
  }
};

template <class T>
struct optional_base_trivially_copyable<
    T, typename std::enable_if_t<std::is_trivially_copyable_v<T>>>
    : optional_base_impl<T> {
  using optional_base_impl<T>::optional_base_impl;

  constexpr optional_base_trivially_copyable(
      optional_base_trivially_copyable&&) = default;
  constexpr optional_base_trivially_copyable(
      const optional_base_trivially_copyable&) = default;
  constexpr optional_base_trivially_copyable&
  operator=(optional_base_trivially_copyable&&) = default;
  constexpr optional_base_trivially_copyable&
  operator=(const optional_base_trivially_copyable&) = default;
};

template <class T, class Enabled = void>
struct optional_base_trivially_destructible
    : optional_base_trivially_copyable<T> {
  using optional_base_trivially_copyable<T>::optional_base_trivially_copyable;

  constexpr optional_base_trivially_destructible(
      optional_base_trivially_destructible&&) = default;
  constexpr optional_base_trivially_destructible(
      const optional_base_trivially_destructible&) = default;
  constexpr optional_base_trivially_destructible&
  operator=(optional_base_trivially_destructible&&) = default;
  constexpr optional_base_trivially_destructible&
  operator=(const optional_base_trivially_destructible&) = default;

  ~optional_base_trivially_destructible() {
    if (this->init) {
      this->storage.value.~T();
    }
  }
};

template <class T>
struct optional_base_trivially_destructible<
    T, typename std::enable_if_t<std::is_trivially_destructible_v<T>>>
    : optional_base_trivially_copyable<T> {
  using optional_base_trivially_copyable<T>::optional_base_trivially_copyable;

  optional_base_trivially_destructible(optional_base_trivially_destructible&&) =
      default;
  optional_base_trivially_destructible(
      const optional_base_trivially_destructible&) = default;
  optional_base_trivially_destructible&
  operator=(optional_base_trivially_destructible&&) = default;
  optional_base_trivially_destructible&
  operator=(const optional_base_trivially_destructible&) = default;
  ~optional_base_trivially_destructible() = default;
};

template <bool>
struct optional_copy_constructible {
  constexpr optional_copy_constructible() = default;
  constexpr optional_copy_constructible(const optional_copy_constructible&) =
      delete;
  constexpr optional_copy_constructible(optional_copy_constructible&&) =
      default;
  constexpr optional_copy_constructible&
  operator=(optional_copy_constructible&&) = default;
  constexpr optional_copy_constructible&
  operator=(const optional_copy_constructible&) = default;
};

template <>
struct optional_copy_constructible<true> {};

template <bool>
struct optional_move_constructible {
  constexpr optional_move_constructible() = default;
  constexpr optional_move_constructible(const optional_move_constructible&) =
      default;
  constexpr optional_move_constructible(optional_move_constructible&&) = delete;
  constexpr optional_move_constructible&
  operator=(optional_move_constructible&&) = default;
  constexpr optional_move_constructible&
  operator=(const optional_move_constructible&) = default;
};

template <>
struct optional_move_constructible<true> {};

template <bool>
struct optional_move_assignable {
  constexpr optional_move_assignable() = default;
  constexpr optional_move_assignable(const optional_move_assignable&) = default;
  constexpr optional_move_assignable(optional_move_assignable&&) = default;
  constexpr optional_move_assignable&
  operator=(optional_move_assignable&&) = delete;
  constexpr optional_move_assignable&
  operator=(const optional_move_assignable&) = default;
};

template <>
struct optional_move_assignable<true> {};

template <bool>
struct optional_copy_assignable {
  constexpr optional_copy_assignable() = default;
  constexpr optional_copy_assignable(const optional_copy_assignable&) = default;
  constexpr optional_copy_assignable(optional_copy_assignable&&) = default;
  constexpr optional_copy_assignable&
  operator=(optional_copy_assignable&&) = default;
  constexpr optional_copy_assignable&
  operator=(const optional_copy_assignable&) = delete;
};

template <>
struct optional_copy_assignable<true> {};

template <typename T>
class optional
    : public optional_base_trivially_destructible<T>,
      public optional_copy_constructible<std::is_copy_constructible_v<T>>,
      public optional_move_constructible<std::is_move_constructible_v<T>>,
      public optional_move_assignable<std::is_move_assignable_v<T>>,
      public optional_copy_assignable<std::is_copy_assignable_v<T>> {
public:
  using optional_base_trivially_destructible<
      T>::optional_base_trivially_destructible;

  constexpr optional(nullopt_t) noexcept {}

  template <typename... Args>
  explicit constexpr optional(in_place_t tag, Args&&... args)
      : optional_base_trivially_destructible<T>(tag,
                                                std::forward<Args>(args)...) {}

  optional& operator=(nullopt_t) noexcept {
    *this = optional(nullopt_t());
    return *this;
  }

  constexpr explicit operator bool() const noexcept {
    return this->init;
  }

  constexpr T& operator*() noexcept {
    return this->storage.value;
  }

  constexpr T const& operator*() const noexcept {
    return this->storage.value;
  }

  constexpr T* operator->() noexcept {
    return &this->storage.value;
  }

  constexpr T const* operator->() const noexcept {
    return &this->storage.value;
  }

  template <typename... Args>
  void emplace(Args&&... args) {
    if (this->init) {
      this->storage.value.~T();
      this->init = false;
    }
    new (&this->storage.value) T(std::forward<Args>(args)...);
    this->init = true;
  }

  void reset() {
    if (this->init) {
      this->storage.value.~T();
    }
    this->init = false;
  }
};

template <typename T>
constexpr bool operator==(optional<T> const& a, optional<T> const& b) {
  if (a.init == b.init) {
    if (a.init) {
      return a.storage.value == b.storage.value;
    }
    return true;
  }
  return false;
}

template <typename T>
constexpr bool operator!=(optional<T> const& a, optional<T> const& b) {
  return !(a == b);
}

template <typename T>
constexpr bool operator<(optional<T> const& a, optional<T> const& b) {
  if (!a.init && b.init) {
    return true;
  }
  if (a.init && b.init && a.storage.value < b.storage.value) {
    return true;
  }
  return false;
}

template <typename T>
constexpr bool operator<=(optional<T> const& a, optional<T> const& b) {
  return (a == b) || (a < b);
}

template <typename T>
constexpr bool operator>(optional<T> const& a, optional<T> const& b) {
  return !(a <= b);
}

template <typename T>
constexpr bool operator>=(optional<T> const& a, optional<T> const& b) {
  return !(a < b);
}