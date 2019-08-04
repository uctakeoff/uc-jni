# uc-jni
 [日本語ドキュメント(Japanese)](https://github.com/uctakeoff/uc-jni/blob/master/READMEjp.md)

*uc-jni* is a Java Native Interface (JNI) wrapper library created C++14 single-header.


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

**If you call `uc::jni::find_class()`/`uc::jni::get_class()` in native thread ( pthread_create(), std::thread, std::async...), also call `uc::jni::replace_with_class_loader_find_class()`**.

```cpp
UC_JNI_DEFINE_JCLASS_ALIAS(YourOwnClass, com/example/YourOwnClass);

jint JNI_OnLoad(JavaVM * vm, void * __unused reserved)
{
    uc::jni::java_vm(vm);

    // call before find_class() / get_class()
    uc::jni::replace_with_class_loader_find_class<YourOwnClass>();

    return JNI_VERSION_1_6;
}
```

see also [FindClass in native thread](#findclass-in-native-thread).


# Example


## Wrapper Class Builder Macros

You can use helper macros from Ver.1.4.

JNI resources are properly cached and overhead is minimized.

```java
package com.example.uc.ucjnitest;
import java.util.Objects;

public class Point {
    public static final Point ZERO = new Point(0, 0);

    public static Point add(Point a, Point b)
    {
        return new Point(a.x + b.x, a.y + b.y);
    }

    private double x, y;

    Point(double x, double y)
    {
        this.x = x;
        this.y = y;
    }
    Point(Point p)
    {
        this.x = p.x;
        this.y = p.y;
    }
    double norm()
    {
        return Math.sqrt(x * x + y * y);
    }
    void multiply(double v)
    {
        x *= v;
        y *= v;
    }

    @Override public String toString() {...} 
    @Override public int hashCode() {...}
    @Override public boolean equals(Object o) {...}
}
```
For the above Java code, in C ++, write as follows.

```cpp
// Define "jPoint_" class  as a wrapper for Java "com/example/uc/ucjnitest/Point" class.
// "jPoint" is defined as an alias for jPoint_*.
UC_JNI_DEFINE_JCLASS(jPoint, com/example/uc/ucjnitest/Point)
{
    UC_JNI_DEFINE_JCLASS_STATIC_FINAL_FIELD(jPoint, ZERO)
    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(jPoint, add, jPoint, jPoint)
    UC_JNI_DEFINE_JCLASS_CONSTRUCTOR(double, double)
    UC_JNI_DEFINE_JCLASS_CONSTRUCTOR(jPoint)
    UC_JNI_DEFINE_JCLASS_FIELD(double, x)
    UC_JNI_DEFINE_JCLASS_FIELD(double, y)
    UC_JNI_DEFINE_JCLASS_METHOD(double, norm)
    UC_JNI_DEFINE_JCLASS_METHOD(void, multiply, double)
};
```

It is not necessary to write everything.
As a result, the C ++ wrapper class `jPoint` of the` com.example.uc.ucjnitest.Point` class was defined.
This can be used as follows.

```cpp
    // If a C++ exception occurs in this, it replaces it with java.lang.Throwable.
    uc::jni::exception_guard([&] {

        // Constructor Method
        // p0,p1,p2 are smart pointer. release automatically.
        auto p0 = jPoint_::new_(3, 4);       // p0 = new Point(12, 34);
        auto p1 = jPoint_::new_(p0);         // p1 = new Point(p0);

        // get field
        TEST_ASSERT_EQUALS(p1->x(), 3);       // p1.x == 3
        TEST_ASSERT_EQUALS(p1->y(), 4);       // p1.y == 4

        // call method
        TEST_ASSERT_EQUALS(p1->norm(), 5);    // p1.norm() == 5

        // java.lang.Object methods are already defined.
        TEST_ASSERT(p0->equals(p1));          // p0.equals(p1)

        // set field
        p1->x(30);                           // p1.x = 30;
        p1->y(40);                           // p1.y = 40;
        TEST_ASSERT(!p0->equals(p1));         // !p0.equals(p1)

        // get static final field
        TEST_ASSERT_EQUALS(jPoint_::ZERO()->x(), 0);
        TEST_ASSERT_EQUALS(jPoint_::ZERO()->y(), 0);

        // call static method
        auto p2 = jPoint_::add(p0, p1);
        TEST_ASSERT_EQUALS(p2->x(), 60);
        TEST_ASSERT_EQUALS(p2->y(), 80);

        // Object.toString() returns std::string.
        LOGD << p0->toString() << ", " << p1->toString() << ", " << p2->toString();
    });
```

# Detail

In addition to the method / field API as in the above sample, various JNI functions such as array operations and string operations are wrapped and provided.

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

### Local References

`uc::jni::ref<T>` is an alias for `std::unique_ptr<T,>`.

`DeleteLocalRef()` is called automatically.
*uc-jni* functions normally returns a "JNI object" in this form.


```cpp
    // clazz, superClazz will be released automatically. 
    auto clazz = uc::jni::get_object_class(thiz);
    auto superClazz = uc::jni::get_super_class(clazz);
```

Also, using the helper function `uc::jni::make_local()` explicitly calls `NewLocalRef()` to create a local reference.


### Global References

`global_ref<T>` is similar to `std::shared_ptr<T>`.

A global reference is created by `NewGlobalRef()` at the time of assignment, and `DeleteGlobalRef()` is called when the reference count reaches 0.

`uc::jni::make_global()` is provided as a helper function.


```cpp
    // global_ref<jclass>
    auto clazz = uc::jni::make_global(uc::jni::get_object_class(thiz));

    // It is also possible to substitute ref. 
    // Ownership does not shift and a new Global Reference can be created.
    uc::jni::global_ref<jclass> superClazz = uc::jni::get_super_class(clazz);
    superClazz =  env->GetSuperclass(clazz.get());
```

### Weak References

`uc::jni::weak_ref<T>` can be handled like `std::weak_ptr<T>`.

`DeleteWeakGlobalRef()` is called automatically.

```cpp
    uc::jni::weak_ref<jobject> wref = jobj;

    uc::jni::ref<jobject> tmp = wref.lock();
    if (tmp) {
        :
        :
    }

```

## Resolve Classes

There is also a wrapper function of `FindClass()`, but you should not normally use this if you use *uc-jni*.

```cpp
    // I do not recommend
    auto cls = uc::jni::find_class("java/lang/RuntimeException");
```

`uc::jni::get_class()` allocates the cache very efficiently.

Provide very fast access with minimal cost.

```cpp
    // You need to define wrapper classes in macros in advance.
    UC_JNI_DEFINE_JCLASS_ALIAS(RuntimeException, java/lang/RuntimeException);

    // jclass instance is cached. Must not release it.
    jclass cls = uc::jni::get_class<RuntimeException>();
```

## FindClass in native thread

 [FAQ: Why didn't FindClass find my class?](https://developer.android.com/training/articles/perf-jni.html#faq_FindClass)

> You can get into trouble if you create a thread yourself (perhaps by calling pthread_create and then attaching it with AttachCurrentThread). Now there are no stack frames from your application. If you call FindClass from this thread, the JavaVM will start in the "system" class loader instead of the one associated with your application, so attempts to find app-specific classes will fail.

```cpp
    // java.lang.ClassNotFoundException is thrown.
    std::async(std::launch::async, [] {
        auto cls = uc::jni::find_class("com/example/YourOwnClass");
    });
```

> There are a few ways to work around this:

- Do your FindClass lookups once, in JNI_OnLoad, and cache the class references for later use. 
    ```cpp
    UC_JNI_DEFINE_JCLASS_ALIAS(YourOwnClass1, com/example/YourOwnClass1);
    UC_JNI_DEFINE_JCLASS_ALIAS(YourOwnClass2, com/example/YourOwnClass2);
    UC_JNI_DEFINE_JCLASS_ALIAS(YourOwnClass3, com/example/YourOwnClass3);
        :

    jint JNI_OnLoad(JavaVM * vm, void * __unused reserved)
    {
        uc::jni::java_vm(vm);

        // create class caches
        uc::jni::get_class<YourOwnClass1>();
        uc::jni::get_class<YourOwnClass2>();
        uc::jni::get_class<YourOwnClass3>();
            :

        return JNI_VERSION_1_6;
    }
    ```

    Class caches created by these calls are only valid with `uc::jni::get_class()`.

- Cache a reference to the ClassLoader object somewhere handy, and issue loadClass calls directly. 

    ```cpp
    UC_JNI_DEFINE_JCLASS_ALIAS(YourOwnClass1, com/example/YourOwnClass1);
    UC_JNI_DEFINE_JCLASS_ALIAS(YourOwnClass2, com/example/YourOwnClass2);
    UC_JNI_DEFINE_JCLASS_ALIAS(YourOwnClass3, com/example/YourOwnClass3);

    jint JNI_OnLoad(JavaVM * vm, void * __unused reserved)
    {
        uc::jni::java_vm(vm);

        // create ClassLoader cache
        uc::jni::replace_with_class_loader_find_class<YourOwnClass1>();

        return JNI_VERSION_1_6;
    }
    ```

    `uc::jni::replace_with_class_loader_find_class()` replaces the implementation of `uc::jni::find_class()` API with `java.lang.ClassLoader#findClass()`.
    This is implemented with reference to the code written [here](https://stackoverflow.com/questions/13263340/findclass-from-any-thread-in-android-jni).

## String Operations

`jstring` and `std::basic_string` can convert to each other.

```cpp

    // decltype(jstr) == uc::jni::ref<jstring>
    auto jstr = uc::jni::to_jstring("Hello World!");

    std::string str = uc::jni::to_string(jstr);
```

Supports C++11 UTF-16 string.

```cpp
    auto jstr = uc::jni::to_jstring(u"こんにちは、世界！"); // japanese

    std::u16string str = uc::jni::to_u16string(jstr);
```

`uc::jni::join` is more convenient to create `jstring`.

```cpp
jstring returnString(JNIEnv* env, jobject obj, jstring str)
{
    // Arguments can include jstring, std::string, std::u16string, and so on.

    return uc::jni::join("[", str, "] received.").release();
}
```



## Method ID, Field ID

You have been freed from tedious ["type signatures"](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/types.html#type_signatures).

```cpp
    UC_JNI_DEFINE_JCLASS_ALIAS(Point, android/graphics/Point);

    jfieldID fieldId = uc::jni::get_field_id<Point, int>("x");


    UC_JNI_DEFINE_JCLASS_ALIAS(System, java/lang/System);

    jmethodID methodId = uc::jni::get_static_method_id<System, void()>("gc");
```

The following "function object" which is more convenient is recommended.

```cpp
UC_JNI_DEFINE_JCLASS_ALIAS(System, java/lang/System);
UC_JNI_DEFINE_JCLASS_ALIAS(Point, android/graphics/Point);

void func(Point point)
{
    // Point#Point(int, int)
    auto newPoint = uc::jni::make_constructor<Point(int,int)>();

    auto point = newPoint(10, 20);

    // Point#x
    auto x = uc::jni::make_field<Point, int>("x");

    // valueX = point.x;
    int valueX = x.get(point);

    // point.x = 100;
    x.set(point, 100);


    auto gc = uc::jni::make_static_method<System, void()>("gc");

    // System.gc();
    gc();
}
```

The above `x` and `gc` can be treated like `jfieldID` , `jmethodID`. (copy, hold etc)


Other examples.

```cpp
    // boolean Object.equals(Object)
    auto equals = jni::make_method<jobject, bool(jobject)>("equals");

    // String Object.toString()
    auto toString = jni::make_method<jobject, jstring()>("toString"); 
    auto toString2 = jni::make_method<jobject, std::string()>("toString"); // Convert to std::string and return it.


    UC_JNI_DEFINE_JCLASS_ALIAS(UcJniTest, com/example/uc/ucjnitest/UcJniTest);

    // String[] UcJniTest.getFieldStringArray()
    auto getFieldStringArray = uc::jni::make_method<UcJniTest, uc::jni::array<jstring>()>("getFieldStringArray");
    auto getFieldStringArray2 = uc::jni::make_method<UcJniTest, std::vector<std::string>()>("getFieldStringArray"); // Convert to std::vector<std::string>.

```

## Array Operations

### Simple to Use

`jarray` and `std::vector` can convert to each other.

```cpp
    // jintArray to std::vector<jint>.
    auto intValues = uc::jni::to_vector(iArray);

    // std::vector<jint> to ref<jintArray>.
    auto jintValues = uc::jni::to_jarray(intValues);


    // uc::jni::array<jstring> (inherited from jobjectArray) to std::vector<std::string>.
    auto stringValues = uc::jni::to_vector<std::string>(sArray);

    // std::vector<std::string> to ref<uc::jni::array<jstring>>.
    auto jstringValues = uc::jni::to_jarray(stringValues);

```

### Primitive Array

```cpp
    // create ref<jintArray>.
    auto array = uc::jni::new_array<jint>(10);

    TEST_ASSERT_EQUALS(10, uc::jni::length(array));

    // set values.
    const int v0[] {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

    uc::jni::set_region(array, 0, 10, v0);

    // get values.
    int v1[10]{};
    uc::jni::get_region(array, 0, 10, v1);

```

### `GetArrayElements()`/ `SetArrayElements()`

```cpp
    // ReleaseArrayElements( mode = 0 ) is called automatically.
    auto elems = uc::jni::get_elements(array);

    // iterator access OK
    std::copy(std::begin(values), std::end(values), uc::jni::begin(elems));

    // Apply `JNI_COMMIT`
    uc::jni::commit(elems);


    // range based for
    for (auto&& e : elems) {
        e = 100;
    }

    // Apply `JNI_ABORT`
    uc::jni::set_abort(elems);

```

The following are read only access.

```cpp
    // ReleaseArrayElements( mode = JNI_ABORT ) is called automatically.
    auto elems = uc::jni::get_const_elements(array);

    std::equal(std::begin(values), std::end(values), uc::jni::begin(elems));
```

### Object Array

```cpp
    // It means String [].
    auto array = uc::jni::new_array<jstring>(2);

    TEST_ASSERT_EQUALS(2, uc::jni::length(array));


    // Can be used like unique_ptr<jobjectArray>.
    TEST_ASSERT_EQUALS(2, env->GetArrayLength(array.get()));


    // set values.
    uc::jni::set(array, 0, uc::jni::to_jstring("Hello"));
    uc::jni::set(array, 1, uc::jni::to_jstring("World"));

    // get values.
    TEST_ASSERT_EQUALS("Hello", uc::jni::to_string(uc::jni::get(array, 0)));
    TEST_ASSERT_EQUALS("World", uc::jni::to_string(uc::jni::get(array, 1)));
```


Use `uc::jni::array<T>` instead of `jobjectArray`.

```cpp

    // String[] UcJniTest.getFieldStringArray()
    auto get = uc::jni::make_method<UcJniTest, uc::jni::array<jstring>()>("getFieldStringArray");

    // decltype(array) == ref<array<jstring>>
    auto array = get(thiz);
```


## Monitor Operations

```c++

void func(JNIEnv *env, jobject thiz, jobject monitor)
{
    auto s = uc::jni::synchronized(monitor);
            :
            :
}


```
