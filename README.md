# uc-jni
uc::jni is a Java Native Interface (JNI) wrapper library created C++14 single-header.



# Install

1. Copy [`uc-jni.hpp`](https://github.com/uctakeoff/uc-jni/blob/master/uc-jni.hpp) to your project.
2. Include it in your c++ source.


# Setup

You must call `uc::jni::java_vm()` only once at the beginning.

```cpp
jint JNI_OnLoad(JavaVM * vm, void * __unused reserved)
{
    uc::jni::java_vm(vm);
    
    return JNI_VERSION_1_6;
}
```

# Example

```cpp
using namespace uc;

extern "C" JNIEXPORT void JNICALL Java_com_example_uc_ucjnitest_UcJniTest_Sample)(JNIEnv *env, jobject thiz)
{
    // If a C++ exception occurs in this, it replaces it with java.lang.RuntimeException.
    jni::exception_guard([&] {

        // Declare Java Class FQCN and define its alias.
        // The "Point" type defined here can be treated like a "jobject".
        DEFINE_JCLASS_ALIAS(Point, android/graphics/Point);

        // Create Constructor Method
        auto makePoint = jni::make_constructor<Point(int,int)>();

        // Create Field Accessor
        auto x = jni::make_field<Point, int>("x");  // Point.x
        auto y = jni::make_field<Point, int>("y");  // Point.y

        // Create Method Accessor
        auto toString = jni::make_method<jobject, std::string()>("toString");  // Object.toString()
        auto equals = jni::make_method<jobject, bool(jobject)>("equals");     // Object.equals()
        auto offset = jni::make_method<Point, void(int,int)>("offset");       // Point.offset()


        // Accessors do not use any extra memory.
        TEST_ASSERT_EQUALS(sizeof(makePoint), sizeof(jmethodID));
        TEST_ASSERT_EQUALS(sizeof(toString), sizeof(jmethodID));
        TEST_ASSERT_EQUALS(sizeof(x), sizeof(jfieldID));


        // decltype(p0) == std::unique_ptr<Point,>. Resources are released automatically.
        auto p0 = makePoint(12, 34);
        auto p1 = makePoint(12, 34);
        auto p2 = makePoint(123, 456);

        LOGD << toString(p0) << ", " << toString(p1) << ", " << toString(p2);

        // Accessing Fields
        TEST_ASSERT_EQUALS(x.get(p0), 12);
        TEST_ASSERT_EQUALS(y.get(p0), 34);

        // Calling Instance Methods
        TEST_ASSERT(equals(p0, p1.get()));
        TEST_ASSERT(!equals(p1, p2.get()));

        x.set(p1, 123);
        y.set(p1, 456);

        TEST_ASSERT(!equals(p0, p1.get()));
        TEST_ASSERT(equals(p1, p2.get()));

        offset(p0, 100, 200);
        TEST_ASSERT_EQUALS(x.get(p0), 112);
        TEST_ASSERT_EQUALS(y.get(p0), 234);
        LOGD << toString(p0) << ", " << toString(p1) << ", " << toString(p2);
    }
}
```

# Detail

## JNIEnv

`JNIEnv* uc::jni::env()` works correctly from any thread call.
[`AttachCurrentThread()`](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/invocation.html#AttachCurrentThread), [`DetachCurrentThread()`](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/invocation.html#DetachCurrentThread) are unnecessary because they are called at an appropriate timing.

```cpp
    std::thread t([thiz]{

        // This is a safe call.
        jclass cls = uc::jni::env()->GetObjectClass(thiz);

        :

        uc::jni::env()->DeleteLocalRef(cls);
    });
```


## References

`uc::jni::local_ref<T>` is an alias for `std::unique_ptr<T,>`.

`uc::jni::global_ref<T>` is an alias for `std::shared_ptr<T,>`.

`uc::jni::weak_ref<T>` can be handled like `std::weak_ptr<T,>`.

```cpp
    // DeleteLocalRef() is called automatically.
    uc::jni::local_ref<jclass> cls = jni::make_local(env->GetObjectClass(thiz));

    // DeleteGlobalRef() is called automatically.
    uc::jni::global_ref<jclass> gcls = jni::make_global(cls);

    // DeleteWeakGlobalRef() is called automatically.
    uc::jni::weak_ref<jclass>  wcls = gcls;

    uc::jni::local_ref<jclass> tmp = wcls.lock();
    if (tmp) {
        :
        :
    }

```

## Resolve Classes


It is not bad.

```cpp
    auto cls = uc::jni::find_class("java/lang/RuntimeException");
```

Recommended.
```cpp
    DEFINE_JCLASS_ALIAS(RuntimeException, java/lang/RuntimeException);

    // jclass instance is cached. Must not release it.
    jclass cls = uc::jni::get_class<RuntimeException>();
```


## Convert Strings

```cpp

    // decltype(jstr) == uc::jni::local_ref<jstring>
    auto jstr = uc::jni::to_jstring("Hello World!");

    std::string str = uc::jni::to_string(jstr);
```

### Support C++11 UTF-16 string

```cpp
    auto jstr = uc::jni::to_jstring(u"こんにちは、世界！"); // japanese

    std::u16string str = uc::jni::to_u16string(jstr);
```


## Method ID, Field ID

You have been freed from tedious ["type signatures"](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/types.html#type_signatures).

```cpp
    DEFINE_JCLASS_ALIAS(Point, android/graphics/Point);

    jfieldID fieldId = uc::jni::get_field_id<Point, int>("x");


    DEFINE_JCLASS_ALIAS(System, java/lang/System);

    jmethodID methodId = uc::jni::get_static_method_id<System, void()>("gc");
```

The following "function object" which is more convenient is recommended.

```cpp
    DEFINE_JCLASS_ALIAS(Point, android/graphics/Point);

    Point point = ...;

    auto x = jni::make_field<Point, int>("x");  // Point.x

    // valueX = point.x;
    int valueX = x.get(point);

    // point.x = 100;
    x.set(point, 100);


    auto gc = uc::jni::make_static_method<System, void()>("gc");

    // System.gc();
    gc();

``
