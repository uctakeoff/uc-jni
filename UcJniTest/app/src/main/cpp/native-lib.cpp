#define UC_JNI_BETA_VERSION
#include "androidlog.hpp"
#include "../../../../../uc-jni.hpp"
#include <string>
#include <thread>
#include <stdexcept>
#include <algorithm>
#include <future>

#define TO_STRING_(n)	#n
#define TO_STRING(n)	TO_STRING_(n)
#define CODE_POSITION	__FILE__ ":" TO_STRING( __LINE__ )
#define JNI(ret, name) extern "C" JNIEXPORT ret JNICALL Java_com_example_uc_ucjnitest_UcJniTest_ ## name

//*************************************************************************************************
// TEST Macro
//*************************************************************************************************
#define TEST_ASSERT(pred) if (!(pred)) { throw std::runtime_error(#pred "\nat " CODE_POSITION); }
#define TEST_ASSERT_EQUALS(expected, actual) TEST_ASSERT(expected == actual)
#define TEST_ASSERT_NOT_EQUALS(unexpected, actual) TEST_ASSERT(unexpected != actual)
#define STATIC_ASSERT(pred) static_assert(pred, #pred)
#define STATIC_ASSERT_EQUALS(expected, actual) STATIC_ASSERT(expected == actual)
#define STATIC_ASSERT_NOT_EQUALS(unexpected, actual) STATIC_ASSERT(unexpected != actual)


//*************************************************************************************************
// Class FQCN
//*************************************************************************************************
UC_JNI_DEFINE_JCLASS_ALIAS(System, java/lang/System);
UC_JNI_DEFINE_JCLASS_ALIAS(Log, android/util/Log);
// UC_JNI_DEFINE_JCLASS_ALIAS(UcJniTest, com/example/uc/ucjnitest/UcJniTest);


UC_JNI_DEFINE_JCLASS(jPoint, com/example/uc/ucjnitest/Point)
{
    UC_JNI_DEFINE_JCLASS_STATIC_FINAL_FIELD(jPoint, ZERO)
    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(jPoint, add, jPoint, jPoint)
    UC_JNI_DEFINE_JCLASS_CONSTRUCTOR(jPoint)
    UC_JNI_DEFINE_JCLASS_CONSTRUCTOR(double, double)
    UC_JNI_DEFINE_JCLASS_FIELD(double, x)
    UC_JNI_DEFINE_JCLASS_FIELD(double, y)
    UC_JNI_DEFINE_JCLASS_METHOD(double, norm)
    UC_JNI_DEFINE_JCLASS_METHOD(void, multiply, double)
};

UC_JNI_DEFINE_JCLASS(InnerA, com/example/uc/ucjnitest/UcJniTest$InnerA)
{
    UC_JNI_DEFINE_JCLASS_CONSTRUCTOR()
    UC_JNI_DEFINE_JCLASS_METHOD(std::string, getString)
    UC_JNI_DEFINE_JCLASS_METHOD(void, setString, std::string)
    UC_JNI_DEFINE_JCLASS_OVERLOADED_METHOD(void, setString, InnerA_*)
    UC_JNI_DEFINE_JCLASS_FIELD(std::string, value)
};

UC_JNI_DEFINE_JCLASS_DERIVED(InnerB, com/example/uc/ucjnitest/UcJniTest$InnerB, InnerA)
{
    UC_JNI_DEFINE_JCLASS_CONSTRUCTOR()
    UC_JNI_DEFINE_JCLASS_METHOD(std::string, getString)
};

UC_JNI_DEFINE_JCLASS(UcJniTest, com/example/uc/ucjnitest/UcJniTest)
{
    UC_JNI_DEFINE_JCLASS_STATIC_FIELD(jboolean, staticFieldBool);
    UC_JNI_DEFINE_JCLASS_STATIC_FIELD(jbyte   , staticFieldByte);
    UC_JNI_DEFINE_JCLASS_STATIC_FIELD(jshort  , staticFieldShort);
    UC_JNI_DEFINE_JCLASS_STATIC_FIELD(jint    , staticFieldInt);
    UC_JNI_DEFINE_JCLASS_STATIC_FIELD(jlong   , staticFieldLong);
    UC_JNI_DEFINE_JCLASS_STATIC_FIELD(jfloat  , staticFieldFloat);
    UC_JNI_DEFINE_JCLASS_STATIC_FIELD(jdouble , staticFieldDouble);
    UC_JNI_DEFINE_JCLASS_STATIC_FIELD(std::string , staticFieldString);
    UC_JNI_DEFINE_JCLASS_STATIC_FIELD(std::u16string , staticFieldStringJp);
    UC_JNI_DEFINE_JCLASS_STATIC_FIELD(jintArray    , staticFieldIntArray);

    UC_JNI_DEFINE_JCLASS_FIELD(jboolean, fieldBool);
    UC_JNI_DEFINE_JCLASS_FIELD(jbyte   , fieldByte);
    UC_JNI_DEFINE_JCLASS_FIELD(jshort  , fieldShort);
    UC_JNI_DEFINE_JCLASS_FIELD(jint    , fieldInt);
    UC_JNI_DEFINE_JCLASS_FIELD(jlong   , fieldLong);
    UC_JNI_DEFINE_JCLASS_FIELD(jfloat  , fieldFloat);
    UC_JNI_DEFINE_JCLASS_FIELD(jdouble , fieldDouble);
    UC_JNI_DEFINE_JCLASS_FIELD(jstring , fieldString);
    UC_JNI_DEFINE_JCLASS_FIELD(jstring , fieldStringJp);
    UC_JNI_DEFINE_JCLASS_FIELD(jintArray, fieldIntArray);

    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(void, setStaticFieldBool    , jboolean)
    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(void, setStaticFieldByte    , jbyte   )
    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(void, setStaticFieldShort   , jshort  )
    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(void, setStaticFieldInt     , jint    )
    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(void, setStaticFieldLong    , jlong   )
    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(void, setStaticFieldFloat   , jfloat  )
    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(void, setStaticFieldDouble  , jdouble )
    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(void, setStaticFieldString  , jstring )
    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(void, setStaticFieldStringJp, jstring )
    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(void, setStaticFieldIntArray, jintArray)

    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(jboolean, getStaticFieldBool)
    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(jbyte   , getStaticFieldByte)
    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(jshort  , getStaticFieldShort)
    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(jint    , getStaticFieldInt)
    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(jlong   , getStaticFieldLong)
    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(jfloat  , getStaticFieldFloat)
    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(jdouble , getStaticFieldDouble)
    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(jstring , getStaticFieldString)
    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(jstring , getStaticFieldStringJp)
    UC_JNI_DEFINE_JCLASS_STATIC_METHOD(jintArray,getStaticFieldIntArray)

    UC_JNI_DEFINE_JCLASS_METHOD(void, setFieldBool    , jboolean)
    UC_JNI_DEFINE_JCLASS_METHOD(void, setFieldByte    , jbyte)
    UC_JNI_DEFINE_JCLASS_METHOD(void, setFieldShort   , jshort)
    UC_JNI_DEFINE_JCLASS_METHOD(void, setFieldInt     , jint)
    UC_JNI_DEFINE_JCLASS_METHOD(void, setFieldLong    , jlong)
    UC_JNI_DEFINE_JCLASS_METHOD(void, setFieldFloat   , jfloat)
    UC_JNI_DEFINE_JCLASS_METHOD(void, setFieldDouble  , jdouble)
    UC_JNI_DEFINE_JCLASS_METHOD(void, setFieldString  , jstring)
    UC_JNI_DEFINE_JCLASS_METHOD(void, setFieldStringJp, jstring)
    UC_JNI_DEFINE_JCLASS_METHOD(void, setFieldIntArray, jintArray)

    UC_JNI_DEFINE_JCLASS_METHOD(jboolean, getFieldBool)
    UC_JNI_DEFINE_JCLASS_METHOD(jbyte   , getFieldByte)
    UC_JNI_DEFINE_JCLASS_METHOD(jshort  , getFieldShort)
    UC_JNI_DEFINE_JCLASS_METHOD(jint    , getFieldInt)
    UC_JNI_DEFINE_JCLASS_METHOD(jlong   , getFieldLong)
    UC_JNI_DEFINE_JCLASS_METHOD(jfloat  , getFieldFloat)
    UC_JNI_DEFINE_JCLASS_METHOD(jdouble , getFieldDouble)
    UC_JNI_DEFINE_JCLASS_METHOD(jstring , getFieldString)
    UC_JNI_DEFINE_JCLASS_METHOD(jstring , getFieldStringJp)
    UC_JNI_DEFINE_JCLASS_METHOD(jintArray, getFieldIntArray)
};


//*************************************************************************************************
// Test sizeof
//*************************************************************************************************
STATIC_ASSERT_EQUALS(sizeof(uc::jni::field<System, int>), sizeof(jfieldID));
STATIC_ASSERT_EQUALS(sizeof(uc::jni::static_field<System, int>), sizeof(jfieldID));
STATIC_ASSERT_EQUALS(sizeof(uc::jni::method<System, void()>), sizeof(jmethodID));
STATIC_ASSERT_EQUALS(sizeof(uc::jni::non_virtual_method<System, void()>), sizeof(jmethodID));
STATIC_ASSERT_EQUALS(sizeof(uc::jni::static_method<System, void()>), sizeof(jmethodID));
STATIC_ASSERT_EQUALS(sizeof(uc::jni::constructor<UcJniTest()>), sizeof(jmethodID));

//*************************************************************************************************
// Static Variables
//*************************************************************************************************
// void System.gc()
uc::jni::static_method<System, void()> gc{};

// int Log.d(String, String)
uc::jni::static_method<Log, int(std::string, jstring)> logd{};

// int String.compareTo(String)
uc::jni::method<jstring, int(jstring)> compareJStrings{};

// String Class.getName()
uc::jni::method<jclass, std::string()> getClassName{};



//*************************************************************************************************
// Sample
//*************************************************************************************************
jint JNI_OnLoad(JavaVM * vm, void * __unused reserved)
{
    uc::jni::java_vm(vm);
    uc::jni::replace_with_class_loader_find_class<UcJniTest>();

    gc = uc::jni::make_static_method<System, void()>("gc");
    logd = uc::jni::make_static_method<Log, int(std::string, jstring)>("d");
    compareJStrings = uc::jni::make_method<jstring, int(jstring)>("compareTo");
    getClassName = uc::jni::make_method<jclass, std::string()>("getName");

    return JNI_VERSION_1_6;
}

void sampleCodeWithMacros()
{
        auto a = InnerA_::new_();
        auto b = InnerB_::new_();
        auto c = static_cast<InnerA>(b.get());

        a->value("a");
        b->value("b");
        TEST_ASSERT_EQUALS(a->value(), "a");
        TEST_ASSERT_EQUALS(b->value(), "b");
        TEST_ASSERT_EQUALS(c->value(), "b");
        TEST_ASSERT_EQUALS(a->getString(), "a from InnerA");
        TEST_ASSERT_EQUALS(b->getString(), "b from InnerB");
        TEST_ASSERT_EQUALS(c->getString(), "b from InnerB");
        TEST_ASSERT_EQUALS(a->getStringNonVirtual(), "a from InnerA");
        TEST_ASSERT_EQUALS(b->getStringNonVirtual(), "b from InnerB");
        TEST_ASSERT_EQUALS(c->getStringNonVirtual(), "b from InnerA");

        b->setString(a);
        a->setString(std::string("A"));
        TEST_ASSERT_EQUALS(a->getString(), "A from InnerA");
        TEST_ASSERT_EQUALS(b->getString(), "a from InnerB");
        TEST_ASSERT_EQUALS(c->getString(), "a from InnerB");

        // Constructor Method
        // p0,p1,p2 are smart pointer. release automatically.
        auto p0 = jPoint_::new_(3, 4);       // p0 = new Point(12, 34);
        auto p1 = jPoint_::new_(p0);         // p1 = new Point(p0);
        LOGD << p0->toString() << ", " << p1->toString();

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

        p0->multiply(10);
        TEST_ASSERT(p0->equals(p1));          // p0.equals(p1)
//        TEST_ASSERT(p0->toString(), std::string("(30,40)"));
        LOGD << p0->toString() << ", " << p1->toString();

        // get static final field
        TEST_ASSERT_EQUALS(jPoint_::ZERO()->x(), 0);
        TEST_ASSERT_EQUALS(jPoint_::ZERO()->y(), 0);

        // call static method
        auto p2 = jPoint_::add(p0, p1);
        TEST_ASSERT_EQUALS(p2->x(), 60);
        TEST_ASSERT_EQUALS(p2->y(), 80);
        LOGD << p0->toString() << ", " << p1->toString() << ", " << p2->toString();
}

JNI(void, samplePoint)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {

        sampleCodeWithMacros();


        UC_JNI_DEFINE_JCLASS_ALIAS(Point, android/graphics/Point);

        const auto newPoint = uc::jni::make_constructor<Point(int,int)>();

        auto x = uc::jni::make_field<Point, int>("x");
        auto y = uc::jni::make_field<Point, int>("y");

        auto toString = uc::jni::make_method<jobject, std::string()>("toString");
        auto equals = uc::jni::make_method<jobject, bool(jobject)>("equals");
        auto offset = uc::jni::make_method<Point, void(int,int)>("offset");

        // Accessors do not use any extra memory.
        TEST_ASSERT_EQUALS(sizeof(newPoint), sizeof(jmethodID));
        TEST_ASSERT_EQUALS(sizeof(x), sizeof(jfieldID));
        TEST_ASSERT_EQUALS(sizeof(offset), sizeof(jmethodID));


        auto p0 = newPoint(12, 34);
        auto p1 = newPoint(12, 34);
        auto p2 = newPoint(123, 456);

        LOGD << "#########################################";
//        LOGD << "####" << getClassName(p0);
        LOGD << "####" << getClassName(uc::jni::get_object_class(p0));
        TEST_ASSERT_EQUALS(std::string(Point_::fqcn()), "android/graphics/Point");
        TEST_ASSERT_EQUALS(getClassName(uc::jni::get_object_class(p0)), "android.graphics.Point");

        LOGD << toString(p0) << ", " << toString(p1) << ", " << toString(p2);

        TEST_ASSERT_EQUALS(x.get(p0), 12);
        TEST_ASSERT_EQUALS(x.get(p1), 12);
        TEST_ASSERT_EQUALS(x.get(p2), 123);
        TEST_ASSERT_EQUALS(y.get(p0), 34);
        TEST_ASSERT_EQUALS(y.get(p1), 34);
        TEST_ASSERT_EQUALS(y.get(p2), 456);

        TEST_ASSERT(equals(p0, p1));
        TEST_ASSERT(!equals(p1, p2));

        x.set(p1, 123);
        y.set(p1, 456);

        TEST_ASSERT(!equals(p0, p1));
        TEST_ASSERT(equals(p1, p2));

        offset(p0, 100, 200);
        TEST_ASSERT_EQUALS(x.get(p0), 112);
        TEST_ASSERT_EQUALS(y.get(p0), 234);
        LOGD << toString(p0) << ", " << toString(p1) << ", " << toString(p2);

    });
}


//*************************************************************************************************
// Test Exception
//*************************************************************************************************
JNI(void, throwRuntimeError)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([] {
        throw std::runtime_error("std::runtime_error");
    });
}
JNI(void, throwBadAlloc)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([] {
        throw std::bad_alloc();
    });
}
JNI(void, throwInt)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([] {
        throw 1;
    });
}


//*************************************************************************************************
// Test get_class() API Spec.
//*************************************************************************************************
#include<mutex>
#include<unordered_map>
inline jclass cached_find_class(JNIEnv* env, const char* name)
{
    static std::mutex mutex{};
    static std::unordered_map<std::string, uc::jni::local_ref<jclass>> table;
    std::lock_guard<std::mutex> lk(mutex);
    auto i = table.find(name);
    if (i == table.end()) {
        auto cls = env->FindClass(name);
        if (!cls) return cls;
        i = table.emplace(name, uc::jni::make_local(cls)).first;
    }
    return i->second.get();
}

JNI(void, testResolveClass)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {
        using namespace std::chrono;
        using clock_type = std::chrono::high_resolution_clock;
        UC_JNI_DEFINE_JCLASS_ALIAS(Point, android/graphics/Point);

        const auto loopCount = 100000;
        {
            auto start = clock_type::now();
            for (size_t i = 0; i < loopCount; ++i) {
                auto cls = env->FindClass("android/graphics/Point");
                env->DeleteLocalRef(cls);
            }
            auto end = clock_type::now();
            LOGD << "## FindClass()  : " << duration_cast<microseconds>(end - start).count() << "us";
        }

        {
            auto start = clock_type::now();
            for (size_t i = 0; i < loopCount; ++i) {
                cached_find_class(env, "android/graphics/Point");
            }
            auto end = clock_type::now();
            LOGD << "## FindClass()+unordered_map  : " << duration_cast<microseconds>(end - start).count() << "us";
        }

        {
            auto start = clock_type::now();
            for (size_t i = 0; i < loopCount; ++i) {
                uc::jni::get_class<Point>();
            }
            auto end = clock_type::now();
            LOGD << "## uc::jni::get_class()  : " << duration_cast<microseconds>(end - start).count() << "us";
        }


        std::string str;
        str.reserve(loopCount*20);
        {
            auto start = clock_type::now();
            for (size_t i = 0; i < loopCount; ++i) {
                str.append(uc::jni::type_traits<jobjectArray>::signature().c_str());
            }
            auto end = clock_type::now();
            LOGD << "## uc::jni::type_traits<T>::signature().c_str()  : " << duration_cast<microseconds>(end - start).count() << "us";
        }
        {
            auto start = clock_type::now();
            for (size_t i = 0; i < loopCount; ++i) {
                str.append(uc::jni::get_signature<jobjectArray>());
            }
            auto end = clock_type::now();
            LOGD << "## uc::jni::get_signature<T>()  : " << duration_cast<microseconds>(end - start).count() << "us";
        }

    });
}

inline void test_findClass(const char* fqcn)
{
    static auto toString = uc::jni::make_method<jobject, std::string()>("toString");

    // If you call "uc::jni::replace_with_class_loader_find_class<UcJniTest>()" in JNI_OnLoad(), the following function is safe.
    auto obj0 = uc::jni::find_class(fqcn);
    LOGD << toString(obj0);

    // JNIEnv::findClass() didn't find your class in native thread.
    try {
        auto obj1 = uc::jni::find_class_native(fqcn);
        LOGD << toString(obj1);
    } catch (std::exception& ex) {
        LOGW << __func__ << "() : find_class_native(" << fqcn << ") failed. " << ex.what();
        uc::jni::env()->ExceptionDescribe();
        uc::jni::env()->ExceptionClear();
    }
}
JNI(void, testFindClassInNativeThread)(JNIEnv *env, jobject thiz)
{
    std::async(std::launch::async, [&] {
        test_findClass("java/lang/Object");

        test_findClass("android/graphics/Point");

        test_findClass("com/example/uc/ucjnitest/MainActivity");

        test_findClass("com/example/uc/ucjnitest/UcJniTest");
    });
}


//*************************************************************************************************
// Test global_ref, weak_lef
//*************************************************************************************************

uc::jni::global_ref<jstring> globalString{};
uc::jni::global_ref<jintArray> globalIntArray{};
uc::jni::global_ref<uc::jni::array<jstring>> globalStringArray{};
uc::jni::weak_ref<jstring> weakString{};

JNI(void, testGlobalRef)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {
        TEST_ASSERT(!globalString);
        TEST_ASSERT(!globalIntArray);
        TEST_ASSERT(!globalStringArray);

        globalString = uc::jni::to_jstring("Hello World");
        globalIntArray = uc::jni::to_jarray(std::vector<jint>{11, 22, 33, 44, 55});
        globalStringArray = uc::jni::to_jarray(std::vector<std::string>{"abc", "de", "fghij", "", "A"});
        // globalString = uc::jni::make_global(uc::jni::to_jstring("Hello World"));

        TEST_ASSERT(globalString);
        TEST_ASSERT(globalIntArray);
        TEST_ASSERT(globalStringArray);
    });
}

JNI(void, testWeakRef)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {

        TEST_ASSERT(globalString);
        TEST_ASSERT_EQUALS(std::string("Hello World"), uc::jni::to_string(globalString));
        TEST_ASSERT(globalIntArray);
        {
            auto expected = std::vector<jint>{11, 22, 33, 44, 55};
            TEST_ASSERT_EQUALS(expected, uc::jni::to_vector(globalIntArray));
        }
        TEST_ASSERT(globalStringArray);
        {
            auto expected = std::vector<std::string>{"abc", "de", "fghij", "", "A"};
            TEST_ASSERT_EQUALS(expected, uc::jni::to_vector<std::string>(globalStringArray));
        }
        TEST_ASSERT(globalStringArray);
        {
            auto expected = std::vector<std::string>{"abc", "de", "fghij", "", "A"};
            auto actual = uc::jni::to_vector<uc::jni::local_ref<jstring>>(globalStringArray);
            TEST_ASSERT(std::equal(expected.begin(), expected.end(), actual.begin(), actual.end(), [](auto& a, auto& b) { return a == uc::jni::to_string(b); }));
        }


        auto jstr = uc::jni::to_jstring("Hello World");

        weakString = jstr;//uc::jni::weak_ref<jstring>(jstr);
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

//*************************************************************************************************
// Test uc::jni::is_instance_of
//*************************************************************************************************
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

        jobject j{};
        TEST_ASSERT(uc::jni::is_instance_of<jobject>(j));
        TEST_ASSERT(uc::jni::is_instance_of<UcJniTest>(j));
        TEST_ASSERT(uc::jni::is_instance_of<jstring>(j));

//        auto ucjnitest = uc::jni::as<UcJniTest>(thiz);
//        LOGD << "ucjnitest@" << ucjnitest;
//        auto ucjnitest2 = uc::jni::as<jstring>(thiz);
//        LOGD << "ucjnitest2 @" << ucjnitest2;
        
    });
}

//*************************************************************************************************
// Test uc::jni::to_jstring
//*************************************************************************************************
JNI(void, testToJstring)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {
        {
            auto field = uc::jni::make_static_field<UcJniTest, jstring>("staticFieldString");
            auto str = uc::jni::to_jstring("Hello World!");
            logd(__func__, str);
            TEST_ASSERT(compareJStrings(str, field.get()) == 0);
        }
        {
            auto field = uc::jni::make_static_field<UcJniTest, jstring>("staticFieldStringJp");
            auto str = uc::jni::to_jstring(u"こんにちは、世界！");
            logd(__func__, str);
            TEST_ASSERT(compareJStrings(str, field.get()) == 0);
        }
    });
}
//*************************************************************************************************
// Test uc::jni::to_string
//*************************************************************************************************
JNI(void, testToString)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {
        const std::string value = "Hello World!";
        const std::u16string valueJp = u"こんにちは、世界！";
        {
            jstring jstr{};
            auto str = uc::jni::to_string(jstr);
            TEST_ASSERT_EQUALS(std::string(), str);
        }
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

        {
            const std::string jpstr = u8"日本語で UTF8 文字列を書き、これを jstring に変換してまた戻したときに、きちんと戻るかどうかテストする。その逆もテストする。";
            const std::u16string jpstr16 = u"日本語で UTF8 文字列を書き、これを jstring に変換してまた戻したときに、きちんと戻るかどうかテストする。その逆もテストする。";

            auto jstr1 = uc::jni::to_jstring(jpstr);
            TEST_ASSERT_EQUALS(jpstr,   uc::jni::to_string(jstr1));
            TEST_ASSERT_EQUALS(jpstr16, uc::jni::to_u16string(jstr1));

            auto jstr2 = uc::jni::to_jstring(jpstr16);
            TEST_ASSERT_EQUALS(jpstr,   uc::jni::to_string(jstr2));
            TEST_ASSERT_EQUALS(jpstr16, uc::jni::to_u16string(jstr2));
        }

#if 0
        // Test : JNI ERROR (app bug): local reference table overflow (max=8388608)
        static auto testDoNothing = uc::jni::make_method<UcJniTest, void(std::string)>("testDoNothing");
        for (int i = 0; i < 9000000; ++i) {
            std::string str = "test";
            testDoNothing(thiz, str);
        }
#endif
    });
}


//*************************************************************************************************
// Test uc::jni::internal::cexprstr
//*************************************************************************************************
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr(""), uc::jni::make_cexprstr(""));
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("a"), uc::jni::make_cexprstr("a"));
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("ABCDE"), uc::jni::make_cexprstr("ABCDE"));
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("ABCDE").append("FGHIJ"), uc::jni::make_cexprstr("ABCDE").append("FGHIJ"));
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("ABC").append("DEFGHIJ"), uc::jni::make_cexprstr("ABCDEFG").append("HIJ"));
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("").append("ABCDEFGHIJ"), uc::jni::make_cexprstr("ABCDEFGHIJ").append(""));

STATIC_ASSERT_NOT_EQUALS(uc::jni::make_cexprstr(""), uc::jni::make_cexprstr("a"));
STATIC_ASSERT_NOT_EQUALS(uc::jni::make_cexprstr("a"), uc::jni::make_cexprstr(""));
STATIC_ASSERT_NOT_EQUALS(uc::jni::make_cexprstr("a"), uc::jni::make_cexprstr("b"));
STATIC_ASSERT_NOT_EQUALS(uc::jni::make_cexprstr("abc"), uc::jni::make_cexprstr("abb"));

STATIC_ASSERT_NOT_EQUALS(uc::jni::make_cexprstr("ABCDE"), uc::jni::make_cexprstr("ABCD"));
STATIC_ASSERT_NOT_EQUALS(uc::jni::make_cexprstr("ABCD"), uc::jni::make_cexprstr("ABCDE"));
STATIC_ASSERT_NOT_EQUALS(uc::jni::make_cexprstr("ABCDE").append("FGHI"), uc::jni::make_cexprstr("ABCDE").append("FGHIJ"));

STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("V"), uc::jni::type_traits<void>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("Z"), uc::jni::type_traits<jboolean>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("B"), uc::jni::type_traits<jbyte>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("C"), uc::jni::type_traits<jchar>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("S"), uc::jni::type_traits<jshort>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("I"), uc::jni::type_traits<jint>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("J"), uc::jni::type_traits<jlong>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("D"), uc::jni::type_traits<jdouble>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("F"), uc::jni::type_traits<jfloat>::signature());

STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("Ljava/lang/Object;"), uc::jni::type_traits<jobject>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("Ljava/lang/String;"), uc::jni::type_traits<jstring>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("Lcom/example/uc/ucjnitest/UcJniTest;"), uc::jni::type_traits<UcJniTest>::signature());

STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[Z"), uc::jni::type_traits<jbooleanArray>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[B"), uc::jni::type_traits<jbyteArray>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[C"), uc::jni::type_traits<jcharArray>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[S"), uc::jni::type_traits<jshortArray>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[I"), uc::jni::type_traits<jintArray>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[J"), uc::jni::type_traits<jlongArray>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[D"), uc::jni::type_traits<jdoubleArray>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[F"), uc::jni::type_traits<jfloatArray>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[Ljava/lang/Object;"), uc::jni::type_traits<jobjectArray>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[Ljava/lang/Object;"), uc::jni::type_traits<uc::jni::array<jobject>>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[Ljava/lang/String;"), uc::jni::type_traits<uc::jni::array<jstring>>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[Lcom/example/uc/ucjnitest/UcJniTest;"), uc::jni::type_traits<uc::jni::array<UcJniTest>>::signature());

STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("Z"), uc::jni::type_traits<bool>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("B"), uc::jni::type_traits<char>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("C"), uc::jni::type_traits<char16_t>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("Ljava/lang/String;"), uc::jni::type_traits<std::string>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("Ljava/lang/String;"), uc::jni::type_traits<std::u16string>::signature());

STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[Z"), uc::jni::type_traits<std::vector<jboolean>>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[B"), uc::jni::type_traits<std::vector<jbyte>>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[C"), uc::jni::type_traits<std::vector<jchar>>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[S"), uc::jni::type_traits<std::vector<jshort>>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[I"), uc::jni::type_traits<std::vector<jint>>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[J"), uc::jni::type_traits<std::vector<jlong>>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[D"), uc::jni::type_traits<std::vector<jdouble>>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[F"), uc::jni::type_traits<std::vector<jfloat>>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("[Ljava/lang/String;"), uc::jni::type_traits<std::vector<std::string>>::signature());

STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()V"), uc::jni::type_traits<void()>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()Z"), uc::jni::type_traits<jboolean()>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()B"), uc::jni::type_traits<jbyte()>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()C"), uc::jni::type_traits<jchar()>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()S"), uc::jni::type_traits<jshort()>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()I"), uc::jni::type_traits<jint()>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()J"), uc::jni::type_traits<jlong()>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()D"), uc::jni::type_traits<jdouble()>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()F"), uc::jni::type_traits<jfloat()>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()Ljava/lang/Object;"), uc::jni::type_traits<jobject()>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()Ljava/lang/String;"), uc::jni::type_traits<jstring()>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()Lcom/example/uc/ucjnitest/UcJniTest;"), uc::jni::type_traits<UcJniTest()>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()[Z"), uc::jni::type_traits<jbooleanArray()>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()[B"), uc::jni::type_traits<jbyteArray()>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()[C"), uc::jni::type_traits<jcharArray()>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()[S"), uc::jni::type_traits<jshortArray()>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()[I"), uc::jni::type_traits<jintArray()>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()[J"), uc::jni::type_traits<jlongArray()>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()[D"), uc::jni::type_traits<jdoubleArray()>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()[F"), uc::jni::type_traits<jfloatArray()>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("()[Ljava/lang/Object;"), uc::jni::type_traits<jobjectArray()>::signature());

STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("(ZBCSIJLjava/lang/String;DF)V"), uc::jni::type_traits<void(jboolean,jbyte,jchar,jshort,jint,jlong,jstring,jdouble,jfloat)>::signature());
STATIC_ASSERT_EQUALS(uc::jni::make_cexprstr("([Ljava/lang/Object;[Z[B[C[S[I[J[D[F)Ljava/lang/String;"), uc::jni::type_traits<jstring(jobjectArray,jbooleanArray,jbyteArray,jcharArray,jshortArray,jintArray,jlongArray,jdoubleArray,jfloatArray)>::signature());


//*************************************************************************************************
// Test uc::jni::type_traits<T>::signature()
//*************************************************************************************************
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
        TEST_ASSERT_EQUALS(std::string("[Ljava/lang/Object;"), uc::jni::type_traits<uc::jni::array<jobject>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[Ljava/lang/String;"), uc::jni::type_traits<uc::jni::array<jstring>>::signature().c_str());
        TEST_ASSERT_EQUALS(std::string("[Lcom/example/uc/ucjnitest/UcJniTest;"), uc::jni::type_traits<uc::jni::array<UcJniTest>>::signature().c_str());


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
        TEST_ASSERT_EQUALS(std::string("[Ljava/lang/String;"), uc::jni::type_traits<std::vector<std::string>>::signature().c_str());


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

//*************************************************************************************************
// Test uc::jni::make_static_field()
//*************************************************************************************************
template<typename T> void testStaticField(const char* fieldName, const T& value1, const T& value2)
{
    auto field = uc::jni::make_static_field<UcJniTest, T>(fieldName);
    field.set(value1);
    TEST_ASSERT_EQUALS(value1, field.get());
    TEST_ASSERT_NOT_EQUALS(value2, field.get());

    field.set(value2);
    TEST_ASSERT_EQUALS(value2, field.get());
    TEST_ASSERT_NOT_EQUALS(value1, field.get());
#if 0
    for (int i = 0; i < 1024; ++i) {
        field.set(value1);
        auto v = field.get();
        TEST_ASSERT_EQUALS(value1, v);

        field.set(value2);
        auto v2 = field.get();
        TEST_ASSERT_EQUALS(value2, v2);
    }
#endif
}

//*************************************************************************************************
// Test uc::jni::make_field()
//*************************************************************************************************
template<typename T> void testField(const char* fieldName, jobject thiz, const T& value1, const T& value2)
{
    auto field = uc::jni::make_field<UcJniTest, T>(fieldName);
    field.set(thiz, value1);
    TEST_ASSERT_EQUALS(value1, field.get(thiz));
    TEST_ASSERT_NOT_EQUALS(value2, field.get(thiz));

    field.set(thiz, value2);
    TEST_ASSERT_EQUALS(value2, field.get(thiz));
    TEST_ASSERT_NOT_EQUALS(value1, field.get(thiz));

#if 0
    for (int i = 0; i < 1024; ++i) {
        field.set(thiz, value1);
        auto v = field.get(thiz);
        TEST_ASSERT_EQUALS(value1, v);

        field.set(thiz, value2);
        auto v2 = field.get(thiz);
        TEST_ASSERT_EQUALS(value2, v2);
    }
#endif
}

//*************************************************************************************************
// Test uc::jni::make_static_method()
//*************************************************************************************************
template<typename T> void testStaticMethod(const char* fieldName, 
    const char* setMethodName, const char* getMethodName, const T& value1, const T& value2)
{
    const auto field = uc::jni::make_static_field<UcJniTest, T>(fieldName);
    const auto setter = uc::jni::make_static_method<UcJniTest, void(T)>(setMethodName);
    const auto getter = uc::jni::make_static_method<UcJniTest, T()>(getMethodName);

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

#if 0
    for (int i = 0; i < 1024; ++i) {
        setter(value1);
        auto v = getter();
        TEST_ASSERT_EQUALS(value1, v);

        setter(value2);
        auto v2 = getter();
        TEST_ASSERT_EQUALS(value2, v2);
    }
#endif
}

//*************************************************************************************************
// Test uc::jni::make_method()
//*************************************************************************************************
template<typename T> void testMethod(const char* fieldName,
    const char* setMethodName, const char* getMethodName, 
    jobject thiz, const T& value1, const T& value2)
{
    const auto field = uc::jni::make_field<UcJniTest, T>(fieldName);
    const auto setter = uc::jni::make_method<UcJniTest, void(T)>(setMethodName);
    const auto getter = uc::jni::make_method<UcJniTest, T()>(getMethodName);

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

#if 0
    for (int i = 0; i < 1024; ++i) {
        setter(thiz, value1);
        auto v = getter(thiz);
        TEST_ASSERT_EQUALS(value1, v);

        setter(thiz, value2);
        auto v2 = getter(thiz);
        TEST_ASSERT_EQUALS(value2, v2);
    }
#endif
}

//*************************************************************************************************
// Define Test
//*************************************************************************************************
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
DEFINE_FIELD_AND_METHOD_TEST(char16_t,       Char,     u'さ', u'A')
DEFINE_FIELD_AND_METHOD_TEST(jbyte,          Byte,     0xff, 0xcc)
DEFINE_FIELD_AND_METHOD_TEST(jshort,         Short,    123, 456)
DEFINE_FIELD_AND_METHOD_TEST(jint,           Int,      1234, 5678)
DEFINE_FIELD_AND_METHOD_TEST(jlong,          Long,     1234, 5678)
DEFINE_FIELD_AND_METHOD_TEST(jfloat,         Float,    123.4f, 567.8f)
DEFINE_FIELD_AND_METHOD_TEST(jdouble,        Double,   1.2345, 6.7891)
DEFINE_FIELD_AND_METHOD_TEST(std::string,    String,   "Hello World!", "Hello Java.")
DEFINE_FIELD_AND_METHOD_TEST(std::u16string, StringJp,  u"こんにちは、世界！", u"こんばんわ, Java.")

#define BOOL_ARRAY_TEST_VALUE1 std::vector<bool> {false, true, false, true, false}
#define BOOL_ARRAY_TEST_VALUE2 std::vector<bool> {true, false, true, false, true, false, true, false}
DEFINE_FIELD_AND_METHOD_TEST(std::vector<bool>, BoolArray,  BOOL_ARRAY_TEST_VALUE1, BOOL_ARRAY_TEST_VALUE2)

#define INT_ARRAY_TEST_VALUE1 std::vector<int> {11, 21, 31, 41, 51}
#define INT_ARRAY_TEST_VALUE2 std::vector<int> {111, 222, 333, 444, 555, 666, 777}
DEFINE_FIELD_AND_METHOD_TEST(std::vector<int>, IntArray,  INT_ARRAY_TEST_VALUE1, INT_ARRAY_TEST_VALUE2)

#define CHAR_ARRAY_TEST_VALUE1 std::vector<jchar> {u'あ', u'い', u'う', u'え', u'お'}
#define CHAR_ARRAY_TEST_VALUE2 std::vector<jchar> {u'a', u'b', u'c', u'd', u'e', u'f', u'g'}
DEFINE_FIELD_AND_METHOD_TEST(std::vector<jchar>, CharArray,  CHAR_ARRAY_TEST_VALUE1, CHAR_ARRAY_TEST_VALUE2)

#define STRING_ARRAY_TEST_VALUE1 std::vector<std::string> { "abc", "defg", "hijk", "lmnopqr" }
#define STRING_ARRAY_TEST_VALUE2 std::vector<std::string> { u8"Hello", u8"World!", u8"こんにちは", u8"世界" }
DEFINE_FIELD_AND_METHOD_TEST(std::vector<std::string>, StringArray,  STRING_ARRAY_TEST_VALUE1, STRING_ARRAY_TEST_VALUE2)

#define STRING16_ARRAY_TEST_VALUE1 std::vector<std::u16string> { u"世界", u"こんにちは", u"World!", u"Hello" }
#define STRING16_ARRAY_TEST_VALUE2 std::vector<std::u16string> { u"Hello", u"World!", u"こんにちは", u"世界" }
DEFINE_FIELD_AND_METHOD_TEST(std::vector<std::u16string>, String16Array,  STRING16_ARRAY_TEST_VALUE1, STRING16_ARRAY_TEST_VALUE2)


//*************************************************************************************************
// Test jarray <-> std::vector<T>
//*************************************************************************************************
template<typename T, typename JArray> void testArrayTransform(const char* fieldName, jobject thiz, const std::vector<T>& value)
{
    auto field = uc::jni::make_field<UcJniTest, JArray>(fieldName);

    field.set(thiz, uc::jni::to_jarray(value));

    auto array = field.get(thiz);
    TEST_ASSERT_EQUALS(value.size(), uc::jni::length(array));

    auto retValue = uc::jni::to_vector<T>(array);
    TEST_ASSERT_EQUALS(value, retValue);
}
#define DEFINE_ARRAY_TRANSFORM_TEST(type, arrType, methodType, value1, value2) \
JNI(void, testArrayTransform ## methodType)(JNIEnv *env, jobject thiz)\
{\
    uc::jni::exception_guard([&] { testArrayTransform<type, arrType>( "field" #methodType, thiz, value1); testArrayTransform<type, arrType>( "field" #methodType, thiz, value2); });\
}
DEFINE_ARRAY_TRANSFORM_TEST(bool,           jbooleanArray,           BoolArray,      BOOL_ARRAY_TEST_VALUE1, BOOL_ARRAY_TEST_VALUE2)
DEFINE_ARRAY_TRANSFORM_TEST(int,            jintArray,               IntArray,       INT_ARRAY_TEST_VALUE1, INT_ARRAY_TEST_VALUE2)
DEFINE_ARRAY_TRANSFORM_TEST(jchar,          jcharArray,              CharArray,      CHAR_ARRAY_TEST_VALUE1, CHAR_ARRAY_TEST_VALUE2)
DEFINE_ARRAY_TRANSFORM_TEST(std::string,    uc::jni::array<jstring>, StringArray,    STRING_ARRAY_TEST_VALUE1, STRING_ARRAY_TEST_VALUE2)
DEFINE_ARRAY_TRANSFORM_TEST(std::u16string, uc::jni::array<jstring>, String16Array,  STRING16_ARRAY_TEST_VALUE1, STRING16_ARRAY_TEST_VALUE2)


//*************************************************************************************************
// Test GetArrayElements() / SetArrayElements()
//*************************************************************************************************
JNI(void, testPrimitiveElements)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {
        const std::vector<int> values1 = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
        const std::vector<int> values2 = {20, 21, 22, 23, 24, 25, 26, 27, 28, 29};

        auto array = uc::jni::new_array<jint>(10);
        TEST_ASSERT_EQUALS(10, env->GetArrayLength(array.get()));
        TEST_ASSERT_EQUALS(10, uc::jni::length(array));

        {
            uc::jni::set_region(array, 0, 10, values2.data());

            std::vector<int> values3(10);
            uc::jni::get_region(array, 0, 10, values3.data());
            TEST_ASSERT_EQUALS(values2, values3);
        }
        {
            auto values3 = uc::jni::to_vector(array);
            TEST_ASSERT_EQUALS(values2, values3);
        }

        {
            auto elems = uc::jni::get_elements(array.get());
            std::copy(values1.begin(), values1.end(), uc::jni::begin(elems));

            TEST_ASSERT(!std::equal(values1.begin(), values1.end(), uc::jni::begin(uc::jni::get_elements(array))));
        }
        TEST_ASSERT(std::equal(values1.begin(), values1.end(), uc::jni::begin(uc::jni::get_const_elements(array.get()))));

        {
            auto elems = uc::jni::get_elements(array);
            std::copy(values2.begin(), values2.end(), uc::jni::begin(elems));
            uc::jni::set_abort(elems);
        }
        TEST_ASSERT(std::equal(values1.begin(), values1.end(), uc::jni::begin(uc::jni::get_const_elements(array))));

        {
            auto elems = uc::jni::get_elements(array);
            std::copy(values2.begin(), values2.end(), uc::jni::begin(elems));

            TEST_ASSERT(std::equal(values1.begin(), values1.end(), uc::jni::begin(uc::jni::get_const_elements(array))));
            uc::jni::commit(elems);
            TEST_ASSERT(std::equal(values2.begin(), values2.end(), uc::jni::begin(uc::jni::get_const_elements(array))));
        }
    });
}


//*************************************************************************************************
// Test GetObjectArrayElement() / SetObjectArrayElement()
//*************************************************************************************************
JNI(void, testObjectElements)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {

        std::vector<std::string> values1 { "abc", "defg", "hijk", "lmnopqr" };
        std::vector<std::string> values2 { "ABC", "DEFG", "HIJK", "LMNOPQR" };

        auto array = uc::jni::new_array<jstring>(4);
        TEST_ASSERT_EQUALS(4, env->GetArrayLength(array.get()));
        TEST_ASSERT_EQUALS(4, uc::jni::length(array));

        {
            int i = 0;
            for (auto&& v : values1) {
                uc::jni::set(array, i++, uc::jni::to_jstring(v));
            }
            i = 0;
            for (auto&& v : values1) {
                auto jstr = uc::jni::get(array, i++);
                TEST_ASSERT_EQUALS(v, uc::jni::to_string(jstr));
            }
        }

        {
            uc::jni::set_region(array, 0, 4, values2.begin(), [](auto&& str) { return uc::jni::to_jstring(str).release(); });

            std::vector<std::string> values3(4);
            uc::jni::get_region(array, 0, 4, values3.begin(), [](auto&& str) { return uc::jni::to_string(str); });
            
            TEST_ASSERT_EQUALS(values2, values3);
        }
        {
            jobjectArray objArray = array.get();
            auto values3 = uc::jni::to_vector<std::string>(objArray);
            TEST_ASSERT_EQUALS(values2, values3);
        }
    });
}

//*************************************************************************************************
// Test uc::jni::join()
//*************************************************************************************************
JNI(void, testStringBuffer)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {
        {
            auto j = uc::jni::join();
            TEST_ASSERT_EQUALS(0, uc::jni::string_traits<char>::length(env, j.get()));
            TEST_ASSERT_EQUALS(std::string(), uc::jni::to_string(j));
        }
        {
            auto j = uc::jni::join("");
            TEST_ASSERT_EQUALS(0, uc::jni::string_traits<char>::length(env, j.get()));
            TEST_ASSERT_EQUALS(std::string(), uc::jni::to_string(j));
        }
        {
            auto j = uc::jni::join("a");
            TEST_ASSERT_EQUALS(1, uc::jni::string_traits<char>::length(env, j.get()));
            TEST_ASSERT_EQUALS(std::string("a"), uc::jni::to_string(j));
        }
        {
            auto j = uc::jni::join(u"a");
            TEST_ASSERT_EQUALS(1, uc::jni::string_traits<char>::length(env, j.get()));
            TEST_ASSERT_EQUALS(std::string("a"), uc::jni::to_string(j));
        }
        {
            auto j = uc::jni::join(std::string());
            TEST_ASSERT_EQUALS(0, uc::jni::string_traits<char>::length(env, j.get()));
            TEST_ASSERT_EQUALS(std::string(), uc::jni::to_string(j));
        }
        {
            auto j = uc::jni::join(std::string("xyz"));
            TEST_ASSERT_EQUALS(3, uc::jni::string_traits<char>::length(env, j.get()));
            TEST_ASSERT_EQUALS(std::string("xyz"), uc::jni::to_string(j));
        }
        {
            auto j = uc::jni::join(std::u16string());
            TEST_ASSERT_EQUALS(0, uc::jni::string_traits<char>::length(env, j.get()));
            TEST_ASSERT_EQUALS(std::string(), uc::jni::to_string(j));
        }
        {
            auto j = uc::jni::join(std::u16string(u"123"));
            TEST_ASSERT_EQUALS(3, uc::jni::string_traits<char>::length(env, j.get()));
            TEST_ASSERT_EQUALS(std::string("123"), uc::jni::to_string(j));
        }
        {
            jstring jstr{};
            auto j = uc::jni::join(jstr);
            TEST_ASSERT_EQUALS(0, uc::jni::string_traits<char>::length(env, j.get()));
            TEST_ASSERT_EQUALS(std::string(), uc::jni::to_string(j));
        }
        {
            auto j = uc::jni::join(uc::jni::to_jstring("xyz"));
            TEST_ASSERT_EQUALS(3, uc::jni::string_traits<char>::length(env, j.get()));
            TEST_ASSERT_EQUALS(std::string("xyz"), uc::jni::to_string(j));
        }
        {
            auto j = uc::jni::join(uc::jni::make_global(uc::jni::to_jstring("xyz")));
            TEST_ASSERT_EQUALS(3, uc::jni::string_traits<char>::length(env, j.get()));
            TEST_ASSERT_EQUALS(std::string("xyz"), uc::jni::to_string(j));
        }
        auto j0 = uc::jni::join("abc", "def", "ghi");
        TEST_ASSERT_EQUALS(9, uc::jni::string_traits<char>::length(env, j0.get()));
        TEST_ASSERT_EQUALS(std::string("abcdefghi"), uc::jni::to_string(j0));
        TEST_ASSERT_EQUALS(std::u16string(u"abcdefghi"), uc::jni::to_u16string(j0));

        auto j1 = uc::jni::join(std::string("abc"), "def", "ghi");
        TEST_ASSERT_EQUALS(9, uc::jni::string_traits<char>::length(env, j1.get()));
        TEST_ASSERT_EQUALS(std::string("abcdefghi"), uc::jni::to_string(j1));
        TEST_ASSERT_EQUALS(std::u16string(u"abcdefghi"), uc::jni::to_u16string(j1));

        auto j2 = uc::jni::join(j0, ", ", j1);
        TEST_ASSERT_EQUALS(20, uc::jni::string_traits<char>::length(env, j2.get()));
        TEST_ASSERT_EQUALS(std::string("abcdefghi, abcdefghi"), uc::jni::to_string(j2));

        auto j3 = uc::jni::join("123, ", j0, u", ABCD, ", std::string("!#$%"), std::u16string(u", qwerty"));
        TEST_ASSERT_EQUALS(34, uc::jni::string_traits<char>::length(env, j3.get()));
        TEST_ASSERT_EQUALS(std::string("123, abcdefghi, ABCD, !#$%, qwerty"), uc::jni::to_string(j3));

    });
}

//*************************************************************************************************
// Test uc::jni::register_natives()
//*************************************************************************************************
jboolean returnTrue(JNIEnv* env, jobject obj)
{
    return JNI_TRUE;
}
jstring returnString(JNIEnv* env, jobject obj, jstring str)
{
    // return uc::jni::to_jstring("[" + uc::jni::to_string(str) + "] received.").release();
    return uc::jni::join("[", str, "] received.").release();
}
jint plus(JNIEnv* env, jobject obj, jint i, jint j)
{
    return i + j;
}
JNI(void, testRegisterNatives)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {
        {
            auto method = uc::jni::make_native_method("returnTrueMethod", &returnTrue);
            TEST_ASSERT_EQUALS(std::string("()Z"), method.signature);
            TEST_ASSERT(uc::jni::register_natives<UcJniTest>(&method, 1));
        }
        {
            auto method = uc::jni::make_native_method("returnStringMethod", &returnString);
            TEST_ASSERT_EQUALS(std::string("(Ljava/lang/String;)Ljava/lang/String;"), method.signature);
        }
        {
            auto method = uc::jni::make_native_method("plusMethod", &plus);
            TEST_ASSERT_EQUALS(std::string("(II)I"), method.signature);
        }
        JNINativeMethod m[]{
            uc::jni::make_native_method("returnStringMethod", &returnString),
            uc::jni::make_native_method("plusMethod", &plus)
        };
        TEST_ASSERT(uc::jni::register_natives<UcJniTest>(m));
    });
}

//*************************************************************************************************
// Test Monitor API
//*************************************************************************************************
JNI(void, testMonitor)(JNIEnv *env, jobject thiz, jobject point)
{
    uc::jni::exception_guard([&] {
std::this_thread::sleep_for(std::chrono::seconds(1));
        UC_JNI_DEFINE_JCLASS_ALIAS(Point, android/graphics/Point);
        auto s = uc::jni::synchronized(point);
        auto x = uc::jni::make_field<Point, int>("x");
        x.set(point, x.get(point) + 1);
    });
}
JNI(void, testMonitorAsync)(JNIEnv *env, jobject thiz, jobject p)
{
    uc::jni::exception_guard([&] {
        std::thread t([point = uc::jni::make_global(p)] {
            UC_JNI_DEFINE_JCLASS_ALIAS(Point, android/graphics/Point);
            auto s = uc::jni::synchronized(point);
            auto x = uc::jni::make_field<Point, int>("x");
            x.set(point, x.get(point) + 1);
        });
        t.detach();
    });
}
        
//*************************************************************************************************
// Test Direct byte buffer API
//*************************************************************************************************
JNI(void, testDirectBuffer)(JNIEnv *env, jobject thiz, jobject point)
{
    uc::jni::exception_guard([&] {
        char buf[256] {};
        auto buffer = uc::jni::new_direct_byte_buffer(buf, sizeof(buf));

        LOGD << "  $$$$$$ " << getClassName(uc::jni::get_object_class(buffer));

        auto intBuffer = uc::jni::new_direct_buffer<int32_t>(100);
        LOGD << "  $$$$$$ " << env->GetDirectBufferAddress(intBuffer.get()) << ", " << env->GetDirectBufferCapacity(intBuffer.get());

        TEST_ASSERT_EQUALS(100, uc::jni::length(intBuffer));
        TEST_ASSERT_EQUALS(100 * sizeof(int32_t), env->GetDirectBufferCapacity(intBuffer.get()));
        TEST_ASSERT_EQUALS(env->GetDirectBufferAddress(intBuffer.get()), uc::jni::address(intBuffer));
       
    });
}

//*************************************************************************************************
// Test Custom Traits
//*************************************************************************************************
#include <deque>
#include <map>
UC_JNI_DEFINE_JCLASS_ALIAS(HashMap, java/util/HashMap);
UC_JNI_DEFINE_JCLASS_ALIAS(MapEntry, java/util/Map$Entry);
UC_JNI_DEFINE_JCLASS_ALIAS(Set, java/util/Set);
UC_JNI_DEFINE_JCLASS_ALIAS(Iterator, java/util/Iterator);
UC_JNI_DEFINE_JCLASS_ALIAS(Integer, java/lang/Integer);

namespace uc {
namespace jni {
template <> struct type_traits<std::deque<std::string>>
{
    using jvalue_type = uc::jni::array<jstring>;
    using jarray_type = uc::jni::array<jvalue_type>;
    static constexpr decltype(auto) signature() noexcept
    {
        return type_traits<jvalue_type>::signature();
    }
    static std::deque<std::string> c_cast(jvalue_type v)
    {
        const auto len = length(v);
        std::deque<std::string> ret(len);
        auto array = uc::jni::make_local(v);
        get_region(v, 0, len, ret.begin(), [](auto&& jvalue) { return uc::jni::to_string(jvalue); });
        return ret;
    }
    static jvalue_type j_cast(const std::deque<std::string>& v)
    {
        const auto len = static_cast<jsize>(v.size());
        auto ret = new_array<jstring>(len);
        set_region(ret, 0, len, v.begin(), [](auto&& value) { return to_jstring(value).release(); });
        return ret.release(); 
    }
};

template <> struct type_traits<std::map<std::string, int>>
{
    using jvalue_type = HashMap;
    using jarray_type = uc::jni::array<jvalue_type>;
    static constexpr decltype(auto) signature() noexcept
    {
        return type_traits<jvalue_type>::signature();
    }
    static std::map<std::string, int> c_cast(jvalue_type value)
    {
        static auto entrySet = make_method<HashMap, Set()>("entrySet");
        static auto iterator = make_method<Set, Iterator()>("iterator");
        static auto hasNext = make_method<Iterator, bool()>("hasNext");
        static auto next = make_method<Iterator, jobject()>("next");
        static auto getKey = make_method<MapEntry, jobject()>("getKey");
        static auto getValue = make_method<MapEntry, jobject()>("getValue");
        static auto intValue = make_method<Integer, int()>("intValue");


        std::map<std::string, int> ret;
        auto set = entrySet(value);
        auto itr = iterator(set);
        while (hasNext(itr)) {
            auto e = next(itr);
            // ret.emplace(to_string(static_cast<jstring>(getKey(e).get())), intValue(getValue(e)));
            ret.emplace(to_string(getKey(e)), intValue(getValue(e)));
        }
        return ret;
    }
    static jvalue_type j_cast(const std::map<std::string, int>& value)
    {
        static auto newInteger = make_constructor<Integer(int)>();
        static auto newHashMap = make_constructor<HashMap()>();
        static auto put = make_method<HashMap, jobject(jobject, jobject)>("put");
        
        auto ret = newHashMap();
        for (auto&& v : value) {
            put(ret, to_jstring(v.first), newInteger(v.second));
        }
        return ret.release(); 
    }
};

}
}

JNI(void, testCustomTraits)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {

        auto getFieldStringArray = uc::jni::make_method<UcJniTest, std::deque<std::string>()>("getFieldStringArray");
        auto setFieldStringArray = uc::jni::make_method<UcJniTest, void(std::deque<std::string>)>("setFieldStringArray");

        const std::deque<std::string> answer { "abc", "defg", "hijk", "lmnopqr" };

        setFieldStringArray(thiz, answer);

        auto values = getFieldStringArray(thiz);
        TEST_ASSERT_EQUALS(answer, values);
    });
}

JNI(void, testCustomTraits2)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {

        auto getHashMap = uc::jni::make_method<UcJniTest, std::map<std::string, int>()>("getHashMap");
        auto putHashMap = uc::jni::make_method<UcJniTest, void(std::map<std::string, int>)>("putHashMap");

        auto map = getHashMap(thiz);

        for (auto&& v : map) {
            LOGD << v.first << ", " << v.second;
        }
        map.emplace("international", 13);
        putHashMap(thiz, map);
    });
}