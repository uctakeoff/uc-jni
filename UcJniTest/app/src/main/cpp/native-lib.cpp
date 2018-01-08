#include "../../../../../uc-jni.hpp"
#include "androidlog.hpp"
#include <string>
#include <thread>
#include <stdexcept>
#include <algorithm>

#define TO_STRING_(n)	#n
#define TO_STRING(n)	TO_STRING_(n)
#define CODE_POSITION	__FILE__ "#" TO_STRING( __LINE__ )

#define JNI(ret, name) extern "C" JNIEXPORT ret JNICALL Java_com_example_uc_ucjnitest_UcJniTest_ ## name

#define TEST_ASSERT(pred) if (!(pred)) { throw std::runtime_error(#pred "\nat " CODE_POSITION); }
#define TEST_ASSERT_EQUALS(expected, actual) TEST_ASSERT(expected == actual)
#define TEST_ASSERT_NOT_EQUALS(unexpected, actual) TEST_ASSERT(unexpected != actual)



DEFINE_JCLASS_ALIAS(System, java/lang/System);
DEFINE_JCLASS_ALIAS(Log, android/util/Log);
DEFINE_JCLASS_ALIAS(UcJniTest, com/example/uc/ucjnitest/UcJniTest);

uc::jni::static_method<System, void()> gc{};
uc::jni::static_method<Log, int(std::string, jstring)> logd{};
uc::jni::method<jstring, int(jstring)> compareJStrings{};

using namespace uc;

jint JNI_OnLoad(JavaVM * vm, void * __unused reserved)
{
    uc::jni::java_vm(vm);

    gc = uc::jni::make_static_method<System, void()>("gc");
    logd = uc::jni::make_static_method<Log, int(std::string, jstring)>("d");
    compareJStrings = uc::jni::make_method<jstring, int(jstring)>("compareTo");
    
    return JNI_VERSION_1_6;
}

JNI(void, samplePoint)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {

        DEFINE_JCLASS_ALIAS(Point, android/graphics/Point);

        auto makePoint = jni::make_constructor<Point(int,int)>();
        auto x = jni::make_field<Point, int>("x");
        auto y = jni::make_field<Point, int>("y");

        auto toString = jni::make_method<jobject, std::string()>("toString");
        auto equals = jni::make_method<jobject, bool(jobject)>("equals");
        auto offset = jni::make_method<Point, void(int,int)>("offset");


        auto p0 = makePoint(12, 34);
        auto p1 = makePoint(12, 34);
        auto p2 = makePoint(123, 456);

        LOGD << toString(p0) << ", " << toString(p1) << ", " << toString(p2);

        TEST_ASSERT_EQUALS(x.get(p0), 12);
        TEST_ASSERT_EQUALS(x.get(p1), 12);
        TEST_ASSERT_EQUALS(x.get(p2), 123);
        TEST_ASSERT_EQUALS(y.get(p0), 34);
        TEST_ASSERT_EQUALS(y.get(p1), 34);
        TEST_ASSERT_EQUALS(y.get(p2), 456);

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

    });
}



JNI(void, testWeakRef)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {
        auto str = uc::jni::make_global(uc::jni::to_jstring("Hello World"));
        auto weakp = uc::jni::weak_ref<jstring>(str);

        TEST_ASSERT(weakp.is_same(str));
        TEST_ASSERT(!weakp.expired());
        {
            auto str2 = weakp.lock();
            TEST_ASSERT(weakp.is_same(str2));
        }

        str.reset();

        TEST_ASSERT(!weakp.expired());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        gc();

        // TEST_ASSERT(weakp.expired());
    });
}

JNI(void, testIsInstanceOf)(JNIEnv *env, jobject thiz)
{
    jni::exception_guard([&] {
        TEST_ASSERT(jni::is_instance_of<jobject>(thiz));
        TEST_ASSERT(jni::is_instance_of<UcJniTest>(thiz));
        TEST_ASSERT(!jni::is_instance_of<jstring>(thiz));

        auto str = uc::jni::to_jstring("Hello World");
        TEST_ASSERT(jni::is_instance_of<jobject>(str));
        TEST_ASSERT(!jni::is_instance_of<UcJniTest>(str));
        TEST_ASSERT(jni::is_instance_of<jstring>(str));
    });
}

JNI(void, testToJstring)(JNIEnv *env, jobject thiz)
{
    jni::exception_guard([&] {
        {
            auto field = uc::jni::make_static_field<UcJniTest, jstring>("staticFieldString");
            auto str = jni::to_jstring("Hello World!");
            logd(__func__, str.get());
            TEST_ASSERT(compareJStrings(str, field.get().get()) == 0);
        }
        {
            auto field = uc::jni::make_static_field<UcJniTest, jstring>("staticFieldStringJp");
            auto str = jni::to_jstring(u"こんにちは、世界！");
            logd(__func__, str.get());
            TEST_ASSERT(compareJStrings(str, field.get().get()) == 0);
        }
    });
}
JNI(void, testToString)(JNIEnv *env, jobject thiz)
{
    jni::exception_guard([&] {
        {
            auto field = uc::jni::make_static_field<UcJniTest, jstring>("staticFieldString");
            auto str = jni::to_string(field.get());
            TEST_ASSERT(str == "Hello World!");
        }
        {
            auto field = uc::jni::make_static_field<UcJniTest, std::string>("staticFieldString");
            TEST_ASSERT(field.get() == "Hello World!");
        }
        {
            auto field = uc::jni::make_static_field<UcJniTest, jstring>("staticFieldStringJp");
            auto str = jni::to_u16string(field.get());
            TEST_ASSERT(str == u"こんにちは、世界！");
        }
        {
            auto field = uc::jni::make_static_field<UcJniTest, std::u16string>("staticFieldStringJp");
            TEST_ASSERT(field.get() == u"こんにちは、世界！");
        }
    });
}


JNI(void, testTypeTraitsSignature)(JNIEnv *env, jobject thiz)
{
    jni::exception_guard([&] {
        TEST_ASSERT_EQUALS(std::string("V"), jni::type_traits<void>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("Z"), jni::type_traits<jboolean>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("B"), jni::type_traits<jbyte>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("C"), jni::type_traits<jchar>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("S"), jni::type_traits<jshort>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("I"), jni::type_traits<jint>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("J"), jni::type_traits<jlong>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("D"), jni::type_traits<jdouble>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("F"), jni::type_traits<jfloat>::signature().c_str());

        TEST_ASSERT_EQUALS(std::string("Ljava/lang/Object;"), jni::type_traits<jobject>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("Ljava/lang/String;"), jni::type_traits<jstring>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("Lcom/example/uc/ucjnitest/UcJniTest;"), jni::type_traits<UcJniTest>::signature().c_str());

        TEST_ASSERT_EQUALS(std::string("[Z"), jni::type_traits<jbooleanArray>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[B"), jni::type_traits<jbyteArray>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[C"), jni::type_traits<jcharArray>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[S"), jni::type_traits<jshortArray>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[I"), jni::type_traits<jintArray>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[J"), jni::type_traits<jlongArray>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[D"), jni::type_traits<jdoubleArray>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[F"), jni::type_traits<jfloatArray>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[Ljava/lang/Object;"), jni::type_traits<jobjectArray>::signature().c_str());


        TEST_ASSERT_EQUALS(std::string("Z"), jni::type_traits<bool>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("B"), jni::type_traits<char>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("C"), jni::type_traits<char16_t>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("Ljava/lang/String;"), jni::type_traits<std::string>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("Ljava/lang/String;"), jni::type_traits<std::u16string>::signature().c_str());

        TEST_ASSERT_EQUALS(std::string("[Z"), jni::type_traits<std::vector<jboolean>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[B"), jni::type_traits<std::vector<jbyte>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[C"), jni::type_traits<std::vector<jchar>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[S"), jni::type_traits<std::vector<jshort>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[I"), jni::type_traits<std::vector<jint>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[J"), jni::type_traits<std::vector<jlong>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[D"), jni::type_traits<std::vector<jdouble>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[F"), jni::type_traits<std::vector<jfloat>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[Ljava/lang/Object;"), jni::type_traits<std::vector<jobject>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[Ljava/lang/String;"), jni::type_traits<std::vector<jstring>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[Lcom/example/uc/ucjnitest/UcJniTest;"), jni::type_traits<std::vector<UcJniTest>>::signature().c_str());


        TEST_ASSERT_EQUALS(std::string("()V"), jni::type_traits<void()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()Z"), jni::type_traits<jboolean()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()B"), jni::type_traits<jbyte()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()C"), jni::type_traits<jchar()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()S"), jni::type_traits<jshort()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()I"), jni::type_traits<jint()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()J"), jni::type_traits<jlong()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()D"), jni::type_traits<jdouble()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()F"), jni::type_traits<jfloat()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()Ljava/lang/Object;"), jni::type_traits<jobject()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()Ljava/lang/String;"), jni::type_traits<jstring()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()Lcom/example/uc/ucjnitest/UcJniTest;"), jni::type_traits<UcJniTest()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()[Z"), jni::type_traits<jbooleanArray()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()[B"), jni::type_traits<jbyteArray()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()[C"), jni::type_traits<jcharArray()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()[S"), jni::type_traits<jshortArray()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()[I"), jni::type_traits<jintArray()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()[J"), jni::type_traits<jlongArray()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()[D"), jni::type_traits<jdoubleArray()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()[F"), jni::type_traits<jfloatArray()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()[Ljava/lang/Object;"), jni::type_traits<jobjectArray()>::signature().c_str());

        TEST_ASSERT_EQUALS(std::string("(ZBCSIJLjava/lang/String;DF)V"), jni::type_traits<void(jboolean,jbyte,jchar,jshort,jint,jlong,jstring,jdouble,jfloat)>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("([Ljava/lang/Object;[Z[B[C[S[I[J[D[F)Ljava/lang/String;"), jni::type_traits<jstring(jobjectArray,jbooleanArray,jbyteArray,jcharArray,jshortArray,jintArray,jlongArray,jdoubleArray,jfloatArray)>::signature().c_str());
    });
}


template<typename T> void testStaticField(const char* fieldName, const T& value1, const T& value2)
{
    auto field = uc::jni::make_static_field<UcJniTest, T>(fieldName);
    field.set(value1);
    TEST_ASSERT_EQUALS(value1, field.get());
    TEST_ASSERT_NOT_EQUALS(value2, field.get());

    field.set(value2);
    TEST_ASSERT_EQUALS(value2, field.get());
    TEST_ASSERT_NOT_EQUALS(value1, field.get());
}
template<typename T> void testField(const char* fieldName, jobject thiz, const T& value1, const T& value2)
{
    auto field = uc::jni::make_field<UcJniTest, T>(fieldName);
    field.set(thiz, value1);
    TEST_ASSERT_EQUALS(value1, field.get(thiz));
    TEST_ASSERT_NOT_EQUALS(value2, field.get(thiz));

    field.set(thiz, value2);
    TEST_ASSERT_EQUALS(value2, field.get(thiz));
    TEST_ASSERT_NOT_EQUALS(value1, field.get(thiz));
}

template<typename T> void testStaticMethod(const char* fieldName, 
    const char* setMethodName, const char* getMethodName, const T& value1, const T& value2)
{
    auto field = uc::jni::make_static_field<UcJniTest, T>(fieldName);
    auto setter = uc::jni::make_static_method<UcJniTest, void(T)>(setMethodName);
    auto getter = uc::jni::make_static_method<UcJniTest, T()>(getMethodName);

    setter(value1);
    TEST_ASSERT_EQUALS(value1, field.get());
    TEST_ASSERT_NOT_EQUALS(value2, field.get());

    TEST_ASSERT_EQUALS(value1, getter());
    TEST_ASSERT_NOT_EQUALS(value2, getter());

    setter(value2);
    TEST_ASSERT_EQUALS(value2, field.get());
    TEST_ASSERT_NOT_EQUALS(value1, field.get());

    TEST_ASSERT_EQUALS(value2, getter());
    TEST_ASSERT_NOT_EQUALS(value1, getter());
}
template<typename T> void testMethod(const char* fieldName,
    const char* setMethodName, const char* getMethodName, 
    jobject thiz, const T& value1, const T& value2)
{
    auto field = uc::jni::make_field<UcJniTest, T>(fieldName);
    auto setter = uc::jni::make_method<UcJniTest, void(T)>(setMethodName);
    auto getter = uc::jni::make_method<UcJniTest, T()>(getMethodName);

    setter(thiz, value1);
    TEST_ASSERT_EQUALS(value1, field.get(thiz));
    TEST_ASSERT_NOT_EQUALS(value2, field.get(thiz));

    TEST_ASSERT_EQUALS(value1, getter(thiz));
    TEST_ASSERT_NOT_EQUALS(value2, getter(thiz));

    setter(thiz, value2);
    TEST_ASSERT_EQUALS(value2, field.get(thiz));
    TEST_ASSERT_NOT_EQUALS(value1, field.get(thiz));

    TEST_ASSERT_EQUALS(value2, getter(thiz));
    TEST_ASSERT_NOT_EQUALS(value1, getter(thiz));
}

#define DEFINE_FIELD_AND_METHOD_TEST(type, methodType, value1, value2) \
JNI(void, testStaticField ## methodType)(JNIEnv *env, jobject thiz)\
{\
    jni::exception_guard([&] { testStaticField<type>( "staticField" #methodType, value1, value2); });\
}\
JNI(void, testField ## methodType)(JNIEnv *env, jobject thiz)\
{\
    jni::exception_guard([&] { testField<type>( "field" #methodType, thiz, value1, value2); });\
}\
JNI(void, testStaticMethod ## methodType)(JNIEnv *env, jobject thiz)\
{\
    jni::exception_guard([&] { testStaticMethod<type>( "staticField" #methodType, "setStaticField" #methodType, "getStaticField" #methodType, value1, value2); });\
}\
JNI(void, testMethod ## methodType)(JNIEnv *env, jobject thiz)\
{\
    jni::exception_guard([&] { testMethod<type>( "field" #methodType, "setField" #methodType, "getField" #methodType, thiz, value1, value2); });\
}
DEFINE_FIELD_AND_METHOD_TEST(bool,           Bool,     false, true)
DEFINE_FIELD_AND_METHOD_TEST(jbyte,          Byte,     0xff, 0xcc)
DEFINE_FIELD_AND_METHOD_TEST(jshort,         Short,    123, 456)
DEFINE_FIELD_AND_METHOD_TEST(jint,           Int,      1234, 5678)
DEFINE_FIELD_AND_METHOD_TEST(jlong,          Long,     1234, 5678)
DEFINE_FIELD_AND_METHOD_TEST(jfloat,         Float,    123.4f, 567.8f)
DEFINE_FIELD_AND_METHOD_TEST(jdouble,        Double,   1.2345, 6.7891)
DEFINE_FIELD_AND_METHOD_TEST(std::string,    String,   "Hello World!", "Hello Java.")
DEFINE_FIELD_AND_METHOD_TEST(std::u16string, StringJp,  u"こんにちは、世界！", u"こんばんわ, Java.")

JNI(void, testArrayField)(JNIEnv *env, jobject thiz)
{
    jni::exception_guard([&] {
        {
            auto field = uc::jni::make_static_field<UcJniTest, jintArray>("staticFieldIntArray");
            auto array = field.get();
            TEST_ASSERT_EQUALS(jni::length(array), 5);

            int i = 1;
            for (auto&& v : jni::get_elements(array)) {
                TEST_ASSERT_EQUALS(i, v);
                ++i;
            }
        }
        {
            auto field = uc::jni::make_static_field<UcJniTest, std::vector<jchar>>("staticFieldCharArray");
            auto fieldValues = field.get();
            std::vector<char16_t> values(fieldValues.size());
            std::transform(fieldValues.begin(), fieldValues.end(), values.begin(), [](auto&& v) { return static_cast<char16_t>(v); });
            const auto answer = std::vector<char16_t> {u'あ', u'い', u'う', u'え', u'お'};
            TEST_ASSERT_EQUALS(answer, values);
        }
        {
            auto field = uc::jni::make_static_field<UcJniTest, std::vector<int>>("staticFieldIntArray");
            const auto answer = std::vector<int> {1, 2, 3, 4, 5};
            TEST_ASSERT_EQUALS(answer, field.get());
        }
        {
            auto field = uc::jni::make_field<UcJniTest, std::vector<bool>>("fieldBoolArray");

            const auto answer = std::vector<bool> {false, true, false, true, false};
            TEST_ASSERT_EQUALS(answer, field.get(thiz));
        }
        {
            auto field = uc::jni::make_field<UcJniTest, std::vector<jstring>>("fieldStringArray");
            auto fieldValues = field.get(thiz);
            std::vector<std::u16string> values(fieldValues.size());
            std::transform(fieldValues.begin(), fieldValues.end(), values.begin(), [](auto&& v) { return jni::to_u16string(v); });
            const auto answer = std::vector<std::u16string> { u"Hello", u"World!", u"こんにちは", u"世界" };
            TEST_ASSERT_EQUALS(answer, values);
        }
        // {
        //     auto field = uc::jni::make_field<UcJniTest, std::vector<std::string>>("fieldStringArray");
        //     const auto answer = std::vector<std::string> { "Hello", "World!", "こんにちは", "世界" };
        //     TEST_ASSERT_EQUALS(answer, field.get(thiz));
        // }
    });
}

