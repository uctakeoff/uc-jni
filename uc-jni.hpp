/**
uc::jni <https://github.com/uctakeoff/uc-jni>
Copyright (c) 2018, Kentaro Ushiyama
This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/
#ifndef UC_JNI_HPP
#define UC_JNI_HPP
#define UC_JNI_VERSION "1.3.1"
#define UC_JNI_VERSION_NUM 0x010301

#include <jni.h>
#include <memory>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <algorithm>

namespace uc {
namespace jni {

//*************************************************************************************************
// Traits
//*************************************************************************************************

template <typename T, typename U> using is_base_ptr_of = std::is_base_of<std::remove_pointer_t<T>, std::remove_pointer_t<U>>;
template <typename T> using is_derived_from_jobject = is_base_ptr_of<jobject, T>;
template <typename T> using is_derived_from_jobjectArray = is_base_ptr_of<jobjectArray, T>;

template<typename T> struct is_primitive_type : std::false_type {};
template<> struct is_primitive_type<jboolean> : std::true_type {};
template<> struct is_primitive_type<jbyte> : std::true_type {};
template<> struct is_primitive_type<jchar> : std::true_type {};
template<> struct is_primitive_type<jshort> : std::true_type {};
template<> struct is_primitive_type<jint> : std::true_type {};
template<> struct is_primitive_type<jlong> : std::true_type {};
template<> struct is_primitive_type<jfloat> : std::true_type {};
template<> struct is_primitive_type<jdouble> : std::true_type {};

template<typename T> struct is_primitive_array_type : std::false_type {};
template<> struct is_primitive_array_type<jbooleanArray> : std::true_type {};
template<> struct is_primitive_array_type<jbyteArray> : std::true_type {};
template<> struct is_primitive_array_type<jcharArray> : std::true_type {};
template<> struct is_primitive_array_type<jshortArray> : std::true_type {};
template<> struct is_primitive_array_type<jintArray> : std::true_type {};
template<> struct is_primitive_array_type<jlongArray> : std::true_type {};
template<> struct is_primitive_array_type<jfloatArray> : std::true_type {};
template<> struct is_primitive_array_type<jdoubleArray> : std::true_type {};

template<typename T, std::enable_if_t<is_derived_from_jobject<T>::value, std::nullptr_t> = nullptr> 
struct array_impl : public std::remove_pointer_t<jobjectArray>
{
    using value_type = T;
};
template<typename T> using array = array_impl<T>*;

// uc-jni exception
class vm_exception : public std::exception {};

//*************************************************************************************************
// JavaVM
//*************************************************************************************************

inline JavaVM* java_vm(JavaVM* init = nullptr) noexcept
{
    static JavaVM* singleton = init;
    return singleton;
}

//*************************************************************************************************
// JNIEnv
//*************************************************************************************************

inline JNIEnv* env() noexcept
{
    struct vm_attacher
    {
        vm_attacher()
        {
            java_vm()->AttachCurrentThread(&env, nullptr);
        }
        ~vm_attacher()
        {
            java_vm()->DetachCurrentThread();
        }
        JNIEnv* env{};
    };
    thread_local vm_attacher instance {};
    return instance.env;
}


//*************************************************************************************************
// Global and Local References
//*************************************************************************************************

template <typename T, std::enable_if_t<is_primitive_type<T>::value, std::nullptr_t> = nullptr>
constexpr T to_native_ref(T obj) noexcept
{
    return obj;
}
template <typename T, std::enable_if_t<is_derived_from_jobject<T>::value, std::nullptr_t> = nullptr>
constexpr T to_native_ref(T obj) noexcept
{
    return obj;
}
template <typename T, std::enable_if_t<is_derived_from_jobject<typename T::element_type*>::value, std::nullptr_t> = nullptr>
typename T::element_type* to_native_ref(const T& obj) noexcept
{
    return obj.get();
}

template <typename JType> using native_ref = decltype(to_native_ref<JType>({}));


template <typename JType> struct local_ref_deleter
{
    void operator()(JType p) const noexcept
    {
        env()->DeleteLocalRef(p);
    }
};
template <typename JType> using local_ref = std::unique_ptr<std::remove_pointer_t<JType>, local_ref_deleter<JType>>;
template <typename JType> local_ref<JType> make_local(JType obj) noexcept
{
    return local_ref<JType>{obj};
}
/*
template <typename JType> using global_ref = std::shared_ptr<std::remove_pointer_t<JType>>;
template <typename JType> global_ref<native_ref<JType>> make_global(const JType& obj)
{
    return global_ref<native_ref<JType>>(static_cast<native_ref<JType>>(env()->NewGlobalRef(to_native_ref(obj))), [](native_ref<JType> p) { env()->DeleteGlobalRef(p); });
}
*/
template <typename JType> class global_ref
{
    using impl_type = std::shared_ptr<std::remove_pointer_t<JType>>;
public:
    using element_type = typename impl_type::element_type;

    constexpr global_ref() noexcept = default;
    global_ref(const global_ref&) noexcept = default;
    global_ref& operator=(const global_ref&) noexcept = default;
    ~global_ref() = default;

    template <typename T> explicit global_ref(const T& obj) : impl(make_impl(obj))
    {
    }
    template <typename T> global_ref& operator=(const T& obj)
    {
        impl = make_impl(obj);
        return *this;
    }
    explicit operator bool() const noexcept
    {
        return static_cast<bool>(impl);
    }
	element_type* operator->() const noexcept 
    {
        return impl.get();
    }
	element_type& operator *() const noexcept 
    {
        return *impl;
    }
    void reset()
    {
        impl.reset();
    }
    void swap(global_ref& x) noexcept
    {
        return impl.swap(x.impl);
    }
    element_type* get() const noexcept
    {
        return impl.get();
    }
private:
    template <typename T, std::enable_if_t<std::is_same<native_ref<T>, JType>::value, std::nullptr_t> = nullptr>
    static impl_type make_impl(const T& obj)
    {
        return impl_type(static_cast<JType>(env()->NewGlobalRef(to_native_ref(obj))), [](JType p) { env()->DeleteGlobalRef(p); });
    }
    impl_type impl;
};
template <typename JType> global_ref<native_ref<JType>> make_global(const JType& obj)
{
    return global_ref<native_ref<JType>>(obj);
}


//*************************************************************************************************
// Weak Global References
//*************************************************************************************************

template <typename JType> class weak_ref
{
    using impl_type = std::shared_ptr<std::remove_pointer_t<jweak>>;
public:
    using element_type = typename impl_type::element_type;

    weak_ref() = default;
    weak_ref(const weak_ref&) = default;
    weak_ref& operator=(const weak_ref&) = default;
    ~weak_ref() = default;

    template <typename T> explicit weak_ref(const T& obj) : impl(make_weak(obj))
    {
    }
    template <typename T> weak_ref& operator=(const T& obj)
    {
        impl = make_weak(obj);
        return *this;
    }
    void reset() noexcept
    {
        impl.reset();
    }
    void swap(weak_ref& x) noexcept
    {
        return impl.swap(x.impl);
    }
    local_ref<JType> lock() const
    {
        return make_local(static_cast<JType>(env()->NewLocalRef(impl.get())));
    }
    bool expired() const
    {
        return env()->IsSameObject(impl.get(), nullptr) == JNI_TRUE;
    }
    template <typename T> bool is_same(const T& obj) const
    {
        return env()->IsSameObject(impl.get(), to_native_ref(obj)) == JNI_TRUE;
    }

private:
    template <typename T, std::enable_if_t<std::is_same<native_ref<T>, JType>::value, std::nullptr_t> = nullptr>
    static impl_type make_weak(const T& obj)
    {
        return impl_type(env()->NewWeakGlobalRef(to_native_ref(obj)), [](jweak p) { env()->DeleteWeakGlobalRef(p); });
    }
    impl_type impl;
};


//*************************************************************************************************
// Resolve Classes
//*************************************************************************************************

template <typename T> constexpr decltype(auto) fqcn() noexcept
{
    return std::remove_pointer_t<T>::fqcn();
}
#define DEFINE_FQCN(type, sign) template <> constexpr decltype(auto) fqcn<type>() noexcept { return #sign; }
DEFINE_FQCN(jobject,    java/lang/Object)
DEFINE_FQCN(jstring,    java/lang/String)
DEFINE_FQCN(jclass,     java/lang/Class)
DEFINE_FQCN(jthrowable, java/lang/Throwable)
#undef DEFINE_FQCN

inline local_ref<jclass> find_class_native(const char* fqcn)
{
    auto o = env()->FindClass(fqcn);
    if (env()->ExceptionCheck()) throw vm_exception();
    return make_local(o);
}

/*
FAQ: Why didn't FindClass find my class? <https://developer.android.com/training/articles/perf-jni.html#faq_FindClass>
> Cache a reference to the ClassLoader object somewhere handy, and issue loadClass calls directly. This requires some effort.

References: FindClass from any thread in Android JNI <https://stackoverflow.com/questions/13263340/findclass-from-any-thread-in-android-jni>
*/
namespace internal {
    inline std::function<local_ref<jclass>(const char*)> get_class_loader_find_class_method(const local_ref<jclass>& cls)
    {
        auto e = env();
        auto getClassLoader = e->GetMethodID(make_local(e->GetObjectClass(cls.get())).get(), "getClassLoader", "()Ljava/lang/ClassLoader;");
        auto classLoader = make_global(e->CallObjectMethod(cls.get(), getClassLoader));
        auto findClass = e->GetMethodID(find_class_native("java/lang/ClassLoader").get(), "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");
        return [classLoader = std::move(classLoader), findClass](const char* fqcn) {
            auto o = static_cast<jclass>(env()->CallObjectMethod(classLoader.get(), findClass, env()->NewStringUTF(fqcn)));
            if (env()->ExceptionCheck()) {
                env()->ExceptionClear();
                return find_class_native(fqcn);
            }
            return make_local(o);
        };
    }
    inline std::function<local_ref<jclass>(const char*)>& class_loader_find_class_cache(std::function<local_ref<jclass>(const char*)>&& cache = {})
    {
        static auto instance(std::move(cache));
        return instance;
    }
}

template<typename JType> void replace_with_class_loader_find_class()
{
    internal::class_loader_find_class_cache(internal::get_class_loader_find_class_method(find_class_native(fqcn<JType>())));
}

inline local_ref<jclass> find_class(const char* fqcn)
{
    const auto& find_class_fun = internal::class_loader_find_class_cache(find_class_native);
    return find_class_fun(fqcn);
}
template<typename JType> jclass get_class()
{
    static auto instance = make_global(find_class(fqcn<JType>()));
    return instance.get();
}


#define DEFINE_JCLASS_ALIAS(className, fullyQualifiedClassName) \
struct className##_impl : public std::remove_pointer_t<jobject> \
{\
    static constexpr decltype(auto) fqcn() noexcept {return #fullyQualifiedClassName;}\
};\
using className = className##_impl*


//*************************************************************************************************
// Type Traits
//*************************************************************************************************

namespace internal
{
    // constexpr string
    template <std::size_t N> struct cexprstr
    {
        template <std::size_t... i> constexpr explicit cexprstr(const char (&v)[N], std::index_sequence<i...>) noexcept : value{v[i]...}
        {
        }
        template <std::size_t M, std::size_t... i1, std::size_t... i2>
        constexpr cexprstr(const char (&v1)[M], std::index_sequence<i1...>, const char (&v2)[N-M+1], std::index_sequence<i2...>) noexcept : value{v1[i1]..., v2[i2]...}
        {
        }
        constexpr explicit cexprstr(const char (&v)[N]) noexcept : cexprstr(v, std::make_index_sequence<N>{})
        {
        }
        template <std::size_t M> constexpr cexprstr(const char (&v1)[M], const char (&v2)[N-M+1]) noexcept
            : cexprstr(v1, std::make_index_sequence<M-1>{}, v2, std::make_index_sequence<N-M+1>{})
        {
        }
        template<std::size_t M> constexpr cexprstr<N+M-1> append(const cexprstr<M>& obj) noexcept
        {
            return cexprstr<N+M-1>(value, obj.value);
        }
        template<std::size_t M> constexpr cexprstr<N+M-1> append(const char (&v)[M]) noexcept
        {
            return cexprstr<N+M-1>(value, v);
        }
        template<std::size_t M> constexpr int compare(std::size_t pos1, std::size_t n1, const cexprstr<M>& str, std::size_t pos2, std::size_t n2) const noexcept
        {
            return compare(pos1, n1, str.value, pos2, n2);
        }
        template<std::size_t M> constexpr int compare(std::size_t pos1, std::size_t n1, const char (&str)[M], std::size_t pos2, std::size_t n2) const noexcept
        {
            return (n1 != n2) ? (n1 < n2 ? -1 : 1)
                :  (n1 == 0) ? 0 
                :  (value[pos1] != str[pos2]) ? value[pos1] - str[pos2]
                :  compare(pos1 + 1, n1 - 1, str, pos2 + 1, n2 - 1);
        }

        constexpr const char* c_str() const noexcept { return value; }

        char value[N];
    };
    template<std::size_t N, std::size_t M> constexpr bool operator==(const cexprstr<N>& a, const cexprstr<M>& b) noexcept
    {
        return a.compare(0, N, b, 0, M) == 0;
    }
    template<std::size_t N, std::size_t M> constexpr bool operator!=(const cexprstr<N>& a, const cexprstr<M>& b) noexcept
    {
            return !operator==(a, b);
    }
}
template <size_t N> constexpr internal::cexprstr<N> make_cexprstr(const char (&v)[N])
{
    return internal::cexprstr<N>(v);
}


template <typename... Ts> struct type_traits;

template <> struct type_traits<>
{
    static constexpr decltype(auto) signature() noexcept { return make_cexprstr(""); }
};
template<typename R, typename... Args> struct type_traits<R(Args...)>
{
    static constexpr decltype(auto) signature() noexcept
    {
        return make_cexprstr("(").append(type_traits<Args...>::signature()).append(")").append(type_traits<R>::signature());
    }
};
template <typename T1, typename T2, typename... Ts> struct type_traits<T1, T2, Ts...>
{
    static constexpr decltype(auto) signature() noexcept
    {
        return type_traits<T1>::signature().append(type_traits<T2>::signature()).append(type_traits<Ts...>::signature());
    }
};

#define DEFINE_TYPE_TRAITS(ctype, jtype, sign)\
template<> struct type_traits<ctype>\
{\
    using jvalue_type = jtype;\
    using jarray_type = jtype ## Array;\
    static constexpr ctype c_cast(jtype v) noexcept { return static_cast<ctype>(v); }\
    static constexpr jtype j_cast(ctype v) noexcept { return static_cast<jtype>(v); }\
    static constexpr decltype(auto) signature() noexcept { return make_cexprstr(#sign); }\
}
#define DEFINE_TYPE_TRAITS_A(type, sign) \
DEFINE_TYPE_TRAITS(type, type, sign);\
template<> struct type_traits<type ## Array>\
{\
    using jvalue_type = type ## Array;\
    using jarray_type = array<jvalue_type>;\
    static constexpr jvalue_type c_cast(jvalue_type v) noexcept { return v; }\
    template<typename V> static constexpr const V& j_cast(const V& v) noexcept { return v; }\
    static constexpr decltype(auto) signature() noexcept { return make_cexprstr("[").append(#sign); }\
}
DEFINE_TYPE_TRAITS_A(jboolean, Z);
DEFINE_TYPE_TRAITS_A(jbyte,    B);
DEFINE_TYPE_TRAITS_A(jchar,    C);
DEFINE_TYPE_TRAITS_A(jshort,   S);
DEFINE_TYPE_TRAITS_A(jint,     I);
DEFINE_TYPE_TRAITS_A(jlong,    J);
DEFINE_TYPE_TRAITS_A(jfloat,   F);
DEFINE_TYPE_TRAITS_A(jdouble,  D);
// DEFINE_TYPE_TRAITS_A(jobject,  Ljava/lang/Object;);
DEFINE_TYPE_TRAITS(char,     jbyte, B);
DEFINE_TYPE_TRAITS(char16_t, jchar, C);

template<> struct type_traits<void>
{
    using jvalue_type = void;
    static constexpr decltype(auto) signature() noexcept { return make_cexprstr("V"); }
};
template<> struct type_traits<bool>
{
    using jvalue_type = jboolean;
    using jarray_type = jbooleanArray;
    static constexpr bool c_cast(jboolean v) noexcept { return v == JNI_TRUE; }
    static constexpr jboolean j_cast(bool v) noexcept { return v ? JNI_TRUE : JNI_FALSE; }
    static constexpr decltype(auto) signature() noexcept { return type_traits<jvalue_type>::signature(); }
};
template <typename T> struct type_traits<T*>
{
    using jvalue_type = T*;
    using jarray_type = array<T*>;
    static local_ref<jvalue_type> c_cast(jvalue_type v) noexcept { return make_local(v); }
    template<typename V> static constexpr const V& j_cast(const V& v) noexcept { return v; }
    static constexpr decltype(auto) signature() noexcept { return make_cexprstr("L").append(fqcn<T*>()).append(";"); }
};
template <> struct type_traits<jobjectArray>
{
    using jvalue_type = jobjectArray;
    using jarray_type = array<jobjectArray>;
    static local_ref<jvalue_type> c_cast(jvalue_type v) noexcept { return make_local(v); }
    template<typename V> static constexpr const V& j_cast(const V& v) noexcept { return v; }
    static constexpr decltype(auto) signature() noexcept { return make_cexprstr("[").append(type_traits<jobject>::signature()); }
};
template <typename T> struct type_traits<array<T>>
{
    using jvalue_type = array<T>;
    using jarray_type = array<jvalue_type>;
    static decltype(auto) c_cast(jvalue_type v) noexcept { return make_local(v); }
    template<typename V> static constexpr const V& j_cast(const V& v) noexcept { return v; }
    static constexpr decltype(auto) signature() noexcept { return make_cexprstr("[").append(type_traits<T>::signature()); }
};


//*************************************************************************************************
// Class and Object Operations
//*************************************************************************************************

template <typename JClass> local_ref<jclass> get_super_class(const JClass& clazz) noexcept
{
    return make_local(env()->GetSuperclass(to_native_ref(clazz)));
}
template <typename JType> local_ref<jclass> get_super_class()
{
    return get_super_class(get_class<JType>());
}
template <typename JClass1, typename JClass2> bool is_assignable_from(const JClass1& clazz1, const JClass2& clazz2) noexcept
{
    return env()->IsAssignableFrom(to_native_ref(clazz1), to_native_ref(clazz2)) == JNI_TRUE;
}
template <typename JType1, typename JType2> bool is_assignable_from()
{
    return is_assignable_from(get_class<JType1>(), get_class<JType2>());
}
template <typename JType> local_ref<jclass> get_object_class(const JType& jobj) noexcept
{
    return make_local(env()->GetObjectClass(to_native_ref(jobj)));
}
template <typename JType, typename JClass> bool is_instance_of(const JType& jobj, const JClass& clazz) noexcept
{
    return env()->IsInstanceOf(to_native_ref(jobj), to_native_ref(clazz)) == JNI_TRUE;
}
template <typename JType, typename T> bool is_instance_of(const T& jobj)
{
    return is_instance_of(jobj, get_class<JType>());
}
template <typename JType1, typename JType2> bool is_same_object(const JType1& ref1, const JType2& ref2) noexcept
{
    return env()->IsSameObject(to_native_ref(ref1), to_native_ref(ref2)) == JNI_TRUE;
}

//*************************************************************************************************
// Exceptions
//*************************************************************************************************

inline void exception_check()
{
    if (env()->ExceptionCheck()) throw vm_exception();
}
template <typename JThrowable> void throw_new(const char* what)
{
    env()->ThrowNew(get_class<JThrowable>(), what);
}

template <class F, class... Args> decltype(auto) exception_guard(F&& func, Args&&... args) noexcept
{
    DEFINE_JCLASS_ALIAS(Error, java/lang/Error);
    DEFINE_JCLASS_ALIAS(RuntimeException, java/lang/RuntimeException);
    DEFINE_JCLASS_ALIAS(OutOfMemoryError, java/lang/OutOfMemoryError);
    try {
        return func(std::forward<Args>(args)...);
    } catch (vm_exception&) {
        // java exception has already thrown
    } catch (std::runtime_error& e) {
        throw_new<RuntimeException>(e.what());
    } catch (std::bad_alloc& e) {
        throw_new<OutOfMemoryError>(e.what());
    } catch (std::exception& e) {
        throw_new<Error>(e.what());
    } catch (...) {
        throw_new<Error>("Un unidentified C++ exception was thrown");
    }
    env()->ExceptionDescribe();
    return decltype(func(std::forward<Args>(args)...))();
}


//*************************************************************************************************
// String Operations
//*************************************************************************************************

template <typename> struct string_traits;
template<> struct string_traits<char>
{
    using value_type = char;
    static jstring new_string(JNIEnv* e, const value_type* chars, jsize) noexcept { return e->NewStringUTF(chars); }
    static const jsize length(JNIEnv* e, jstring str) noexcept { return e->GetStringUTFLength(str); }
    static const value_type* get_chars(JNIEnv* e, jstring str, jboolean* isCopy) noexcept { return e->GetStringUTFChars(str, isCopy); }
    static void release_chars(JNIEnv* e, jstring str, const value_type* chars) noexcept { e->ReleaseStringUTFChars(str, chars); }
    static void get_region(JNIEnv* e, jstring str, jsize start, jsize len, value_type* buf) noexcept { e->GetStringUTFRegion(str, start, len, buf); }
};
template<> struct string_traits<jchar>
{
    using value_type = jchar;
    static jstring new_string(JNIEnv* e, const value_type* unicodeChars, jsize length) noexcept { return e->NewString(unicodeChars, length); }
    static const jsize length(JNIEnv* e, jstring str) noexcept { return e->GetStringLength(str); }
    static const value_type* get_chars(JNIEnv* e, jstring str, jboolean* isCopy) noexcept { return e->GetStringChars(str, isCopy); }
    static void release_chars(JNIEnv* e, jstring str, const value_type* chars) noexcept { e->ReleaseStringChars(str, chars); }
    static void get_region(JNIEnv* e, jstring str, jsize start, jsize len, value_type* buf) noexcept { e->GetStringRegion(str, start, len, buf); }
};
template<> struct string_traits<char16_t>
{
    using value_type = char16_t;
    static jstring new_string(JNIEnv* e, const value_type* chars, jsize length) noexcept { return e->NewString(reinterpret_cast<const jchar*>(chars), length); }
    static const jsize length(JNIEnv* e, jstring str) noexcept { return e->GetStringLength(str); }
    static const value_type* get_chars(JNIEnv* e, jstring str, jboolean* isCopy) noexcept { return reinterpret_cast<const value_type*>(e->GetStringChars(str, isCopy)); }
    static void release_chars(JNIEnv* e, jstring str, const value_type* chars) noexcept { e->ReleaseStringChars(str, reinterpret_cast<const jchar*>(chars)); }
    static void get_region(JNIEnv* e, jstring str, jsize start, jsize len, value_type* buf) noexcept { e->GetStringRegion(str, start, len, reinterpret_cast<jchar*>(buf)); }
};

template <typename T, typename Traits = string_traits<T>> decltype(auto) get_chars(jstring str, jboolean* isCopy = nullptr)
{
    auto deleter = [str](const T* p) { Traits::release_chars(env(), str, p); };
    return std::unique_ptr<const T, decltype(deleter)>(Traits::get_chars(env(), str, isCopy), std::move(deleter));
}

template <typename T, typename Traits = string_traits<T>> local_ref<jstring> to_jstring(const T* str, size_t n) noexcept
{
    return make_local(Traits::new_string(env(), str, static_cast<jsize>(n)));
}
template <typename T, typename Traits = string_traits<T>> local_ref<jstring> to_jstring(const std::basic_string<T>& str) noexcept
{
    return to_jstring<T,Traits>(str.c_str(), str.size());
}
template <typename T, size_t N, typename Traits = string_traits<T>> local_ref<jstring> to_jstring(const T (&str)[N]) noexcept
{
    return to_jstring<T,Traits>(str, N-1);
}

namespace internal
{
    template <typename JObj, std::enable_if_t<std::is_same<native_ref<JObj>, jstring>::value, std::nullptr_t> = nullptr> constexpr jstring as_jstring(const JObj& jobj) 
    {
        return to_native_ref(jobj);
    }
    //! return jstring if it is an instance of String. Otherwise it returns null.
    template <typename JObj, std::enable_if_t<!std::is_same<native_ref<JObj>, jstring>::value, std::nullptr_t> = nullptr> jstring as_jstring(const JObj& jobj)
    {
        return is_instance_of<jstring>(jobj) ? static_cast<jstring>(to_native_ref(jobj)) : jstring{};
    }
}

//! Convert to std::basic_string from jstring. If it is null it returns an empty string.
template <typename T, typename JStr, typename Traits = string_traits<T>>
std::basic_string<T> to_basic_string(const JStr& str)
{
    std::basic_string<T> ret;
    auto jstr = internal::as_jstring(str);
    if (jstr) {
        auto e = env();
        ret.resize(Traits::length(e, jstr), 0);
        Traits::get_region(e, jstr, 0, e->GetStringLength(jstr), &ret[0]);
    }
    return  ret;
}
template <typename JStr> std::string to_string(const JStr& str)
{
    return to_basic_string<char>(str);
}
template <typename JStr> std::u16string to_u16string(const JStr& str)
{
    return to_basic_string<char16_t>(str);
}


using string_buffer = std::basic_string<jchar>;

template <typename JStr> void append(string_buffer& buf, const JStr& str)
{
    auto jstr = internal::as_jstring(str);
    if (jstr) {
        const auto len = string_traits<jchar>::length(env(), jstr);
        const auto prevlen = buf.size();
        buf.resize(prevlen + len);
        string_traits<jchar>::get_region(env(), jstr, 0, len, &buf[prevlen]);
    }
}
inline void append(string_buffer& buf, const jchar* str, size_t len)
{
    buf.append(str, len);
}
inline void append(string_buffer& buf, const char16_t* str, size_t len)
{
    buf.append(reinterpret_cast<const jchar*>(str), len);
}
inline void append(string_buffer& buf, const char* str, size_t len)
{
    append(buf, to_jstring(str, len));
}
template <typename T, size_t N> void append(string_buffer& buf, const T (&str)[N])
{
    append(buf, str, N-1);
}
template <typename T> void append(string_buffer& buf, const std::basic_string<T>& str)
{
    append(buf, str.c_str(), str.size());
}

inline void join_buffer(string_buffer& buf) {}
template <typename T, typename... Ts> void join_buffer(string_buffer& buf, T&& str, Ts&&... strings)
{
    append(buf, std::forward<T>(str));
    join_buffer(buf, std::forward<Ts>(strings)...);
}
template <typename... Ts> local_ref<jstring> join(Ts&&... strings)
{
    string_buffer buf;
    join_buffer(buf, std::forward<Ts>(strings)...);
    return to_jstring(buf);
}

// Custom Traits
template<typename T> struct type_traits<std::basic_string<T>>
{
    using jvalue_type = jstring;
    using jarray_type = array<jvalue_type>;
    static std::basic_string<T> c_cast(jstring v) { return to_basic_string<T>(v); }
    static decltype(auto) j_cast(const std::basic_string<T>& v) { return to_jstring(v); }
    static constexpr decltype(auto) signature() noexcept { return type_traits<jvalue_type>::signature(); }
};



//*************************************************************************************************
// Function Traits
//*************************************************************************************************

#define DEFINE_ARRAY_OPERATOR(type, methodType) \
    using array_type = type##Array;\
    static array_type new_array(JNIEnv* e, jsize length) noexcept { return e->New##methodType##Array(length); }\
    static value_type* get_elements(JNIEnv* e, array_type arr, jboolean* isCopy) noexcept { return e->Get##methodType##ArrayElements(arr, isCopy); }\
    static void release_elements(JNIEnv* e, array_type arr, type* elems, jint mode) noexcept { e->Release##methodType##ArrayElements(arr, elems, mode); }\
    static void get_region(JNIEnv* e, array_type arr, jsize start, jsize len, value_type* buf) noexcept { e->Get##methodType##ArrayRegion(arr, start, len, buf); }\
    static void set_region(JNIEnv* e, array_type arr, jsize start, jsize len, const value_type* buf) noexcept { e->Set##methodType##ArrayRegion(arr, start, len, buf); }\

#define DEFINE_FIELD_OPERATOR(type, methodType) \
    static type get_field(JNIEnv* e, jobject obj, jfieldID fieldID) noexcept {return static_cast<type>(e->Get##methodType##Field(obj, fieldID));} \
    template<typename V> static void set_field(JNIEnv* e, jobject obj, jfieldID fieldID, const V& field) noexcept {e->Set##methodType##Field(obj, fieldID, to_native_ref(field));} \
    static type get_static_field(JNIEnv* e, jclass clazz, jfieldID fieldID) noexcept {return static_cast<type>(e->GetStatic##methodType##Field(clazz, fieldID));} \
    template<typename V> static void set_static_field(JNIEnv* e, jclass clazz, jfieldID fieldID, const V& field) noexcept {e->SetStatic##methodType##Field(clazz, fieldID, to_native_ref(field));}

#define DEFINE_METHOD_OPERATOR(type, methodType) \
    template<typename... Args> static type call_method(JNIEnv* e, jobject obj, jmethodID methodID, const Args&... args) noexcept {return static_cast<type>(e->Call##methodType##Method(obj, methodID, to_native_ref(args)...));}\
    template<typename... Args> static type call_non_virtual_method(JNIEnv* e, jobject obj, jclass clazz, jmethodID methodID, const Args&... args) noexcept {return static_cast<type>(e->CallNonvirtual##methodType##Method(obj, clazz, methodID, to_native_ref(args)...));}\
    template<typename... Args> static type call_static_method(JNIEnv* e, jclass clazz, jmethodID methodID, const Args&... args) noexcept {return static_cast<type>(e->CallStatic##methodType##Method(clazz, methodID, to_native_ref(args)...));}


template<typename> struct function_traits;
template<typename T> struct function_traits<T*>
{
    using value_type = T*;
    DEFINE_FIELD_OPERATOR(T*, Object)
    DEFINE_METHOD_OPERATOR(T*, Object)
};
template<> struct function_traits<void>
{
    using value_type = void;
    DEFINE_METHOD_OPERATOR(void, Void)
};
#define DEFINE_PRIMITIVE_FUNC_TRAITS(type, methodType) \
template<> struct function_traits<type>\
{\
    using value_type = type;\
    DEFINE_FIELD_OPERATOR(type, methodType)\
    DEFINE_METHOD_OPERATOR(type, methodType)\
};\
template<> struct function_traits<type##Array>\
{\
    using value_type = type;\
    DEFINE_ARRAY_OPERATOR(type, methodType)\
    DEFINE_FIELD_OPERATOR(type##Array, Object)\
    DEFINE_METHOD_OPERATOR(type##Array, Object)\
}
DEFINE_PRIMITIVE_FUNC_TRAITS(jboolean, Boolean);
DEFINE_PRIMITIVE_FUNC_TRAITS(jbyte,    Byte);
DEFINE_PRIMITIVE_FUNC_TRAITS(jchar,    Char);
DEFINE_PRIMITIVE_FUNC_TRAITS(jshort,   Short);
DEFINE_PRIMITIVE_FUNC_TRAITS(jint,     Int);
DEFINE_PRIMITIVE_FUNC_TRAITS(jlong,    Long);
DEFINE_PRIMITIVE_FUNC_TRAITS(jfloat,   Float);
DEFINE_PRIMITIVE_FUNC_TRAITS(jdouble,  Double);
#undef DEFINE_PRIMITIVE_FUNC_TRAITS
#undef DEFINE_METHOD_OPERATOR
#undef DEFINE_FIELD_OPERATOR
#undef DEFINE_ARRAY_OPERATOR



//*************************************************************************************************
// Primitive Array Operations
//*************************************************************************************************

template <typename T, std::enable_if_t<is_base_ptr_of<jarray, T>::value, std::nullptr_t> = nullptr>
jsize length(T array) noexcept
{
    return env()->GetArrayLength(array);
}
template <typename T, std::enable_if_t<is_base_ptr_of<jarray, typename T::element_type*>::value, std::nullptr_t> = nullptr>
jsize length(const T& array) noexcept
{
    return length(array.get());
}

template <typename T, typename Traits = function_traits<typename type_traits<T>::jarray_type>, std::enable_if_t<is_primitive_type<T>::value, std::nullptr_t> = nullptr>
local_ref<typename type_traits<T>::jarray_type> new_array(jsize length)
{
    return make_local(Traits::new_array(env(), length));
}
template <typename JArray, typename Traits = function_traits<native_ref<JArray>>, std::enable_if_t<is_primitive_array_type<native_ref<JArray>>::value, std::nullptr_t> = nullptr>
void get_region(const JArray& array, jsize start, jsize len, typename Traits::value_type* buf)
{
    Traits::get_region(env(), to_native_ref(array), start, len, buf);
}
template <typename JArray, typename Traits = function_traits<native_ref<JArray>>, std::enable_if_t<is_primitive_array_type<native_ref<JArray>>::value, std::nullptr_t> = nullptr>
void set_region(const JArray& array, jsize start, jsize len, const typename Traits::value_type* buf)
{
    Traits::set_region(env(), to_native_ref(array), start, len, buf);
}

// array elements

template<typename Traits> struct array_elements_deleter
{
    void operator()(typename Traits::value_type* p) const noexcept
    {
        Traits::release_elements(env(), array, p, release_mode);
    }
    void operator()(const typename Traits::value_type* p) const noexcept
    {
        Traits::release_elements(env(), array, const_cast<typename Traits::value_type*>(p), release_mode);
    }
    typename Traits::array_type array;
    jint release_mode;
};
template <typename Traits> using array_elements = std::unique_ptr<typename Traits::value_type, array_elements_deleter<Traits>>;

template <typename JArray, typename Traits = function_traits<native_ref<JArray>>>
array_elements<Traits> get_elements(const JArray& array, jboolean* isCopy = nullptr)
{
    return array_elements<Traits>(Traits::get_elements(env(), to_native_ref(array), isCopy), array_elements_deleter<Traits>{to_native_ref(array), 0});
}
template <typename Traits> void commit(array_elements<Traits>& elems)
{
    Traits::release_elements(env(), elems.get_deleter().array, elems.get(), JNI_COMMIT);
}
template <typename Traits> void set_abort(array_elements<Traits>& elems, bool abortive = true)
{
    elems.get_deleter().release_mode = abortive ? JNI_ABORT : 0;
}
template <typename Traits> typename array_elements<Traits>::pointer begin(array_elements<Traits>& elems)
{
    return elems.get();
}
template <typename Traits> typename array_elements<Traits>::pointer end(array_elements<Traits>& elems)
{
    return elems.get() + length(elems.get_deleter().array);
}
template <typename Traits> const typename array_elements<Traits>::pointer begin(const array_elements<Traits>& elems)
{
    return elems.get();
}
template <typename Traits> const typename array_elements<Traits>::pointer end(const array_elements<Traits>& elems)
{
    return elems.get() + length(elems.get_deleter().array);
}

// const array elements

template <typename Traits> using const_array_elements = std::unique_ptr<const typename Traits::value_type, array_elements_deleter<Traits>>;

template <typename JArray, typename Traits = function_traits<native_ref<JArray>>>
const_array_elements<Traits> get_const_elements(const JArray& array, jboolean* isCopy = nullptr)
{
    return array_elements<Traits>(Traits::get_elements(env(), to_native_ref(array), isCopy), array_elements_deleter<Traits>{to_native_ref(array), JNI_ABORT});
}
template <typename Traits> typename const_array_elements<Traits>::pointer begin(const const_array_elements<Traits>& elems)
{
    return elems.get();
}
template <typename Traits> typename const_array_elements<Traits>::pointer end(const const_array_elements<Traits>& elems)
{
    return elems.get() + length(elems.get_deleter().array);
}

//*************************************************************************************************
// Object Array Operations
//*************************************************************************************************

template<typename T> using object_array_value_type = typename std::remove_pointer_t<native_ref<T>>::value_type;

template <typename T, std::enable_if_t<is_derived_from_jobject<T>::value, std::nullptr_t> = nullptr>
local_ref<array<T>> new_array(jsize length)
{
    return make_local(static_cast<array<T>>(env()->NewObjectArray(length, get_class<T>(), nullptr)));
}
template <typename JObjArray, std::enable_if_t<is_derived_from_jobject<object_array_value_type<JObjArray>>::value, std::nullptr_t> = nullptr> 
decltype(auto) get(const JObjArray& array, jsize index) noexcept
{
    return make_local(static_cast<object_array_value_type<JObjArray>>(env()->GetObjectArrayElement(to_native_ref(array), index)));
}
template <typename JObjArray, typename JType, std::enable_if_t<is_derived_from_jobject<object_array_value_type<JObjArray>>::value, std::nullptr_t> = nullptr> 
void set(JObjArray& array, jsize index, const JType& value) noexcept
{
    env()->SetObjectArrayElement(to_native_ref(array), index, to_native_ref(value));
}
template <typename JObjArray, typename OutItr, typename F, std::enable_if_t<is_derived_from_jobject<object_array_value_type<JObjArray>>::value, std::nullptr_t> = nullptr> 
OutItr get_region(const JObjArray& array, jsize start, jsize len, OutItr itr, F transform)
{
    const auto e = env();
    auto arr = to_native_ref(array);
    for (jsize i = start, ie = start + len; i < ie; ++i) {
        *itr = transform(make_local(static_cast<object_array_value_type<JObjArray>>(e->GetObjectArrayElement(arr, i))));
        ++itr;
    }
    return itr;
}
template <typename JObjArray, typename InItr, typename F, std::enable_if_t<is_derived_from_jobject<object_array_value_type<JObjArray>>::value, std::nullptr_t> = nullptr> 
InItr set_region(JObjArray& array, jsize start, jsize len, InItr itr, F transform)
{
    const auto e = env();
    auto arr = to_native_ref(array);
    for (jsize i = start, ie = start + len; i < ie; ++i) {
        auto lref = make_local(transform(*itr));
        e->SetObjectArrayElement(arr, i, lref.get());
        ++itr;
    }
    return itr;
}



template <typename T, typename JArray, std::enable_if_t<std::is_same<T, bool>::value && std::is_same<native_ref<JArray>, jbooleanArray>::value, std::nullptr_t> = nullptr> 
std::vector<T> to_vector(const JArray& array)
{
    auto elems = get_const_elements(array);
    return std::vector<bool>(jni::begin(elems), jni::end(elems));
}
template <typename T, typename JArray, std::enable_if_t<is_primitive_type<T>::value && is_primitive_array_type<native_ref<JArray>>::value, std::nullptr_t> = nullptr> 
std::vector<T> to_vector(const JArray& array)
{
    const auto len = length(array);
    std::vector<T> ret(len);
    get_region(array, 0, len, ret.data());
    return ret;
}
template <typename JArray, std::enable_if_t<is_primitive_array_type<native_ref<JArray>>::value, std::nullptr_t> = nullptr> 
decltype(auto) to_vector(const JArray& array)
{
    return to_vector<typename function_traits<native_ref<JArray>>::value_type>(array);
}

template <typename T, typename JArray, std::enable_if_t<is_derived_from_jobject<typename type_traits<T>::jvalue_type>::value && is_derived_from_jobjectArray<native_ref<JArray>>::value, std::nullptr_t> = nullptr>
std::vector<T> to_vector(const JArray& array)
{
    std::vector<T> ret;
    auto arr = to_native_ref(array);
    if (arr) {
        const auto e = env();
        const auto len = length(array);
        ret.reserve(len);
        for (jsize i = 0; i < len; ++i) {
            auto lref = make_local(static_cast<typename type_traits<T>::jvalue_type>(e->GetObjectArrayElement(arr, i)));
            ret.emplace_back(type_traits<T>::c_cast(lref.get()));
        }
    }
    return ret;
}


template <typename T, std::enable_if_t<is_primitive_type<T>::value, std::nullptr_t> = nullptr> 
local_ref<typename type_traits<T>::jarray_type> to_jarray(const std::vector<T>& vec)
{
    const auto len = static_cast<jsize>(vec.size());
    auto ret = new_array<T>(len);
    set_region(ret, 0, len, vec.data());
    return ret;
}
template <typename T, std::enable_if_t<std::is_same<T, bool>::value, std::nullptr_t> = nullptr> 
local_ref<jbooleanArray> to_jarray(const std::vector<T>& vec)
{
    auto ret = new_array<jboolean>(static_cast<jsize>(vec.size()));
    auto elems = get_elements(ret);
    std::copy(vec.begin(), vec.end(), jni::begin(elems));
    return ret;
}
template <typename T, std::enable_if_t<is_derived_from_jobject<typename type_traits<T>::jvalue_type>::value, std::nullptr_t> = nullptr> 
local_ref<typename type_traits<T>::jarray_type> to_jarray(const std::vector<T>& vec)
{
    const auto e = env();
    const auto len = static_cast<jsize>(vec.size());
    auto ret = new_array<typename type_traits<T>::jvalue_type>(len);
    for (jsize i = 0; i < len; ++i) {
        e->SetObjectArrayElement(ret.get(), i, to_native_ref(type_traits<T>::j_cast(vec[i])));
    }
    return ret;
}

// Custom Traits
template <typename T> struct type_traits<std::vector<T>>
{
    using jvalue_type = typename type_traits<T>::jarray_type;
    using jarray_type = array<jvalue_type>;
    static std::vector<T> c_cast(jvalue_type v) { return to_vector<T>(v); }
    static decltype(auto) j_cast(const std::vector<T>& v) { return to_jarray(v); }
    static constexpr decltype(auto) signature() noexcept { return type_traits<jvalue_type>::signature(); }
};


//*************************************************************************************************
// Field IDs and Method IDs
//*************************************************************************************************

template<typename T> const char* get_signature() noexcept
{
    static auto instance = type_traits<T>::signature();
    return instance.c_str();
}
template<typename JType, typename T> jfieldID get_field_id(const char* name)
{
    return env()->GetFieldID(get_class<JType>(), name, get_signature<T>());
}
template<typename JType, typename T> jfieldID get_static_field_id(const char* name)
{
    return env()->GetStaticFieldID(get_class<JType>(), name, get_signature<T>());
}
template<typename JType, typename T> jmethodID get_method_id(const char* name)
{
    return env()->GetMethodID(get_class<JType>(), name, get_signature<T>());
}
template<typename JType, typename T> jmethodID get_static_method_id(const char* name)
{
    return env()->GetStaticMethodID(get_class<JType>(), name, get_signature<T>());
}


//*************************************************************************************************
// Accessing Fields
//*************************************************************************************************

template<typename JType, typename T> struct field
{
    template<typename JObj> decltype(auto) get(const JObj& obj) const
    {
        return type_traits<T>::c_cast(function_traits<typename type_traits<T>::jvalue_type>::get_field(env(), jni::to_native_ref(obj), id));
    }
    template<typename JObj, typename U> void set(const JObj& obj, const U& value) const
    {
        function_traits<typename type_traits<T>::jvalue_type>::set_field(env(), jni::to_native_ref(obj), id, type_traits<T>::j_cast(value));
    }
    jfieldID id{};
};
template <typename JType, typename T> field<JType, T> make_field(const char* name)
{
    return field<JType, T>{ get_field_id<JType, T>(name) };
}

//*************************************************************************************************
// Accessing Static Fields
//*************************************************************************************************

template<typename JType, typename T> struct static_field
{
    decltype(auto) get() const
    {
        return type_traits<T>::c_cast(function_traits<typename type_traits<T>::jvalue_type>::get_static_field(env(), get_class<JType>(), id));
    }
    template<typename U> void set(const U& value) const
    {
        function_traits<typename type_traits<T>::jvalue_type>::set_static_field(env(), get_class<JType>(), id, type_traits<T>::j_cast(value));
    }
    jfieldID id{};
};
template <typename JType, typename T> static_field<JType, T> make_static_field(const char* name)
{
    return static_field<JType, T>{ get_static_field_id<JType, T>(name) };
}

//*************************************************************************************************
// Calling Instance Methods
//*************************************************************************************************

template <typename...> struct method;
template <typename JType, typename... Args> struct method<JType, void(Args...)> 
{
    template<typename JObj, typename... Ts> void operator()(const JObj& obj, const Ts&... args) const
    {
        function_traits<void>::call_method(env(), to_native_ref(obj), id, type_traits<Args>::j_cast(args)...);
        exception_check();
    }

    jmethodID id{};
};
template <typename JType, typename R, typename... Args> struct method<JType, R(Args...)> 
{
    template<typename JObj, typename... Ts> decltype(auto) operator()(const JObj& obj, const Ts&... args) const
    {
        auto result = function_traits<typename type_traits<R>::jvalue_type>::call_method(env(), to_native_ref(obj), id, type_traits<Args>::j_cast(args)...);
        exception_check();
        return type_traits<R>::c_cast(result);
    }

    jmethodID id{};
};
template <typename JType, typename Fun> method<JType, Fun> make_method(const char* name)
{
    return method<JType, Fun>{ get_method_id<JType, Fun>(name) };
}


template <typename...> struct non_virtual_method;
template <typename JType, typename... Args> struct non_virtual_method<JType, void(Args...)> 
{
    template<typename JObj, typename... Ts> void operator()(const JObj& obj, const Ts&... args) const
    {
        function_traits<void>::call_non_virtual_method(env(), to_native_ref(obj), get_class<JType>(), id, type_traits<Args>::j_cast(args)...);
        exception_check();
    }

    jmethodID id{};
};
template <typename JType, typename R, typename... Args> struct non_virtual_method<JType, R(Args...)> 
{
    template<typename JObj, typename... Ts> decltype(auto) operator()(const JObj& obj, const Ts&... args) const
    {
        auto result = function_traits<typename type_traits<R>::jvalue_type>::call_non_virtual_method(env(), to_native_ref(obj), get_class<JType>(), id, type_traits<Args>::j_cast(args)...);
        exception_check();
        return type_traits<R>::c_cast(result);
    }

    jmethodID id{};
};
template <typename JType, typename Fun> non_virtual_method<JType, Fun> make_non_virtual_method(const char* name)
{
    return non_virtual_method<JType, Fun>{ get_method_id<JType, Fun>(name) };
}

//*************************************************************************************************
// Calling Static Methods
//*************************************************************************************************

template <typename...> struct static_method;
template <typename JType, typename... Args> struct static_method<JType, void(Args...)> 
{
    template<typename... Ts> void operator()(const Ts&... args) const
    {
        function_traits<void>::call_static_method(env(), get_class<JType>(), id, type_traits<Args>::j_cast(args)...);
        exception_check();
    }

    jmethodID id{};
};
template <typename JType, typename R, typename... Args> struct static_method<JType, R(Args...)>
{
    template<typename... Ts> decltype(auto) operator()(const Ts&... args) const
    {
        auto result = function_traits<typename type_traits<R>::jvalue_type>::call_static_method(env(), get_class<JType>(), id, type_traits<Args>::j_cast(args)...);
        exception_check();
        return type_traits<R>::c_cast(result);
    }

    jmethodID id{};
};
template <typename JType, typename Fun> static_method<JType, Fun> make_static_method(const char* name)
{
    return static_method<JType, Fun>{ get_static_method_id<JType, Fun>(name) };
}


//*************************************************************************************************
// Constructor Methods
//*************************************************************************************************

template <typename...> struct constructor;
template <typename JType, typename... Args> struct constructor<JType(Args...)> 
{
    static jmethodID get_id()
    {
        return get_method_id<JType, void(Args...)>("<init>");
    }
    template<typename... Ts> local_ref<JType> operator()(const Ts&... args) const
    {
        auto result = env()->NewObject(get_class<JType>(), id, to_native_ref(type_traits<Args>::j_cast(args))...);
        exception_check();
        return make_local(static_cast<JType>(result));
    }

    jmethodID id{};
};
template <typename Fun> constructor<Fun> make_constructor()
{
    return constructor<Fun>{ constructor<Fun>::get_id() };
}


//*************************************************************************************************
// Monitor Operations
//*************************************************************************************************

template <typename JType> struct monitor_exit
{
    void operator()(JType monitor) const noexcept
    {
        env()->MonitorExit(monitor);
    }
};
template <typename JType> using monitor = std::unique_ptr<std::remove_pointer_t<JType>, monitor_exit<JType>>;
template <typename JType> monitor<native_ref<JType>> synchronized(const JType& obj)
{
    if (env()->MonitorEnter(to_native_ref(obj)) != 0) throw std::runtime_error("uc::jni::syncronized");
    return monitor<native_ref<JType>>(to_native_ref(obj));
}


//*************************************************************************************************
// Registering Native Methods (Beta)
//*************************************************************************************************
template<typename R, typename... Args> JNINativeMethod make_native_method(const char* name, R(*fnPtr)(JNIEnv*,jobject,Args...)) noexcept
{
    return JNINativeMethod { name, get_signature<R(Args...)>(), (void*)fnPtr };
}

template <typename JType> bool register_natives(const JNINativeMethod* methods, jint nMethods)
{
    return env()->RegisterNatives(get_class<JType>(), methods, nMethods) == 0;
}
template <typename JType, size_t N> bool register_natives(const JNINativeMethod (&methods)[N])
{
    return register_natives<JType>(methods, N);
}


//*************************************************************************************************
// NIO Support (Beta)
//*************************************************************************************************
// DEFINE_JCLASS_ALIAS(DirectByteBuffer, java/nio/DirectByteBuffer);

inline local_ref<jobject> new_direct_byte_buffer(void* address, jlong capacity) noexcept
{
    return make_local(env()->NewDirectByteBuffer(address, capacity));
}


template<typename T> struct direct_buffer_deleter
{
    void operator()(jobject obj) const noexcept
    {
        std::default_delete<T[]>()(static_cast<T*>(env()->GetDirectBufferAddress(obj)));
        env()->DeleteGlobalRef(obj);
    }
};
template <typename T> using direct_buffer = std::unique_ptr<std::remove_pointer_t<jobject>, direct_buffer_deleter<T>>;

template <typename T> direct_buffer<T> new_direct_buffer(size_t size)
{
    std::unique_ptr<T[]> address(new T[size]);
    auto buf = new_direct_byte_buffer(address.get(), size * sizeof(T));
    exception_check();
    if (!buf) throw std::runtime_error("uc::jni::new_direct_buffer");
    address.release();
    return direct_buffer<T>(env()->NewGlobalRef(buf.get()));
}
template <typename T> size_t length(const direct_buffer<T>& obj) noexcept
{
    return static_cast<size_t>(env()->GetDirectBufferCapacity(obj.get()) / sizeof(T));
}
template <typename T> T* address(const direct_buffer<T>& obj) noexcept
{
    return static_cast<T*>(env()->GetDirectBufferAddress(obj.get()));
}

}
}
#endif
