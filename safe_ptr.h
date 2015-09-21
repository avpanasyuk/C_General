#ifndef SAFE_PTR_HPP_INCLUDED
#define SAFE_PTR_HPP_INCLUDED

/**
  * @file
  * @author Alexander Panasyuk
  */

#include "Error.h"

namespace avp {
  template<typename T>
  struct safe_ptr {
    safe_ptr(T *p_ = nullptr): p(p_) {}
    safe_ptr &operator=(T *p_) { p = p_; return *this; }
    T &operator*() const { AVP_ASSERT(p != nullptr); return *p; }
    T *operator-> () const { return &(this->operator*()); }
    T &operator[] (size_t index) { AVP_ASSERT(p != nullptr); return p[index]; }
    // T *operator->() const { AVP_ASSERT(p != nullptr); return p; }
    T *get() const { return p; }
    operator T *() const { return p; } // we can return pointer value if it is nullptr, just not dereference it.
    bool operator==(T *p_) const { return p==p_; }
    bool operator==(decltype(nullptr) p_) const { return p==p_; }
  protected:
    T *p;
  }; // safe_ptr
} // namespace avp



#if 0
#include <memory>
#include <type_traits>
#include <exception>

template<typename T>
class safe_ptr {
  template <typename> friend class safe_ptr;
public:
  typedef T element_type;

  safe_ptr() : impl_(std::make_shared<T>()) {}

  safe_ptr(const safe_ptr<T>& other) : impl_(other.impl_) {}

  template<typename U>
  safe_ptr(const safe_ptr<U>& other, typename std::enable_if<std::is_convertible<U *, T *>::value, void *>::type = 0) : impl_(other.impl_) {}

  template<typename U>
  safe_ptr(const U &impl, typename std::enable_if<std::is_convertible<typename std::add_pointer<U>::type, T *>::value, void>::type* = 0)
    : impl_(std::make_shared<U>(impl)) {}

  template<typename U, typename D>
  safe_ptr(const U &impl, D dtor, typename std::enable_if<std::is_convertible<typename std::add_pointer<U>::type, T *>::value, void>::type* = 0)
    : impl_(new U(impl), dtor) {}

  template<typename U>
  safe_ptr(U&& impl, typename std::enable_if<std::is_convertible<typename std::add_pointer<U>::type, T *>::value, void>::type* = 0)
    : impl_(std::make_shared<U>(std::forward<U>(impl))) {}

  template<typename U, typename D>
  safe_ptr(U&& impl, D dtor, typename std::enable_if<std::is_convertible<typename std::add_pointer<U>::type, T *>::value, void>::type* = 0)
    : impl_(new U(std::forward<U>(impl)), dtor) {}

  template<typename U>
  explicit safe_ptr(const std::shared_ptr<U>& impl, typename std::enable_if<std::is_convertible<U *, T *>::value, void *>::type = 0) : impl_(impl) {
    if(!impl_)
      throw std::invalid_argument("impl");
  }

  template<typename U>
  explicit safe_ptr(std::shared_ptr<U>&& impl, typename std::enable_if<std::is_convertible<U *, T *>::value, void *>::type = 0) : impl_(std::move(impl)) {
    if(!impl_)
      throw std::invalid_argument("impl");
  }

  template<typename U>
  explicit safe_ptr(U *impl, typename std::enable_if<std::is_convertible<U *, T *>::value, void *>::type = 0) : impl_(impl) {
    if(!impl_)
      throw std::invalid_argument("impl");
  }

  template<typename U, typename D>
  explicit safe_ptr(U *impl, D dtor, typename std::enable_if<std::is_convertible<U *, T *>::value, void *>::type = 0) : impl_(impl, dtor) {
    if(!impl_)
      throw std::invalid_argument("impl");
  }

  template<typename U>
  typename std::enable_if<std::is_convertible<U *, T *>::value, safe_ptr<T>&>::type
  operator=(const safe_ptr<U>& other) {
    safe_ptr<T> temp(other);
    temp.swap(*this);
    return *this;
  }

  template <typename U>
  typename std::enable_if<std::is_convertible<typename std::add_pointer<U>::type, T *>::value, safe_ptr<T>&>::type
  operator=(U&& impl) {
    safe_ptr<T> temp(std::forward<T>(impl));
    temp.swap(*this);
    return *this;
  }

  T &operator*() const { return *impl_.get();}

  T *operator->() const { return impl_.get();}

  T *get() const { return impl_.get();}

  bool unique() const { return impl_.unique();}

  long use_count() const { return impl_.use_count();}

  void swap(safe_ptr &other) { impl_.swap(other.impl_); }

  operator std::shared_ptr<T>() const { return impl_;}

  template<class U>
  bool owner_before(const safe_ptr<T>& ptr) { return impl_.owner_before(ptr.impl_); }

  template<class U>
  bool owner_before(const std::shared_ptr<U>& ptr) { return impl_.owner_before(ptr); }

  template<class D, class U>
  D *get_deleter(safe_ptr<U> const &ptr) { return impl_.get_deleter(); }

private:
  std::shared_ptr<T> impl_;
};

template<class T, class U>
bool operator==(const safe_ptr<T>& a, const safe_ptr<U>& b) {
  return a.get() == b.get();
}

template<class T, class U>
bool operator!=(const safe_ptr<T>& a, const safe_ptr<U>& b) {
  return a.get() != b.get();
}

template<class T, class U>
bool operator<(const safe_ptr<T>& a, const safe_ptr<U>& b) {
  return a.get() < b.get();
}

template<class T, class U>
bool operator>(const safe_ptr<T>& a, const safe_ptr<U>& b) {
  return a.get() > b.get();
}

template<class T, class U>
bool operator>=(const safe_ptr<T>& a, const safe_ptr<U>& b) {
  return a.get() >= b.get();
}

template<class T, class U>
bool operator<=(const safe_ptr<T>& a, const safe_ptr<U>& b) {
  return a.get() <= b.get();
}

template<class E, class T, class U>
std::basic_ostream<E, T>& operator<<(std::basic_ostream<E, T>& out, const safe_ptr<U>& p) {
  return out << p.get();
}

template<class T>
void swap(safe_ptr<T>& a, safe_ptr<T>& b) {
  a.swap(b);
}

template<class T>
T *get_pointer(safe_ptr<T> const &p) {
  return p.get();
}

template <class T, class U>
safe_ptr<T> static_pointer_cast(const safe_ptr<U>& p) {
  return safe_ptr<T>(std::static_pointer_cast<T>(std::shared_ptr<U>(p)));
}

template <class T, class U>
safe_ptr<T> const_pointer_cast(const safe_ptr<U>& p) {
  return safe_ptr<T>(std::const_pointer_cast<T>(std::shared_ptr<U>(p)));
}

template <class T, class U>
safe_ptr<T> dynamic_pointer_cast(const safe_ptr<U>& p) {
  auto temp = std::dynamic_pointer_cast<T>(std::shared_ptr<U>(p));
  if(!temp)
    throw std::bad_cast();
  return safe_ptr<T>(temp);
}

template<typename T>
safe_ptr<T> make_safe() {
  return safe_ptr<T>();
}

template<typename T, typename P0>
safe_ptr<T> make_safe(P0&& p0) {
  return safe_ptr<T>(std::make_shared<T>(std::forward<P0>(p0)));
}

template<typename T, typename P0, typename P1>
safe_ptr<T> make_safe(P0&& p0, P1&& p1) {
  return safe_ptr<T>(std::make_shared<T>(std::forward<P0>(p0), std::forward<P1>(p1)));
}

template<typename T, typename P0, typename P1, typename P2>
safe_ptr<T> make_safe(P0&& p0, P1&& p1, P2&& p2) {
  return safe_ptr<T>(std::make_shared<T>(std::forward<P0>(p0), std::forward<P1>(p1), std::forward<P2>(p2)));
}

template<typename T, typename P0, typename P1, typename P2, typename P3>
safe_ptr<T> make_safe(P0&& p0, P1&& p1, P2&& p2, P3&& p3) {
  return safe_ptr<T>(std::make_shared<T>(std::forward<P0>(p0), std::forward<P1>(p1), std::forward<P2>(p2), std::forward<P3>(p3)));
}

template<typename T, typename P0, typename P1, typename P2, typename P3, typename P4>
safe_ptr<T> make_safe(P0&& p0, P1&& p1, P2&& p2, P3&& p3, P4&&) {
  return safe_ptr<T>(std::make_shared<T>(std::forward<P0>(p0), std::forward<P1>(p1), std::forward<P2>(p2), std::forward<P3>(p3), std::forward<P3>(p4)));
}
#endif


#endif /* SAFE_PTR_HPP_INCLUDED */
