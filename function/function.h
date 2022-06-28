#pragma once

template <typename F>
struct function;

template <typename F, typename Ret, typename... Args>
Ret invoker_func(void* obj, Args... args) {
  return (*static_cast<F*>(obj))(std::forward<Args>(args)...);
}

template <typename F>
void delete_func(void* obj) {
  delete static_cast<F*>(obj);
}

template <typename F>
void* copy_func(void* obj) {
  return new F(*static_cast<F*>(obj));
}

template <typename F>
bool eq_type_info(const std::type_info& type_info) {
  return typeid(F) == type_info;
}

template <typename R, typename... Args>
struct function<R(Args...)> {
  function() noexcept = default;

  function(const function& other) {
    *this = other;
  }

  function(function&& other) noexcept {
    *this = std::move(other);
  }

  template <typename T>
  function(T val)
      : obj(new T(std::move(val))), deleter(&delete_func<T>),
        invoker(&invoker_func<T>), copy(&copy_func<T>),
        type_info(&eq_type_info<T>) {}

  function& operator=(function const& rhs) {
    if (this == &rhs) {
      return *this;
    }
    void* temp_obj = nullptr;
    if (rhs.obj) {
      temp_obj = (*rhs.copy)(rhs.obj);
    }
    clean_function();
    obj = temp_obj;
    deleter = rhs.deleter;
    type_info = rhs.type_info;
    copy = rhs.copy;
    invoker = rhs.invoker;
    return *this;
  }

  function& operator=(function&& rhs) noexcept {
    if (this == &rhs) {
      return *this;
    }
    clean_function();
    std::swap(deleter, rhs.deleter);
    std::swap(copy, rhs.copy);
    std::swap(invoker, rhs.invoker);
    std::swap(obj, rhs.obj);
    std::swap(type_info, rhs.type_info);
    return *this;
  }

  ~function() {
    clean_function();
  }

  explicit operator bool() const noexcept {
    return obj;
  }

  R operator()(Args... args) const {
    if (!invoker) {
      throw std::bad_function_call();
    }
    return (*invoker)(obj, std::forward<Args>(args)...);
  }

  template <typename T>
  T* target() noexcept {
    if (deleter == nullptr) {
      return nullptr;
    }
    if ((*type_info)(typeid(T))) {
      return static_cast<T*>(obj);
    } else {
      return nullptr;
    }
  }

  template <typename T>
  T const* target() const noexcept {
    if (!deleter) {
      return nullptr;
    }
    if ((*type_info)(typeid(T))) {
      return static_cast<T const*>(obj);
    } else {
      return nullptr;
    }
  }

private:
  void clean_function() {
    if (deleter) {
      (*deleter)(obj);
      deleter = nullptr;
      copy = nullptr;
      obj = nullptr;
      invoker = nullptr;
    }
  }

  void (*deleter)(void*) = nullptr;
  R (*invoker)(void*, Args...) = nullptr;
  void* (*copy)(void*) = nullptr;
  void* obj = nullptr;
  bool (*type_info)(const std::type_info&) = nullptr;
};
