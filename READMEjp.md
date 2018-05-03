# uc-jni
uc::jni は C++14 シングルヘッダで書かれた Java Native Interface (JNI) のラッパーライブラリである。

* [Qiita 記事](https://qiita.com/uctakeoff/items/3160031712dadbacfc97)


# Install

1. [`uc-jni.hpp`](https://github.com/uctakeoff/uc-jni/blob/master/uc-jni.hpp) をプロジェクトにコピーする。
2. C++ソースにインクルードする。


# Setup

`uc::jni::java_vm()` を最初に1度だけ実行する必要がある。


```cpp
jint JNI_OnLoad(JavaVM * vm, void * __unused reserved)
{
    uc::jni::java_vm(vm);
    
    return JNI_VERSION_1_6;
}
```

**もし、ネイティブスレッド ( pthread_create, std::thread, std::async...)上で `uc::jni::find_class()`/`uc::jni::get_class()` を呼び出す場合は `uc::jni::replace_with_class_loader_find_class()` も1度だけ呼び出しておく。**

```cpp
DEFINE_JCLASS_ALIAS(YourOwnClass, com/example/YourOwnClass);

jint JNI_OnLoad(JavaVM * vm, void * __unused reserved)
{
    uc::jni::java_vm(vm);

    // uc::jni::java_vm() の次に1度だけ呼び出す。
    uc::jni::replace_with_class_loader_find_class<YourOwnClass>();

    return JNI_VERSION_1_6;
}
```

詳細は [FindClass in native thread](#findclass-in-native-thread) を参照.


# Example

```cpp
using namespace uc;

extern "C" JNIEXPORT void JNICALL Java_com_example_uc_ucjnitest_UcJniTest_Sample)(JNIEnv *env, jobject thiz)
{
    // この中で発生した C++ 例外は、Java例外に置き換えられる。
    jni::exception_guard([&] {

        // Java android.graphics.Point クラスの C++上の別名として Point 型を定義する。
        // Point型は "jobject" と同じように扱うことができる。
        DEFINE_JCLASS_ALIAS(Point, android/graphics/Point);

        // コンストラクタメソッドを生成する。
        auto newPoint = jni::make_constructor<Point(int,int)>();

        // フィールドへのアクセサを生成する。
        auto x = jni::make_field<Point, int>("x");  // Point.x
        auto y = jni::make_field<Point, int>("y");  // Point.y

        // メソッドを生成する。
        auto toString = jni::make_method<jobject, std::string()>("toString"); // Object.toString()
        auto equals = jni::make_method<jobject, bool(jobject)>("equals");     // Object.equals(Object)
        auto offset = jni::make_method<Point, void(int,int)>("offset");       // Point.offset(int,int)


        // 上で作ったアクセサ類は、余計なメモリを一切消費しない。
        TEST_ASSERT_EQUALS(sizeof(newPoint), sizeof(jmethodID));
        TEST_ASSERT_EQUALS(sizeof(toString), sizeof(jmethodID));
        TEST_ASSERT_EQUALS(sizeof(x), sizeof(jfieldID));


        // decltype(p0) == std::unique_ptr<Point,>. リソースは自動的に解放される
        auto p0 = newPoint(12, 34);             // p0 = new Point(12, 34);
        auto p1 = newPoint(12, 34);             // p1 = new Point(12, 34);
        auto p2 = newPoint(123, 456);           // p2 = new Point(123, 456);

        LOGD << toString(p0);                   // p0.toString()

        // フィールドを参照する
        TEST_ASSERT_EQUALS(x.get(p0), 12);      // p0.x == 12
        TEST_ASSERT_EQUALS(y.get(p0), 34);      // p0.y == 34

        // メソッドを呼び出す
        TEST_ASSERT(equals(p0, p1));            // p0.equals(p1)
        TEST_ASSERT(!equals(p1, p2));           // !p1.equals(p2)

        // フィールドを更新する
        x.set(p1, 123);                         // p1.x = 123;
        y.set(p1, 456);                         // p1.y = 456;

        TEST_ASSERT(!equals(p0, p1));
        TEST_ASSERT(equals(p1, p2));

        offset(p0, 100, 200);                   // p0.offset(100, 200);
        TEST_ASSERT_EQUALS(x.get(p0), 112);
        TEST_ASSERT_EQUALS(y.get(p0), 234);

        LOGD << toString(p0) << ", " << toString(p1) << ", " << toString(p2);
    });
}
```

## Wrapper Class Builder Macros

Ver.1.4 よりラッパークラス作成を容易にするヘルパーマクロを用意した。

JNIリソースは適切にキャッシュされ、JNI呼び出しのオーバーヘッドを最小限に抑えている。

```cpp
// Java android.graphics.Point クラスの ラッパーとして "Point_" クラスを定義する。
// Point型は Point_* の別名として定義される。
UC_JNI_DEFINE_JCLASS(Point, android/graphics/Point)
{
    UC_JNI_DEFINE_JCLASS_CONSTRUCTOR(int, int)             // Point(int, int)
    UC_JNI_DEFINE_JCLASS_FIELD(int, x)                     // Point#x
    UC_JNI_DEFINE_JCLASS_FIELD(int, y)                     // Point#y
    UC_JNI_DEFINE_JCLASS_METHOD(void, offset, int, int)    // void Point#offset(int, int)
};

extern "C" JNIEXPORT void JNICALL Java_com_example_uc_ucjnitest_UcJniTest_Sample)(JNIEnv *env, jobject thiz)
{
    uc::jni::exception_guard([&] {

        auto p0 = Point_::new_(12, 34);       // p0 = new Point(12, 34);
        auto p1 = Point_::new_(12, 34);       // p1 = new Point(12, 34);
        auto p2 = Point_::new_(123, 456);     // p2 = new Point(123, 456);

        // java.lang.Object のメソッドは既に定義されている。
        LOGD << p0->toString();               // p0.toString()

        TEST_ASSERT_EQUALS(p0->x(), 12);      // p0.x == 12
        TEST_ASSERT_EQUALS(p0->y(), 34);      // p0.y == 34

        TEST_ASSERT(p0->equals(p1));          // p0.equals(p1)
        TEST_ASSERT(!p1->equals(p2));         // !p1.equals(p2)

        p1->x(123);                           // p1.x = 123;
        p1->y(456);                           // p1.y = 456;

        TEST_ASSERT(!p0->equals(p1));         // !p0.equals(p1)
        TEST_ASSERT(p1->equals(p2));          // p1.equals(p2)

        p0->offset(100, 200);                 // p0.offset(100, 200);
        TEST_ASSERT_EQUALS(p0->x(), 112);     // p0.x == 112
        TEST_ASSERT_EQUALS(p0->y(), 234);     // p0.y == 234

        LOGD << p0->toString() << ", " << p1->toString() << ", " << p2->toString();
    });
}
```

# Detail

## JNIEnv

`JNIEnv* uc::jni::env()` はどのスレッドからでも JNIEnv* を取得することができる。

[`AttachCurrentThread()`](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/invocation.html#AttachCurrentThread) や  [`DetachCurrentThread()`](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/invocation.html#DetachCurrentThread) は、ライブラリ内で適切に呼び出されるため、呼び出し側で実行する必要ない。

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

`uc::jni::local_ref<T>` は `std::unique_ptr<T,>` の別名として定義されている。

スコープが切れた時点で `DeleteLocalRef()` が自動的に呼び出される。

```cpp
    // local_ref<jclass>
    auto clazz = jni::make_local(env->GetObjectClass(thiz));

    uc::jni::local_ref<jclass> superClazz { env->GetSuperclass(clazz.get()) };

    // 上の式は以下のように書くこともできる。
    // auto superClazz = uc::jni::get_super_class(clazz);
```

### Global References

`global_ref<T>` は `std::shared_ptr<T>` に似せて作られている。

やはり `DeleteGlobalRef()` が自動的に呼び出される。

```cpp
    // global_ref<jclass>
    auto clazz = jni::make_global(env->GetObjectClass(thiz));

    // Assignment is also safe.
    uc::jni::global_ref<jclass> superClazz{};
    superClazz = env->GetSuperclass(clazz.get());

    // It is also possible to substitute local_ref. 
    // Ownership does not shift and a new Global Reference can be created.
    superClazz = uc::jni::get_super_class(clazz);
```

### Weak References

JNIの弱参照を扱う `uc::jni::weak_ref<T>` は、 `std::weak_ptr<T>` のように使うことができる。

```cpp
    uc::jni::weak_ref<jobject> wref = jobj;

    uc::jni::local_ref<jobject> tmp = wref.lock();
    if (tmp) {
        :
        :
    }

```

## Resolve Classes


以下の書き方は問題ない。

```cpp
    auto cls = uc::jni::find_class("java/lang/RuntimeException");
```

が、以下のように書くとライブラリがキャッシュをつくり、高速にアクセスすることができる。
```cpp
    DEFINE_JCLASS_ALIAS(RuntimeException, java/lang/RuntimeException);

    // jclass instance is cached. Must not release it.
    jclass cls = uc::jni::get_class<RuntimeException>();
```

## FindClass in native thread

 [FAQ: Why didn't FindClass find my class?](https://developer.android.com/training/articles/perf-jni.html#faq_FindClass)

> 自分でスレッドを作成する (おそらくpthread_createを呼び出してAttachCurrentThreadをアタッチしたもの)と、問題になります。 今度は、アプリケーションからのスタックフレームはありません。 このスレッドからFindClassを呼び出すと、JavaVMはアプリケーションに関連付けられたクラスローダーではなく「システム」クラスローダーで開始されるため、アプリケーション固有のクラスの検索は失敗します。

```cpp
    // このままでは java.lang.ClassNotFoundException 例外が発生する
    std::async(std::launch::async, [] {
        auto cls = uc::jni::find_class("com/example/YourOwnClass");
    });
```

> これを回避するにはいくつかの方法があります：

>・ Do your FindClass lookups once, in JNI_OnLoad, and cache the class references for later use. (FindClassルックアップをJNI_OnLoadで一度行い、後で使用できるようにクラス参照をキャッシュします。)

これは、`uc::jni::get_class()` によって行うことができる。

```cpp
DEFINE_JCLASS_ALIAS(YourOwnClass1, com/example/YourOwnClass1);
DEFINE_JCLASS_ALIAS(YourOwnClass2, com/example/YourOwnClass2);
DEFINE_JCLASS_ALIAS(YourOwnClass3, com/example/YourOwnClass3);
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
上記のように `uc::jni::get_class()` を1度呼び出せばキャッシュを作ることができるため、これ以降は `uc::jni::get_class()` をネイティブスレッドからも実行できるようになる。
ただし、ネイティブスレッド上で呼び出される可能性のあるすべての「アプリケーション固有のクラス」について実行しなければならない。

> ・ Cache a reference to the ClassLoader object somewhere handy, and issue loadClass calls directly. (どこか便利なところに ClassLoaderオブジェクトへの参照をキャッシュし、loadClass 呼び出しを直接発行します。)

このテクニックを実現するために、 `uc::jni::replace_with_class_loader_find_class()` を用意した。

```cpp
DEFINE_JCLASS_ALIAS(YourOwnClass1, com/example/YourOwnClass1);
DEFINE_JCLASS_ALIAS(YourOwnClass2, com/example/YourOwnClass2);
DEFINE_JCLASS_ALIAS(YourOwnClass3, com/example/YourOwnClass3);

jint JNI_OnLoad(JavaVM * vm, void * __unused reserved)
{
    uc::jni::java_vm(vm);

    // テンプレート引数にはアプリケーション固有のクラスを指定する。
    uc::jni::replace_with_class_loader_find_class<YourOwnClass1>();

    return JNI_VERSION_1_6;
}
```

 `uc::jni::find_class()` を呼び出す前に

`uc::jni::replace_with_class_loader_find_class()` は、 `uc::jni::find_class()` の実装を `FindClass()` から `java.lang.ClassLoader#findClass()` に置き換える。
この場合は、アプリケーション固有のクラスをどれか1つだけ、テンプレート引数に指定して実行すればよい。

なお、この処理に関しては、[ここ](https://stackoverflow.com/questions/13263340/findclass-from-any-thread-in-android-jni) での議論を参考にしている。

## String Operations

`jstring` と `std::basic_string` は相互に変換することができる。

```cpp

    // decltype(jstr) == uc::jni::local_ref<jstring>
    auto jstr = uc::jni::to_jstring("Hello World!");

    std::string str = uc::jni::to_string(jstr);
```

C++11 UTF-16 string もサポートする。

```cpp
    auto jstr = uc::jni::to_jstring(u"こんにちは、世界！"); // japanese

    std::u16string str = uc::jni::to_u16string(jstr);
```

`jstring` を作成するには `uc::jni::join` がより便利だ。

```cpp
jstring returnString(JNIEnv* env, jobject obj, jstring str)
{
    // Arguments can include jstring, std::string, std::u16string, and so on.

    return uc::jni::join("[", str, "] received.").release();
}
```



## Method ID, Field ID

面倒な **[タイプシグネチャ](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/types.html#type_signatures)** からは開放される。

```cpp
    DEFINE_JCLASS_ALIAS(Point, android/graphics/Point);

    jfieldID fieldId = uc::jni::get_field_id<Point, int>("x");


    DEFINE_JCLASS_ALIAS(System, java/lang/System);

    jmethodID methodId = uc::jni::get_static_method_id<System, void()>("gc");
```

上記より便利で型安全な、以下の **function object** の利用を推奨する。

```cpp
DEFINE_JCLASS_ALIAS(System, java/lang/System);
DEFINE_JCLASS_ALIAS(Point, android/graphics/Point);

void func(Point point)
{
    auto x = uc::jni::make_field<Point, int>("x");  // Point.x

    // valueX = point.x;
    int valueX = x.get(point);

    // point.x = 100;
    x.set(point, 100);


    auto gc = uc::jni::make_static_method<System, void()>("gc");

    // System.gc();
    gc();
}
```

上の `x` や `gc` は、実質 `jfieldID` , `jmethodID` と同じなため、同じように値をコピーして使ってよい。


その他の例

```cpp
    // boolean Object.equals(Object)
    auto equals = jni::make_method<jobject, bool(jobject)>("equals");

    // String Object.toString()
    auto toString = jni::make_method<jobject, jstring()>("toString"); 
    auto toString2 = jni::make_method<jobject, std::string()>("toString"); // 戻り値を std::string にすることもできる。


    DEFINE_JCLASS_ALIAS(UcJniTest, com/example/uc/ucjnitest/UcJniTest);

    // String[] UcJniTest.getFieldStringArray()
    auto getFieldStringArray = uc::jni::make_method<UcJniTest, uc::jni::array<jstring>()>("getFieldStringArray");
    auto getFieldStringArray2 = uc::jni::make_method<UcJniTest, std::vector<std::string>()>("getFieldStringArray"); // Convert to std::vector<std::string>.

```

## Array Operations

### 簡単な使い方

`jarray` と `std::vector` の相互変換を提供する。

```cpp
    // jintArray to std::vector<jint>.
    auto intValues = uc::jni::to_vector(iArray);

    // std::vector<jint> to local_ref<jintArray>.
    auto jintValues = uc::jni::to_jarray(intValues);


    // uc::jni::array<jstring> (inherited from jobjectArray) to std::vector<std::string>.
    auto stringValues = uc::jni::to_vector<std::string>(sArray);

    // std::vector<std::string> to local_ref<uc::jni::array<jstring>>.
    auto jstringValues = uc::jni::to_jarray(stringValues);

```

### Primitive Array

```cpp
    // create local_ref<jintArray>.
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

以下は読み取り専用アクセスを提供する。

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


要素の型が不明な `jobjectArray` の代わりに `uc::jni::array<T>` を使用する。

```cpp

    // String[] UcJniTest.getFieldStringArray()
    auto get = uc::jni::make_method<UcJniTest, uc::jni::array<jstring>()>("getFieldStringArray");

    // decltype(array) == local_ref<array<jstring>>
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
