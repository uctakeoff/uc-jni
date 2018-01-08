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