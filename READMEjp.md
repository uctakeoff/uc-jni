# uc-jni
uc-jni は C++14 シングルヘッダで書かれた Java Native Interface (JNI) のラッパーライブラリである。

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
UC_JNI_DEFINE_JCLASS_ALIAS(YourOwnClass, com/example/YourOwnClass);

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


## Wrapper Class Builder Macros

Ver.1.4 よりラッパークラス作成を容易にするヘルパーマクロを用意した。

シンプルな記述で適切にメソッド・フィールドを取得できる上、JNIリソースは適切にキャッシュされ、JNI呼び出しのオーバーヘッドを最小限に抑えられる。

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

    @Override public String toString() {...}         // 省略
    @Override public int hashCode() {...}            // 省略
    @Override public boolean equals(Object o) {...}  // 省略
}
```
上記のような Java コードに対して、C++側で以下のようにマクロを記述する。

```cpp
// Java com/example/uc/ucjnitest/Point クラスの ラッパーとして "jPoint_" クラスを定義する。
// jPoint 型は jPoint_* の別名として定義される。
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

必ずしもすべて書く必要はない。
これにより、`com.example.uc.ucjnitest.Point` クラスの C++ラッパークラス `jPoint` が定義された。
これは、以下のようにして使うことができる。

```cpp
    // この中で発生した C++ 例外は、Java例外に置き換えられる。
    uc::jni::exception_guard([&] {

        // コンストラクタメソッド
        // p0,p1 は Point のスマートポインタとなっていて、スコープを抜けると自動的に解放される。
        auto p0 = jPoint_::new_(3, 4);       // p0 = new Point(12, 34);
        auto p1 = jPoint_::new_(p0);         // p1 = new Point(p0);

        // フィールドの取得
        TEST_ASSERT_EQUALS(p1->x(), 3);       // p1.x == 3
        TEST_ASSERT_EQUALS(p1->y(), 4);       // p1.y == 4

        // メソッド呼び出し
        TEST_ASSERT_EQUALS(p1->norm(), 5);    // p1.norm() == 5

        // java.lang.Object のメソッドは既に定義されている。
        TEST_ASSERT(p0->equals(p1));          // p0.equals(p1)

        // フィールドの更新
        p1->x(30);                           // p1.x = 30;
        p1->y(40);                           // p1.y = 40;
        TEST_ASSERT(!p0->equals(p1));         // !p0.equals(p1)

        // static フィールドの取得
        TEST_ASSERT_EQUALS(jPoint_::ZERO()->x(), 0);
        TEST_ASSERT_EQUALS(jPoint_::ZERO()->y(), 0);

        // static メソッド呼び出し
        auto p2 = jPoint_::add(p0, p1);
        TEST_ASSERT_EQUALS(p2->x(), 60);
        TEST_ASSERT_EQUALS(p2->y(), 80);

        // Object.toString() は std::string を返す。
        LOGD << p0->toString() << ", " << p1->toString() << ", " << p2->toString();
    });
```

# Detail

上記サンプルにあるようなメソッド・フィールドAPIだけでなく、配列操作や文字列操作など、各種のJNI関数をラッピングして提供している。

## JNIEnv

`JNIEnv* uc::jni::env()` はどのスレッドからでも `JNIEnv*` を取得することができる。

[`AttachCurrentThread()`](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/invocation.html#AttachCurrentThread) や  [`DetachCurrentThread()`](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/invocation.html#DetachCurrentThread) は、ライブラリ内で適切に呼び出されるため、呼び出し側で実行する必要はない。

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

`uc::jni::ref<T>` は `std::unique_ptr<T,>` の別名として定義されている。

スコープが切れた時点で `DeleteLocalRef()` が自動的に呼び出される。
uc-jni のラッパー関数は、通常 JNIオブジェクトをこの形で返す。


```cpp
    // clazz, superClazz は自動的に解放される。
    auto clazz = uc::jni::get_object_class(thiz);
    auto superClazz = uc::jni::get_super_class(clazz);
```

また、ヘルパー関数 `uc::jni::make_local()` を使うと、明示的に `NewLocalRef()` を呼び出してローカル参照を作る。


### Global References

`global_ref<T>` は `std::shared_ptr<T>` に似せて作られている。

代入時に `NewGlobalRef()` によってグローバル参照が作られ、参照カウントが0になった時点で `DeleteGlobalRef()` が呼び出される。

ヘルパー関数として `uc::jni::make_global()` が用意されている。


```cpp
    // global_ref<jclass>
    auto clazz = uc::jni::make_global(uc::jni::get_object_class(thiz));

    // ref や jobject を直接代入できる.
    // 所有権の移動はなく、新しいグローバル参照を作る。
    uc::jni::global_ref<jclass> superClazz = uc::jni::get_super_class(clazz);
    superClazz =  env->GetSuperclass(clazz.get());
```

### Weak References

JNIの弱参照を扱う `uc::jni::weak_ref<T>` は、 `std::weak_ptr<T>` のように使うことができる。

```cpp
    uc::jni::weak_ref<jobject> wref = jobj;

    uc::jni::ref<jobject> tmp = wref.lock();
    if (tmp) {
        :
        :
    }

```

## Resolve Classes

`FindClass()` のラッパー関数もあるが、uc-jni を利用するなら通常はこれを使うべきではない。

```cpp
    // おすすめしない
    auto cls = uc::jni::find_class("java/lang/RuntimeException");
```

`uc::jni::get_class()` は、非常に効率の良いキャッシュ機能を提供する。

最小のコストで、非常に高速なアクセスを提供する。

```cpp
    // 事前にマクロでラッパークラス定義をしておく必要がある。
    UC_JNI_DEFINE_JCLASS_ALIAS(RuntimeException, java/lang/RuntimeException);

    // これは特別に生の jclass を返す。キャッシュなので解放してはならない。
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
上記のように `uc::jni::get_class()` を1度呼び出せばキャッシュを作ることができるため、これ以降は `uc::jni::get_class()` をネイティブスレッドからも実行できるようになる。
ただし、ネイティブスレッド上で呼び出される可能性のあるすべての「アプリケーション固有のクラス」について実行しなければならない。

> ・ Cache a reference to the ClassLoader object somewhere handy, and issue loadClass calls directly. (どこか便利なところに ClassLoaderオブジェクトへの参照をキャッシュし、loadClass 呼び出しを直接発行します。)

このテクニックを実現するために、 `uc::jni::replace_with_class_loader_find_class()` を用意した。

```cpp
UC_JNI_DEFINE_JCLASS_ALIAS(YourOwnClass1, com/example/YourOwnClass1);
UC_JNI_DEFINE_JCLASS_ALIAS(YourOwnClass2, com/example/YourOwnClass2);
UC_JNI_DEFINE_JCLASS_ALIAS(YourOwnClass3, com/example/YourOwnClass3);

jint JNI_OnLoad(JavaVM * vm, void * __unused reserved)
{
    uc::jni::java_vm(vm);

    // テンプレート引数にはアプリケーション固有のクラスを指定する。
    uc::jni::replace_with_class_loader_find_class<YourOwnClass1>();

    return JNI_VERSION_1_6;
}
```

`uc::jni::replace_with_class_loader_find_class()` は、 `uc::jni::find_class()` の実装を `FindClass()` から `java.lang.ClassLoader#findClass()` に置き換える。
この場合は、アプリケーション固有のクラスをどれか1つだけ、テンプレート引数に指定して実行すればよい。

なお、この処理に関しては、[ここ](https://stackoverflow.com/questions/13263340/findclass-from-any-thread-in-android-jni) での議論を参考にしている。

## String Operations

`jstring` と `std::basic_string` は相互に変換することができる。

```cpp

    // decltype(jstr) == uc::jni::ref<jstring>
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
    UC_JNI_DEFINE_JCLASS_ALIAS(Point, android/graphics/Point);

    jfieldID fieldId = uc::jni::get_field_id<Point, int>("x");


    UC_JNI_DEFINE_JCLASS_ALIAS(System, java/lang/System);

    jmethodID methodId = uc::jni::get_static_method_id<System, void()>("gc");
```

上記より便利で型安全な、以下の **function object** の利用を推奨する。

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

上の `x` や `gc` は、実質 `jfieldID` , `jmethodID` と同じなため、同じように値をコピーして使ってよい。


その他の例

```cpp
    // boolean Object.equals(Object)
    auto equals = jni::make_method<jobject, bool(jobject)>("equals");

    // String Object.toString()
    auto toString = jni::make_method<jobject, jstring()>("toString"); 
    auto toString2 = jni::make_method<jobject, std::string()>("toString"); // 戻り値を std::string にすることもできる。


    UC_JNI_DEFINE_JCLASS_ALIAS(UcJniTest, com/example/uc/ucjnitest/UcJniTest);

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
