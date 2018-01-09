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
#define STATIC_ASSERT(pred) static_assert(pred, #pred)



DEFINE_JCLASS_ALIAS(System, java/lang/System);
DEFINE_JCLASS_ALIAS(Log, android/util/Log);
DEFINE_JCLASS_ALIAS(UcJniTest, com/example/uc/ucjnitest/UcJniTest);

uc::jni::static_method<System, void()> gc{};
uc::jni::static_method<Log, int(std::string, jstring)> logd{};
uc::jni::method<jstring, int(jstring)> compareJStrings{};

STATIC_ASSERT(sizeof(uc::jni::field<System, int>) == sizeof(jfieldID));
STATIC_ASSERT(sizeof(uc::jni::static_field<System, int>) == sizeof(jfieldID));

STATIC_ASSERT(sizeof(uc::jni::method<System, void()>) == sizeof(jmethodID));
STATIC_ASSERT(sizeof(uc::jni::non_virtual_method<System, void()>) == sizeof(jmethodID));
STATIC_ASSERT(sizeof(uc::jni::static_method<System, void()>) == sizeof(jmethodID));
STATIC_ASSERT(sizeof(uc::jni::constructor<UcJniTest()>) == sizeof(jmethodID));


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

        auto makePoint = uc::jni::make_constructor<Point(int,int)>();

        auto x = uc::jni::make_field<Point, int>("x");
        auto y = uc::jni::make_field<Point, int>("y");

        auto toString = uc::jni::make_method<jobject, std::string()>("toString");
        auto equals = uc::jni::make_method<jobject, bool(jobject)>("equals");
        auto offset = uc::jni::make_method<Point, void(int,int)>("offset");

        // Accessors do not use any extra memory.
        TEST_ASSERT_EQUALS(sizeof(makePoint), sizeof(jmethodID));
        TEST_ASSERT_EQUALS(sizeof(x), sizeof(jfieldID));
        TEST_ASSERT_EQUALS(sizeof(offset), sizeof(jmethodID));


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

uc::jni::global_ref<jstring> globalString{};
uc::jni::weak_ref<jstring> weakString{};

JNI(void, testGlobalRef)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {
        globalString = uc::jni::make_global(uc::jni::to_jstring("Hello World"));

        TEST_ASSERT(globalString);
    });
}

JNI(void, testWeakRef)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {

        TEST_ASSERT(globalString);


        auto jstr = uc::jni::to_jstring("Hello World");

        weakString = uc::jni::weak_ref<jstring>(jstr);
        TEST_ASSERT(weakString.is_same(jstr));
        TEST_ASSERT(!weakString.is_same(globalString));
        TEST_ASSERT(!weakString.expired());
        {
            auto jstr2 = weakString.lock();
            TEST_ASSERT(weakString.is_same(jstr2));
            TEST_ASSERT(uc::jni::is_same_object(jstr, jstr2));
        }

        weakString.reset();
        TEST_ASSERT(!weakString.is_same(jstr));
        TEST_ASSERT(!weakString.is_same(globalString));
        TEST_ASSERT(weakString.expired());


        weakString = uc::jni::weak_ref<jstring>(jstr);
        TEST_ASSERT(weakString.is_same(jstr));
        TEST_ASSERT(!weakString.is_same(globalString));
        TEST_ASSERT(!weakString.expired());
        {
            auto jstr2 = weakString.lock();
            TEST_ASSERT(weakString.is_same(jstr2));
            TEST_ASSERT(uc::jni::is_same_object(jstr, jstr2));
        }


        weakString = globalString;
        TEST_ASSERT(!weakString.is_same(jstr));
        TEST_ASSERT(weakString.is_same(globalString));
        TEST_ASSERT(!weakString.expired());
        {
            auto jstr2 = weakString.lock();
            TEST_ASSERT(weakString.is_same(jstr2));
            TEST_ASSERT(uc::jni::is_same_object(globalString, jstr2));
        }

        globalString.reset();
        TEST_ASSERT(!globalString);

        TEST_ASSERT(!weakString.expired());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        gc();

        // TEST_ASSERT(weakp.expired());
    });
}


JNI(void, testIsInstanceOf)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {
        TEST_ASSERT(uc::jni::is_instance_of<jobject>(thiz));
        TEST_ASSERT(uc::jni::is_instance_of<UcJniTest>(thiz));
        TEST_ASSERT(!uc::jni::is_instance_of<jstring>(thiz));

        auto str = uc::jni::to_jstring("Hello World");
        TEST_ASSERT(uc::jni::is_instance_of<jobject>(str));
        TEST_ASSERT(!uc::jni::is_instance_of<UcJniTest>(str));
        TEST_ASSERT(uc::jni::is_instance_of<jstring>(str));
    });
}

JNI(void, testToJstring)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {
        {
            auto field = uc::jni::make_static_field<UcJniTest, jstring>("staticFieldString");
            auto str = uc::jni::to_jstring("Hello World!");
            logd(__func__, str.get());
            TEST_ASSERT(compareJStrings(str, field.get().get()) == 0);
        }
        {
            auto field = uc::jni::make_static_field<UcJniTest, jstring>("staticFieldStringJp");
            auto str = uc::jni::to_jstring(u"こんにちは、世界！");
            logd(__func__, str.get());
            TEST_ASSERT(compareJStrings(str, field.get().get()) == 0);
        }
    });
}
JNI(void, testToString)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {
        const std::string value = "Hello World!";
        const std::u16string valueJp = u"こんにちは、世界！";

        {
            auto field = uc::jni::make_static_field<UcJniTest, jstring>("staticFieldString");
            auto str = uc::jni::to_string(field.get());
            TEST_ASSERT_EQUALS(value, str);
        }
        {
            auto field = uc::jni::make_static_field<UcJniTest, std::string>("staticFieldString");
            TEST_ASSERT_EQUALS(value, field.get());
        }
        {
            auto field = uc::jni::make_static_field<UcJniTest, jstring>("staticFieldStringJp");
            auto str = uc::jni::to_u16string(field.get());
            TEST_ASSERT_EQUALS(valueJp, str);
        }
        {
            auto field = uc::jni::make_static_field<UcJniTest, std::u16string>("staticFieldStringJp");
            TEST_ASSERT_EQUALS(valueJp, field.get());
        }
        {
            auto jstr = uc::jni::to_jstring(value);
            auto str = uc::jni::to_string(jstr);
            TEST_ASSERT_EQUALS(value, str);
        }
        {
            auto jstr = uc::jni::to_jstring(valueJp);
            auto str = uc::jni::to_u16string(jstr);
            TEST_ASSERT_EQUALS(valueJp, str);
        }
    });
}


JNI(void, testTypeTraitsSignature)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {
        TEST_ASSERT_EQUALS(std::string("V"), uc::jni::type_traits<void>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("Z"), uc::jni::type_traits<jboolean>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("B"), uc::jni::type_traits<jbyte>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("C"), uc::jni::type_traits<jchar>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("S"), uc::jni::type_traits<jshort>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("I"), uc::jni::type_traits<jint>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("J"), uc::jni::type_traits<jlong>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("D"), uc::jni::type_traits<jdouble>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("F"), uc::jni::type_traits<jfloat>::signature().c_str());

        TEST_ASSERT_EQUALS(std::string("Ljava/lang/Object;"), uc::jni::type_traits<jobject>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("Ljava/lang/String;"), uc::jni::type_traits<jstring>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("Lcom/example/uc/ucjnitest/UcJniTest;"), uc::jni::type_traits<UcJniTest>::signature().c_str());

        TEST_ASSERT_EQUALS(std::string("[Z"), uc::jni::type_traits<jbooleanArray>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[B"), uc::jni::type_traits<jbyteArray>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[C"), uc::jni::type_traits<jcharArray>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[S"), uc::jni::type_traits<jshortArray>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[I"), uc::jni::type_traits<jintArray>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[J"), uc::jni::type_traits<jlongArray>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[D"), uc::jni::type_traits<jdoubleArray>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[F"), uc::jni::type_traits<jfloatArray>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[Ljava/lang/Object;"), uc::jni::type_traits<jobjectArray>::signature().c_str());


        TEST_ASSERT_EQUALS(std::string("Z"), uc::jni::type_traits<bool>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("B"), uc::jni::type_traits<char>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("C"), uc::jni::type_traits<char16_t>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("Ljava/lang/String;"), uc::jni::type_traits<std::string>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("Ljava/lang/String;"), uc::jni::type_traits<std::u16string>::signature().c_str());

        TEST_ASSERT_EQUALS(std::string("[Z"), uc::jni::type_traits<std::vector<jboolean>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[B"), uc::jni::type_traits<std::vector<jbyte>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[C"), uc::jni::type_traits<std::vector<jchar>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[S"), uc::jni::type_traits<std::vector<jshort>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[I"), uc::jni::type_traits<std::vector<jint>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[J"), uc::jni::type_traits<std::vector<jlong>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[D"), uc::jni::type_traits<std::vector<jdouble>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[F"), uc::jni::type_traits<std::vector<jfloat>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[Ljava/lang/Object;"), uc::jni::type_traits<std::vector<jobject>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[Ljava/lang/String;"), uc::jni::type_traits<std::vector<jstring>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[Lcom/example/uc/ucjnitest/UcJniTest;"), uc::jni::type_traits<std::vector<UcJniTest>>::signature().c_str());


        TEST_ASSERT_EQUALS(std::string("()V"), uc::jni::type_traits<void()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()Z"), uc::jni::type_traits<jboolean()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()B"), uc::jni::type_traits<jbyte()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()C"), uc::jni::type_traits<jchar()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()S"), uc::jni::type_traits<jshort()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()I"), uc::jni::type_traits<jint()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()J"), uc::jni::type_traits<jlong()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()D"), uc::jni::type_traits<jdouble()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()F"), uc::jni::type_traits<jfloat()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()Ljava/lang/Object;"), uc::jni::type_traits<jobject()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()Ljava/lang/String;"), uc::jni::type_traits<jstring()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()Lcom/example/uc/ucjnitest/UcJniTest;"), uc::jni::type_traits<UcJniTest()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()[Z"), uc::jni::type_traits<jbooleanArray()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()[B"), uc::jni::type_traits<jbyteArray()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()[C"), uc::jni::type_traits<jcharArray()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()[S"), uc::jni::type_traits<jshortArray()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()[I"), uc::jni::type_traits<jintArray()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()[J"), uc::jni::type_traits<jlongArray()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()[D"), uc::jni::type_traits<jdoubleArray()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()[F"), uc::jni::type_traits<jfloatArray()>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("()[Ljava/lang/Object;"), uc::jni::type_traits<jobjectArray()>::signature().c_str());

        TEST_ASSERT_EQUALS(std::string("(ZBCSIJLjava/lang/String;DF)V"), uc::jni::type_traits<void(jboolean,jbyte,jchar,jshort,jint,jlong,jstring,jdouble,jfloat)>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("([Ljava/lang/Object;[Z[B[C[S[I[J[D[F)Ljava/lang/String;"), uc::jni::type_traits<jstring(jobjectArray,jbooleanArray,jbyteArray,jcharArray,jshortArray,jintArray,jlongArray,jdoubleArray,jfloatArray)>::signature().c_str());
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
    uc::jni::exception_guard([&] { testStaticField<type>( "staticField" #methodType, value1, value2); });\
}\
JNI(void, testField ## methodType)(JNIEnv *env, jobject thiz)\
{\
    uc::jni::exception_guard([&] { testField<type>( "field" #methodType, thiz, value1, value2); });\
}\
JNI(void, testStaticMethod ## methodType)(JNIEnv *env, jobject thiz)\
{\
    uc::jni::exception_guard([&] { testStaticMethod<type>( "staticField" #methodType, "setStaticField" #methodType, "getStaticField" #methodType, value1, value2); });\
}\
JNI(void, testMethod ## methodType)(JNIEnv *env, jobject thiz)\
{\
    uc::jni::exception_guard([&] { testMethod<type>( "field" #methodType, "setField" #methodType, "getField" #methodType, thiz, value1, value2); });\
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
    uc::jni::exception_guard([&] {
        {
            const auto answer = std::vector<int> {1, 2, 3, 4, 5};
            {
                auto field = uc::jni::make_static_field<UcJniTest, jintArray>("staticFieldIntArray");
                auto array = field.get();
                TEST_ASSERT_EQUALS(answer.size(), uc::jni::length(array));
                int i = 0;
                for (auto&& v : uc::jni::get_elements(array)) {
                    TEST_ASSERT_EQUALS(answer[i], v);
                    ++i;
                }
                TEST_ASSERT_EQUALS(answer.size(), i);
            }
            {
                auto field = uc::jni::make_static_field<UcJniTest, std::vector<int>>("staticFieldIntArray");
                TEST_ASSERT_EQUALS(answer, field.get());
            }
        }

        {
            const auto answer = std::vector<jchar> {u'あ', u'い', u'う', u'え', u'お'};
            {
                auto field = uc::jni::make_static_field<UcJniTest,jcharArray>("staticFieldCharArray");
                auto array = field.get();
                TEST_ASSERT_EQUALS(answer.size(), uc::jni::length(array));
                int i = 0;
                for (auto&& v : uc::jni::get_elements(array)) {
                    TEST_ASSERT_EQUALS(answer[i], v);
                    ++i;
                }
                TEST_ASSERT_EQUALS(answer.size(), i);
            }
            {
                auto field = uc::jni::make_static_field<UcJniTest, std::vector<jchar>>("staticFieldCharArray");
                TEST_ASSERT_EQUALS(answer, field.get());
            }
            // {
            //     auto field = uc::jni::make_static_field<UcJniTest, std::vector<jchar>>("staticFieldCharArray");
            //     auto fieldValues = field.get();
            //     std::vector<char16_t> values(fieldValues.size());
            //     std::transform(fieldValues.begin(), fieldValues.end(), values.begin(), [](auto&& v) { return static_cast<char16_t>(v); });
            //     const auto answer = std::vector<char16_t> {u'あ', u'い', u'う', u'え', u'お'};
            //     TEST_ASSERT_EQUALS(answer, values);
            // }
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
            std::transform(fieldValues.begin(), fieldValues.end(), values.begin(), [](auto&& v) { return uc::jni::to_u16string(v); });
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

JNI(void, testArray)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {
        auto array = uc::jni::new_array<jint>(10);

        TEST_ASSERT_EQUALS(10, uc::jni::length(array));

        const int values1[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
        const int values2[] = {20, 21, 22, 23, 24, 25, 26, 27, 28, 29};

        {
            auto elems = uc::jni::get_elements(array.get());
            std::copy(std::begin(values1), std::end(values1), uc::jni::begin(elems));

            TEST_ASSERT(!std::equal(std::begin(values1), std::end(values1), uc::jni::begin(uc::jni::get_elements(array.get()))));
        }
        TEST_ASSERT(std::equal(std::begin(values1), std::end(values1), uc::jni::begin(uc::jni::get_elements(array.get()))));

        {
            auto elems = uc::jni::get_elements(array);
            std::copy(std::begin(values2), std::end(values2), uc::jni::begin(elems));
            uc::jni::set_abort(elems);
        }
        TEST_ASSERT(std::equal(std::begin(values1), std::end(values1), uc::jni::begin(uc::jni::get_elements(array.get()))));

        {
            auto elems = uc::jni::get_elements(array, true);
            std::copy(std::begin(values2), std::end(values2), uc::jni::begin(elems));
        }
        TEST_ASSERT(std::equal(std::begin(values1), std::end(values1), uc::jni::begin(uc::jni::get_elements(array.get()))));


        {
            auto elems = uc::jni::get_elements(array);
            std::copy(std::begin(values2), std::end(values2), uc::jni::begin(elems));

            TEST_ASSERT(std::equal(std::begin(values1), std::end(values1), uc::jni::begin(uc::jni::get_elements(array.get()))));
            uc::jni::commit(elems);
            TEST_ASSERT(std::equal(std::begin(values2), std::end(values2), uc::jni::begin(uc::jni::get_elements(array.get()))));
        }
    });
LOGD << "###############################################";
}
