// #pragma once
// #include <memory>
// #include <typeinfo>
// #include <type_traits>
// #include <utility>
// #include <stdexcept>

// namespace ex {

// // --------------------------------------
// // ex::Any - a std::any-like implementation (C++17)
// // --------------------------------------
// class Any {
// public:
//     // tags
//     template <class T>
//     using in_place_type_t = std::in_place_type_t<T>;

//     // ctors / dtor
//     Any() noexcept = default;
//     Any(std::nullptr_t) noexcept {} // for symmetry with reset semantics
//     ~Any() = default;

//     Any(const Any& other) : base_(other.base_ ? other.base_->clone() : nullptr) {}
//     Any(Any&& other) noexcept = default;

//     Any& operator=(const Any& other) {
//         if (this != &other) {
//             base_ = other.base_ ? other.base_->clone() : nullptr;
//         }
//         return *this;
//     }
//     Any& operator=(Any&& other) noexcept = default;

//     // perfect-forwarding ctor (store any decayed type)
//     template <class T,
//               class U = std::decay_t<T>,
//               std::enable_if_t<!std::is_same<U, Any>::value, int> = 0>
//     Any(T&& value) : base_(std::make_unique<Holder<U>>(std::forward<T>(value))) {}

//     // in_place_type ctor
//     template <class T, class... Args>
//     explicit Any(in_place_type_t<T>, Args&&... args)
//         : base_(std::make_unique<Holder<T>>(std::forward<Args>(args)...)) {}

//     // modifiers
//     void reset() noexcept { base_.reset(); }

//     template <class T, class... Args>
//     T& emplace(Args&&... args) {
//         auto ptr = std::make_unique<Holder<T>>(std::forward<Args>(args)...);
//         T& ref = ptr->value;
//         base_ = std::move(ptr);
//         return ref;
//     }

//     void swap(Any& other) noexcept { base_.swap(other.base_); }

//     // observers
//     bool has_value() const noexcept { return static_cast<bool>(base_); }

//     const std::type_info& type() const noexcept {
//         return base_ ? base_->type() : typeid(void);
//     }

// private:
//     struct Base {
//         virtual ~Base() = default;
//         virtual std::unique_ptr<Base> clone() const = 0;
//         virtual const std::type_info& type() const noexcept = 0;
//     };

//     template <class T>
//     struct Holder final : Base {
//         template <class... Args>
//         explicit Holder(Args&&... args) : value(std::forward<Args>(args)...) {}

//         std::unique_ptr<Base> clone() const override {
//             return std::make_unique<Holder<T>>(value);
//         }
//         const std::type_info& type() const noexcept override { return typeid(T); }

//         T value;
//     };

//     // internal typed pointer accessors
//     template <class T>
//     static T* ptr(Any* a) noexcept {
//         if (!a || !a->base_) return nullptr;
//         using U = std::remove_cv_t<std::remove_reference_t<T>>;
//         auto h = dynamic_cast<Holder<U>*>(a->base_.get());
//         if (!h) return nullptr;
//         if constexpr (std::is_reference<T>::value) {
//             return std::addressof(h->value);
//         } else {
//             return std::addressof(h->value);
//         }
//     }

//     std::unique_ptr<Base> base_;

//     // allow any_cast to access internals
//     template <class T>
//     friend T any_cast(Any&);
//     template <class T>
//     friend T any_cast(const Any&);
//     template <class T>
//     friend T any_cast(Any&&);
//     template <class T>
//     friend T* any_cast(Any*) noexcept;
//     template <class T>
//     friend const T* any_cast(const Any*) noexcept;
// };

// // --------------------------------------
// // free any_cast overloads (pointer + ref + rvalue)
// // --------------------------------------

// // pointer forms (no-throw). Return nullptr on mismatch.
// template <class T>
// T* any_cast(Any* a) noexcept {
//     static_assert(!std::is_reference<T>::value,
//                   "any_cast<T*> requires T to be non-reference.");
//     return Any::ptr<T>(a);
// }
// template <class T>
// const T* any_cast(const Any* a) noexcept {
//     static_assert(!std::is_reference<T>::value,
//                   "any_cast<T*> requires T to be non-reference.");
//     return Any::ptr<T>(const_cast<Any*>(a));
// }

// // reference forms (throw std::bad_any_cast on mismatch)
// template <class T>
// T any_cast(Any& a) {
//     using U = std::remove_reference_t<T>;
//     auto p = any_cast<U>(&a);
//     if (!p) throw std::bad_any_cast();
//     if constexpr (std::is_lvalue_reference<T>::value) {
//         return *p;
//     } else { // T is value (non-ref)
//         return *p;
//     }
// }

// template <class T>
// T any_cast(const Any& a) {
//     using U = std::remove_reference_t<T>;
//     auto p = any_cast<U>(const_cast<Any*>(&a));
//     if (!p) throw std::bad_any_cast();
//     if constexpr (std::is_lvalue_reference<T>::value) {
//         return static_cast<const U&>(*p);
//     } else { // value
//         return *p;
//     }
// }

// // rvalue cast (move out value). If mismatch -> throw.
// template <class T>
// T any_cast(Any&& a) {
//     using U = std::remove_reference_t<T>;
//     auto p = any_cast<U>(&a);
//     if (!p) throw std::bad_any_cast();
//     if constexpr (std::is_rvalue_reference<T>::value) {
//         return std::move(*p);
//     } else {
//         return *p;
//     }
// }

// } // namespace ex
