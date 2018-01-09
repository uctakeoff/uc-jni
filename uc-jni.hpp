/**
uc::jni
Copyright (c) 2018, Kentaro Ushiyama
This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/
#ifndef UC_JNI_HPP
#define UC_JNI_HPP
#define UC_JNI_VERSION "0.1.1"
#define UC_JNI_VERSION_NUM 0x000101

#include <jni.h>
#include <memory>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace uc {
namespace jni {

// uc-jni exception
class exception : public std::exception {};

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

template <typename T, typename U> using is_base_ptr_of = std::is_base_of<std::remove_pointer_t<T>, std::remove_pointer_t<U>>;

template <typename T, std::enable_if_t<is_base_ptr_of<jobject, T>::value, std::nullptr_t> = nullptr>
T get(T obj) noexcept
{
    return obj;
}
template <typename T, std::enable_if_t<is_base_ptr_of<jobject, typename T::element_type*>::value, std::nullptr_t> = nullptr>
typename T::element_type* get(const T& obj) noexcept
{
    return obj.get();
}

template <typename JObj> using native_ref = decltype(get<JObj>({}));


template <typename JObj> struct local_ref_deleter
{
    void operator()(JObj p) const noexcept
    {
        env()->DeleteLocalRef(p);
    }
};
template <typename JObj> using local_ref = std::unique_ptr<std::remove_pointer_t<JObj>, local_ref_deleter<JObj>>;
template <typename JObj> local_ref<JObj> make_local(JObj obj) noexcept
{
    return local_ref<JObj>{obj};
}


template <typename JObj> using global_ref = std::shared_ptr<std::remove_pointer_t<JObj>>;
template <typename JObj> global_ref<native_ref<JObj>> make_global(const JObj& obj)
{
    return global_ref<native_ref<JObj>>(static_cast<native_ref<JObj>>(env()->NewGlobalRef(get(obj))), [](native_ref<JObj> p) { env()->DeleteGlobalRef(p); });
}


//*************************************************************************************************
// Weak Global References
//*************************************************************************************************

template <typename JObj> class weak_ref
{
public:
    weak_ref() = default;

    template <typename T> weak_ref(const T& obj)
        : impl(static_cast<JObj>(env()->NewWeakGlobalRef(get(obj))), [](JObj p) { env()->DeleteWeakGlobalRef(p); })
    {
    }
    local_ref<JObj> lock() const
    {
        return make_local(static_cast<JObj>(env()->NewLocalRef(impl.get())));
    }
    bool expired() const
    {
        return env()->IsSameObject(impl.get(), nullptr) == JNI_TRUE;
    }
    template <typename T> bool is_same(const T& obj) const
    {
        return env()->IsSameObject(impl.get(), get(obj)) == JNI_TRUE;
    }

private:
    std::shared_ptr<std::remove_pointer_t<jweak>> impl;
};


//*************************************************************************************************
// Resolve Classes
//*************************************************************************************************

template <typename T> constexpr decltype(auto) fqcn() noexcept
{
    return std::remove_pointer_t<T>::fqcn();
};
#define DEFINE_FQDN(type, sign) template <> constexpr decltype(auto) fqcn<type>() noexcept { return #sign; }
DEFINE_FQDN(jobject,    java/lang/Object)
DEFINE_FQDN(jstring,    java/lang/String)
DEFINE_FQDN(jclass,     java/lang/Class)
DEFINE_FQDN(jthrowable, java/lang/Throwable)
#undef DEFINE_FQDN

inline local_ref<jclass> find_class(const char* fqcn)
{
    auto o = env()->FindClass(fqcn);
    if (env()->ExceptionCheck()) throw exception();
    return make_local(o);
}
template<typename JObj> jclass get_class()
{
    static auto instance = make_global(find_class(fqcn<JObj>()));
    return instance.get();
}


#define DEFINE_JCLASS_ALIAS(className, fullyQualifiedClassName) \
struct className##_impl : public std::remove_pointer_t<jobject> \
{\
    static constexpr decltype(auto) fqcn() noexcept {return #fullyQualifiedClassName;}\
};\
using className = className##_impl*


//*************************************************************************************************
// Object Operations
//*************************************************************************************************

template <typename JObj> local_ref<jclass> get_object_class(const JObj& obj) noexcept
{
    return make_local(env()->GetObjectClass(get(obj)));
}
template <typename JObj, typename JClass> bool is_instance_of(const JObj& obj, const JClass& clazz) noexcept
{
    return env()->IsInstanceOf(get(obj), get(clazz)) == JNI_TRUE;
}
template <typename JType, typename JObj> bool is_instance_of(const JObj& obj) noexcept
{
    return is_instance_of(obj, get_class<JType>());
}
template <typename JObj1, typename JObj2> bool is_same_object(const JObj1& ref1, const JObj2& ref2) noexcept
{
    return env()->IsSameObject(get(ref1), get(ref2)) == JNI_TRUE;
}

//*************************************************************************************************
// Exceptions
//*************************************************************************************************

inline void exception_check()
{
    if (env()->ExceptionCheck()) throw exception();
}
template <typename JThrowable> void throw_new(const char* what) noexcept
{
    env()->ThrowNew(get_class<JThrowable>(), what);
}

DEFINE_JCLASS_ALIAS(RuntimeException, java/lang/RuntimeException);
template <class F, class... Args> decltype(auto) exception_guard(F&& func, Args&&... args) noexcept
{
    try {
        return func(std::forward<Args>(args)...);
    } catch (jni::exception&) {
        // do nothing
    } catch (std::exception& e) {
        throw_new<RuntimeException>(e.what());
    } catch (...) {
        throw_new<RuntimeException>("Un unidentified C++ exception was thrown");
    }
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

template <typename T, typename Traits = string_traits<T>> local_ref<jstring> to_jstring(const std::basic_string<T>& str)
{
    return make_local(Traits::new_string(env(), str.c_str(), static_cast<jsize>(str.size())));
}
template <typename T, typename Traits = string_traits<T>> local_ref<jstring> to_jstring(const T* str)
{
    return make_local(Traits::new_string(env(), str, static_cast<jsize>(std::char_traits<T>::length(str))));
}

template <typename T, typename JStr, typename Traits = string_traits<T>> std::basic_string<T> to_basic_string(const JStr& str)
{
    return std::basic_string<T>(get_chars<T,Traits>(get(str)).get(), Traits::length(env(), get(str)));
}
template <typename JStr> std::string to_string(const JStr& str)
{
    return to_basic_string<char>(str);
}
template <typename JStr> std::u16string to_u16string(const JStr& str)
{
    return to_basic_string<char16_t>(str);
}



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
    static void set_field(JNIEnv* e, jobject obj, jfieldID fieldID, type field) noexcept {e->Set##methodType##Field(obj, fieldID, field);} \
    static type get_static_field(JNIEnv* e, jclass clazz, jfieldID fieldID) noexcept {return static_cast<type>(e->GetStatic##methodType##Field(clazz, fieldID));} \
    static void set_static_field(JNIEnv* e, jclass clazz, jfieldID fieldID, type field) noexcept {e->SetStatic##methodType##Field(clazz, fieldID, field);}

#define DEFINE_METHOD_OPERATOR(type, methodType) \
    template<typename... Args> static type call_method(JNIEnv* e, jobject obj, jmethodID methodID, Args... args) noexcept {return static_cast<type>(e->Call##methodType##Method(obj, methodID, args...));}\
    template<typename... Args> static type call_non_virtual_method(JNIEnv* e, jobject obj, jclass clazz, jmethodID methodID, Args... args) noexcept {return static_cast<type>(e->CallNonvirtual##methodType##Method(obj, clazz, methodID, args...));}\
    template<typename... Args> static type call_static_method(JNIEnv* e, jclass clazz, jmethodID methodID, Args... args) noexcept {return static_cast<type>(e->CallStatic##methodType##Method(clazz, methodID, args...));}


template<typename> struct function_traits;
template<typename T> struct function_traits<T*>
{
    using value_type = T*;
    using array_type = jobjectArray;
 
    DEFINE_FIELD_OPERATOR(T*, Object)
    DEFINE_METHOD_OPERATOR(T*, Object)
    static array_type new_array(JNIEnv* e, jsize length, jclass elementClass, jobject initialElement) noexcept { return e->NewObjectArray(length, elementClass, initialElement); }
    static value_type get_element(JNIEnv* e, array_type array, jsize index) noexcept { return static_cast<T>(e->GetObjectArrayElement(array, index)); }
    static void set_element(JNIEnv* e, array_type array, jsize index, value_type value) noexcept { return e->SetObjectArrayElement(array, index, value); }
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
    DEFINE_ARRAY_OPERATOR(type, methodType)\
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

template <typename T, typename Traits = function_traits<T>> local_ref<typename Traits::array_type> new_array(jsize length)
{
    return make_local(Traits::new_array(env(), length));
}
template <typename JArray, typename Traits = function_traits<native_ref<JArray>>> 
void get_region(const JArray& array, jsize start, jsize len, typename Traits::value_type* buf)
{
    Traits::get_region(env(), get(array), start, len, buf);
}
template <typename JArray, typename Traits = function_traits<native_ref<JArray>>>
void set_region(const JArray& array, jsize start, jsize len, const typename Traits::value_type* buf)
{
    Traits::set_region(env(), get(array), start, len, buf);
}
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

template<typename Traits> struct array_elements_deleter
{
    void operator()(typename Traits::value_type* p) const noexcept
    {
        Traits::release_elements(env(), array, p, release_mode);
    }
    typename Traits::array_type array;
    jint release_mode;
};
template <typename Traits> using array_elements = std::unique_ptr<typename Traits::value_type, array_elements_deleter<Traits>>;

template <typename JArray, typename Traits = function_traits<native_ref<JArray>>>
array_elements<Traits> get_elements(const JArray& array, bool abortive = false, jboolean* isCopy = nullptr)
{
    return array_elements<Traits>(Traits::get_elements(env(), get(array), isCopy), array_elements_deleter<Traits>{get(array), abortive ? JNI_ABORT : 0});
}

//! copy back the content  cf. Release<PrimitiveType>ArrayElements(...,JNI_COMMIT)
template <typename Traits> void commit(array_elements<Traits>& elems)
{
    Traits::release_elements(env(), elems.get_deleter().array, elems.get(), JNI_COMMIT);
}
template <typename Traits> void set_abort(array_elements<Traits>& elems, bool abortive = true)
{
    elems.get_deleter().release_mode = abortive ? JNI_ABORT : 0;
}

template <typename Traits> typename Traits::value_type* begin(array_elements<Traits>& elems)
{
    return elems.get();
}
template <typename Traits> typename Traits::value_type* end(array_elements<Traits>& elems)
{
    return elems.get() + length(elems.get_deleter().array);
}

template <typename Traits> const typename Traits::value_type* begin(const array_elements<Traits>& elems)
{
    return elems.get();
}
template <typename Traits> const typename Traits::value_type* end(const array_elements<Traits>& elems)
{
    return elems.get() + length(elems.get_deleter().array);
}


template <typename T, std::enable_if_t<is_base_ptr_of<jobject, T>::value, std::nullptr_t> = nullptr>
class array
{
public:
    array() = default;
    array(jsize size) : impl(make_global(make_local(env()->NewObjectArray(size, get_class<T>(), nullptr))))
    {
    }
    template<typename JObj> array(const JObj& array) : impl(make_global(array))
    {
    }
    jobjectArray get() const
    {
        return impl.get();
    }
    jsize size() const
    {
        return length(impl);
    }
    local_ref<T> get(jsize i) const
    {
        return make_local(static_cast<T>(env()->GetObjectArrayElement(impl.get(), i)));
    }
    template<typename JObj> void set(jsize i, const JObj& value)
    {
        env()->SetObjectArrayElement(impl.get(), i, jni::get(value));
    }
private:
    global_ref<jobjectArray> impl;
};



//*************************************************************************************************
// Type Traits
//*************************************************************************************************

namespace internal
{
    template <std::size_t... i> struct indices {};
    template <std::size_t M, std::size_t... i> struct make_indices : public make_indices<M - 1, i..., sizeof...(i)> {};
    template <std::size_t... i> struct make_indices<0, i...> { using type = indices<i...>; };

    template <std::size_t N> struct cexprstr
    {
        template <std::size_t... i> constexpr cexprstr(const char (&v)[N], indices<i...>) : value{v[i]...} {}

        template <std::size_t M, std::size_t... i1, std::size_t... i2>
        constexpr cexprstr(const char (&v1)[M], indices<i1...>, const char (&v2)[N-M+1], indices<i2...>) : value{v1[i1]..., v2[i2]...} {}
        constexpr cexprstr(const char (&v)[N]) : cexprstr(v, typename make_indices<N>::type{})
        {
        }
        template <std::size_t M> constexpr cexprstr(const char (&v1)[M], const char (&v2)[N-M+1])
            : cexprstr(v1, typename make_indices<M-1>::type{}, v2, typename make_indices<N-M+1>::type{})
        {
        }
        template<std::size_t M> constexpr cexprstr<N+M-1> append(const cexprstr<M>& obj)
        {
            return cexprstr<N+M-1>(value, obj.value);
        }
        template<std::size_t M> constexpr cexprstr<N+M-1> append(const char (&v)[M])
        {
            return cexprstr<N+M-1>(value, v);
        }
        constexpr const char* c_str() const { return value; }

        char value[N];
    };
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
template<> struct type_traits<void>
{
    using jvalue_type = void;
    static constexpr decltype(auto) signature() noexcept { return make_cexprstr("V"); }
};
template <typename T> struct type_traits<T*>
{
    using jvalue_type = T*;
    using jarray_type = jobjectArray;
    static constexpr local_ref<jvalue_type> c_cast(jvalue_type v) noexcept { return make_local(v); }
    static constexpr jvalue_type j_cast(jvalue_type v) noexcept { return v; }
    static constexpr decltype(auto) signature() noexcept { return make_cexprstr("L").append(fqcn<T*>()).append(";"); }
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
    static ctype c_cast(jtype v) noexcept { return static_cast<ctype>(v); }\
    static jtype j_cast(ctype v) noexcept { return static_cast<jtype>(v); }\
    static constexpr decltype(auto) signature() noexcept { return make_cexprstr(#sign); }\
}
#define DEFINE_TYPE_TRAITS_A(type, sign) \
DEFINE_TYPE_TRAITS(type, type, sign);\
template<> struct type_traits<type##Array>\
{\
    using jvalue_type = type##Array;\
    using jarray_type = jobjectArray;\
    static jvalue_type c_cast(jvalue_type v) noexcept { return v; }\
    static jvalue_type j_cast(jvalue_type v) noexcept { return v; }\
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
DEFINE_TYPE_TRAITS_A(jobject,  Ljava/lang/Object;);
DEFINE_TYPE_TRAITS(char,     jbyte, B);
DEFINE_TYPE_TRAITS(char16_t, jchar, C);

template<> struct type_traits<bool>
{
    using jvalue_type = jboolean;
    using jarray_type = jbooleanArray;
    static constexpr bool c_cast(jboolean v) noexcept { return v == JNI_TRUE; }
    static constexpr jboolean j_cast(bool v) noexcept { return v ? JNI_TRUE : JNI_FALSE; }
    static constexpr decltype(auto) signature() noexcept { return type_traits<jboolean>::signature(); }
};
template<typename T> struct type_traits<std::basic_string<T>>
{
    using jvalue_type = jstring;
    using jarray_type = jobjectArray;
    static std::basic_string<T> c_cast(jstring v) { return to_basic_string<T>(v); }
    static jstring j_cast(const std::basic_string<T>& v) { return to_jstring(v).release(); }
    static constexpr decltype(auto) signature() noexcept { return type_traits<jstring>::signature(); }
};
template<> struct type_traits<const char*>
{
    using jvalue_type = jstring;
    using jarray_type = jobjectArray;
    static jstring j_cast(const char* v) { return to_jstring(v).release(); }
    static constexpr decltype(auto) signature() noexcept { return type_traits<jstring>::signature(); }
};
template<> struct type_traits<const char16_t*>
{
    using jvalue_type = jstring;
    using jarray_type = jobjectArray;
    static jstring j_cast(const char16_t* v) { return to_jstring(v).release(); }
    static constexpr decltype(auto) signature() noexcept { return type_traits<jstring>::signature(); }
};
template <typename T> struct type_traits<array<T>>
{
    using jvalue_type = jobjectArray;
    using jarray_type = jobjectArray;
    static decltype(auto) c_cast(jobjectArray v) { return array<T>(v); }
    static jobjectArray j_cast(const array<T>& v) { return v.get(); }
    static constexpr decltype(auto) signature() noexcept { return make_cexprstr("[").append(type_traits<T>::signature()); }
};

template <> struct type_traits<std::vector<bool>>
{
    using jvalue_type = jbooleanArray;
    using jarray_type = jobjectArray;
    static constexpr decltype(auto) signature() noexcept
    {
        return make_cexprstr("[").append(type_traits<bool>::signature()); 
    }
    static std::vector<bool> c_cast(jvalue_type v)
    {
        auto elems = jni::get_elements(v);
        return std::vector<bool>(jni::begin(elems), jni::end(elems));
    }
    static jvalue_type j_cast(const std::vector<bool>& v)
    {
        auto ret = jni::new_array<jboolean>(static_cast<jsize>(v.size()));
        auto elems = jni::get_elements(ret, true);
        std::copy(v.begin(), v.end(), jni::begin(elems));
        return ret.release();
    }
};
template <typename T> struct type_traits<std::vector<T>>
{
    using jvalue_type = typename type_traits<T>::jarray_type;
    using jarray_type = jobjectArray;
    static constexpr decltype(auto) signature() noexcept
    {
        return make_cexprstr("[").append(type_traits<T>::signature());
    }
    static std::vector<T> c_cast(jvalue_type v)
    {
        const auto len = length(v);
        std::vector<T> ret(len);
        jni::get_region(v, 0, len, ret.data());
        return ret;
    }
    static jvalue_type j_cast(const std::vector<T>& v)
    {
        const auto len = static_cast<jsize>(v.size());
        auto ret = jni::new_array<T>(len);
        jni::set_region(ret, 0, len, v.data());
        return ret.release();
    }
};
template <typename T> struct type_traits<std::vector<T*>>
{
    using jvalue_type = jobjectArray;
    using jarray_type = jobjectArray;
    static constexpr decltype(auto) signature() noexcept 
    {
        return make_cexprstr("[").append(type_traits<T*>::signature());
    }
    static decltype(auto) c_cast(jobjectArray v)
    {
        const auto e = env();
        const auto len = length(v);
        std::vector<T*> ret(len);
        for (jsize i = 0; i < len; ++i) {
            ret[i] = static_cast<T*>(e->GetObjectArrayElement(v, i));
        }
        return ret;
    }
    static jobjectArray j_cast(const std::vector<T*>& v)
    {
        const auto e = env();
        const auto len = static_cast<jsize>(v.size());
        auto ret = jni::make_local(e->NewObjectArray(len, get_class<T*>(), nullptr));
        for (jsize i = 0; i < len; ++i) {
            e->SetObjectArrayElement(ret.get(), i, v[i]);
        }
        return ret.release();
    }
};


template<typename JType, typename T> jfieldID get_field_id(const char* name) noexcept
{
    return env()->GetFieldID(get_class<JType>(), name, type_traits<T>::signature().c_str());
}
template<typename JType, typename T> jfieldID get_static_field_id(const char* name) noexcept
{
    return env()->GetStaticFieldID(get_class<JType>(), name, type_traits<T>::signature().c_str());
}
template<typename JType, typename T> jmethodID get_method_id(const char* name) noexcept
{
    return env()->GetMethodID(get_class<JType>(), name, type_traits<T>::signature().c_str());
}
template<typename JType, typename T> jmethodID get_static_method_id(const char* name) noexcept
{
    return env()->GetStaticMethodID(get_class<JType>(), name, type_traits<T>::signature().c_str());
}


//*************************************************************************************************
// Accessing Fields
//*************************************************************************************************

template<typename JType, typename T> struct field
{
    template<typename JObj> decltype(auto) get(const JObj& obj) const
    {
        return type_traits<T>::c_cast(function_traits<typename type_traits<T>::jvalue_type>::get_field(env(), jni::get(obj), id));
    }
    template<typename JObj> void set(const JObj& obj, const T& value)
    {
        function_traits<typename type_traits<T>::jvalue_type>::set_field(env(), jni::get(obj), id, type_traits<T>::j_cast(value));
    }
    jfieldID id{};
};
template <typename JType, typename T> field<JType, T> make_field(const char* name) noexcept
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
    void set(const T& value)
    {
        function_traits<typename type_traits<T>::jvalue_type>::set_static_field(env(), get_class<JType>(), id, type_traits<T>::j_cast(value));
    }
    jfieldID id{};
};
template <typename JType, typename T> static_field<JType, T> make_static_field(const char* name) noexcept
{
    return static_field<JType, T>{ get_static_field_id<JType, T>(name) };
}

//*************************************************************************************************
// Calling Instance Methods
//*************************************************************************************************

template <typename...> struct method;
template <typename JType, typename... Args> struct method<JType, void(Args...)> 
{
    template<typename JObj> void operator()(const JObj& obj, const Args&... args) noexcept
    {
        function_traits<void>::call_method(env(), get(obj), id, type_traits<Args>::j_cast(args)...);
        exception_check();
    }

    jmethodID id{};
};
template <typename JType, typename R, typename... Args> struct method<JType, R(Args...)> 
{
    template<typename JObj> decltype(auto) operator()(const JObj& obj, const Args&... args) noexcept
    {
        auto result = function_traits<typename type_traits<R>::jvalue_type>::call_method(env(), get(obj), id, type_traits<Args>::j_cast(args)...);
        exception_check();
        return type_traits<R>::c_cast(result);
    }

    jmethodID id{};
};
template <typename JType, typename Fun> method<JType, Fun> make_method(const char* name) noexcept
{
    return method<JType, Fun>{ get_method_id<JType, Fun>(name) };
}


template <typename...> struct non_virtual_method;
template <typename JType, typename... Args> struct non_virtual_method<JType, void(Args...)> 
{
    template<typename JObj> void operator()(const JObj& obj, const Args&... args) noexcept
    {
        function_traits<void>::call_non_virtual_method(env(), get(obj), get_class<JType>(), id, type_traits<Args>::j_cast(args)...);
        exception_check();
    }

    jmethodID id{};
};
template <typename JType, typename R, typename... Args> struct non_virtual_method<JType, R(Args...)> 
{
    template<typename JObj> decltype(auto) operator()(const JObj& obj, const Args&... args) noexcept
    {
        auto result = function_traits<typename type_traits<R>::jvalue_type>::call_non_virtual_method(env(), get(obj), get_class<JType>(), id, type_traits<Args>::j_cast(args)...);
        exception_check();
        return type_traits<R>::c_cast(result);
    }

    jmethodID id{};
};
template <typename JType, typename Fun> non_virtual_method<JType, Fun> make_non_virtual_method(const char* name) noexcept
{
    return non_virtual_method<JType, Fun>{ get_method_id<JType, Fun>(name) };
}

//*************************************************************************************************
// Calling Static Methods
//*************************************************************************************************

template <typename...> struct static_method;
template <typename JType, typename... Args> struct static_method<JType, void(Args...)> 
{
    void operator()(const Args&... args) noexcept
    {
        function_traits<void>::call_static_method(env(), get_class<JType>(), id, type_traits<Args>::j_cast(args)...);
        exception_check();
    }

    jmethodID id{};
};
template <typename JType, typename R, typename... Args> struct static_method<JType, R(Args...)>
{
    decltype(auto) operator()(const Args&... args) noexcept
    {
        auto result = function_traits<typename type_traits<R>::jvalue_type>::call_static_method(env(), get_class<JType>(), id, type_traits<Args>::j_cast(args)...);
        exception_check();
        return type_traits<R>::c_cast(result);
    }

    jmethodID id{};
};
template <typename JType, typename Fun> static_method<JType, Fun> make_static_method(const char* name) noexcept
{
    return static_method<JType, Fun>{ get_static_method_id<JType, Fun>(name) };
}


//*************************************************************************************************
// Constructor Methods
//*************************************************************************************************

template <typename...> struct constructor;
template <typename JType, typename... Args> struct constructor<JType(Args...)> 
{
    static jmethodID get_id() noexcept
    {
        return get_method_id<JType, void(Args...)>("<init>");
    }
    local_ref<JType> operator()(const Args&... args) noexcept
    {
        auto result = env()->NewObject(get_class<JType>(), id, type_traits<Args>::j_cast(args)...);
        exception_check();
        return make_local(static_cast<JType>(result));
    }

    jmethodID id{};
};
template <typename Fun> constructor<Fun> make_constructor() noexcept
{
    return constructor<Fun>{ constructor<Fun>::get_id() };
}


}
}
#endif
