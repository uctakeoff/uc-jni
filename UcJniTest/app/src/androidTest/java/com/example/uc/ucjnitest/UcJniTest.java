package com.example.uc.ucjnitest;

import android.graphics.Point;
import android.util.Log;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.util.HashMap;
import java.util.Map;

import static org.junit.Assert.*;
/**
 * Created by ushi on 2018/01/07.
 */
public class UcJniTest {
    static {
        System.loadLibrary("native-lib");
    }
    class InnerA
    {
        public String getString()
        {
            return "InnerA";
        }
        int value;
    }
    class InnerB extends InnerA
    {
        public String getString()
        {
            return "InnerB";
        }
    }


    public static boolean staticFieldBool   = true;
    public static byte    staticFieldByte   = 10;
    public static short   staticFieldShort  = 20;
    public static int     staticFieldInt    = 30;
    public static long    staticFieldLong   = 40;
    public static float   staticFieldFloat  = 50;
    public static double  staticFieldDouble = 60;
    public static String  staticFieldString = "Hello World!";
    public static String  staticFieldStringJp = "こんにちは、世界！";
    public static int     staticFieldIntArray[] = {10, 20, 30, 40, 50};

    public boolean fieldBool   = true;
    public byte    fieldByte   = 1;
    public short   fieldShort  = 2;
    public int     fieldInt    = 3;
    public long    fieldLong   = 4;
    public float   fieldFloat  = 5;
    public double  fieldDouble = 6;
    public String  fieldString = "Hello World!";
    public String  fieldStringJp = "こんにちは、世界！";
    public int     fieldIntArray[] = {1, 2, 3, 4, 5};



    public static void setStaticFieldBool    (boolean value) { staticFieldBool = value; }
    public static void setStaticFieldByte    (byte    value) { staticFieldByte = value; }
    public static void setStaticFieldShort   (short   value) { staticFieldShort = value; }
    public static void setStaticFieldInt     (int     value) { staticFieldInt = value; }
    public static void setStaticFieldLong    (long    value) { staticFieldLong = value; }
    public static void setStaticFieldFloat   (float   value) { staticFieldFloat = value; }
    public static void setStaticFieldDouble  (double  value) { staticFieldDouble = value; }
    public static void setStaticFieldString  (String  value) { staticFieldString = value; }
    public static void setStaticFieldStringJp(String  value) { staticFieldStringJp = value; }
    public static void setStaticFieldIntArray(int[]   value) { staticFieldIntArray = value; }

    public static boolean getStaticFieldBool()     { return staticFieldBool; }
    public static byte    getStaticFieldByte()     { return staticFieldByte; }
    public static short   getStaticFieldShort()    { return staticFieldShort; }
    public static int     getStaticFieldInt()      { return staticFieldInt; }
    public static long    getStaticFieldLong()     { return staticFieldLong; }
    public static float   getStaticFieldFloat()    { return staticFieldFloat; }
    public static double  getStaticFieldDouble()   { return staticFieldDouble; }
    public static String  getStaticFieldString()   { return staticFieldString; }
    public static String  getStaticFieldStringJp() { return staticFieldStringJp; }
    public static int[]   getStaticFieldIntArray() { return staticFieldIntArray; }

    public void setFieldBool    (boolean value) { fieldBool = value; }
    public void setFieldByte    (byte    value) { fieldByte = value; }
    public void setFieldShort   (short   value) { fieldShort = value; }
    public void setFieldInt     (int     value) { fieldInt = value; }
    public void setFieldLong    (long    value) { fieldLong = value; }
    public void setFieldFloat   (float   value) { fieldFloat = value; }
    public void setFieldDouble  (double  value) { fieldDouble = value; }
    public void setFieldString  (String  value) { fieldString = value; }
    public void setFieldStringJp(String  value) { fieldStringJp = value; }
    public void setFieldIntArray(int[]   value) { fieldIntArray = value; }

    public boolean getFieldBool()     { return fieldBool; }
    public byte    getFieldByte()     { return fieldByte; }
    public short   getFieldShort()    { return fieldShort; }
    public int     getFieldInt()      { return fieldInt; }
    public long    getFieldLong()     { return fieldLong; }
    public float   getFieldFloat()    { return fieldFloat; }
    public double  getFieldDouble()   { return fieldDouble; }
    public String  getFieldString()   { return fieldString; }
    public String  getFieldStringJp() { return fieldStringJp; }
    public int[]   getFieldIntArray() { return fieldIntArray; }



    @Test public native void testToJstring() throws Exception;
    @Test public native void testToString() throws Exception;
    @Test public native void testIsInstanceOf() throws Exception;

    @Test public native void testTypeTraitsSignature() throws Exception;

    @Test public native void testStaticFieldBool() throws Exception;
    @Test public native void testStaticFieldByte() throws Exception;
    @Test public native void testStaticFieldShort() throws Exception;
    @Test public native void testStaticFieldInt() throws Exception;
    @Test public native void testStaticFieldLong() throws Exception;
    @Test public native void testStaticFieldFloat() throws Exception;
    @Test public native void testStaticFieldDouble() throws Exception;
    @Test public native void testStaticFieldString() throws Exception;
    @Test public native void testStaticFieldStringJp() throws Exception;
    @Test public native void testStaticFieldIntArray() throws Exception;

    @Test public native void testFieldBool() throws Exception;
    @Test public native void testFieldByte() throws Exception;
    @Test public native void testFieldShort() throws Exception;
    @Test public native void testFieldInt() throws Exception;
    @Test public native void testFieldLong() throws Exception;
    @Test public native void testFieldFloat() throws Exception;
    @Test public native void testFieldDouble() throws Exception;
    @Test public native void testFieldString() throws Exception;
    @Test public native void testFieldStringJp() throws Exception;
    @Test public native void testFieldIntArray() throws Exception;

    @Test public native void testStaticMethodBool() throws Exception;
    @Test public native void testStaticMethodByte() throws Exception;
    @Test public native void testStaticMethodShort() throws Exception;
    @Test public native void testStaticMethodInt() throws Exception;
    @Test public native void testStaticMethodLong() throws Exception;
    @Test public native void testStaticMethodFloat() throws Exception;
    @Test public native void testStaticMethodDouble() throws Exception;
    @Test public native void testStaticMethodString() throws Exception;
    @Test public native void testStaticMethodStringJp() throws Exception;
    @Test public native void testStaticMethodIntArray() throws Exception;

    @Test public native void testMethodBool() throws Exception;
    @Test public native void testMethodByte() throws Exception;
    @Test public native void testMethodShort() throws Exception;
    @Test public native void testMethodInt() throws Exception;
    @Test public native void testMethodLong() throws Exception;
    @Test public native void testMethodFloat() throws Exception;
    @Test public native void testMethodDouble() throws Exception;
    @Test public native void testMethodString() throws Exception;
    @Test public native void testMethodStringJp() throws Exception;
    @Test public native void testMethodIntArray() throws Exception;

    public static char staticFieldChar = 'a';
    public static void setStaticFieldChar(char value) { staticFieldChar = value; }
    public static char getStaticFieldChar() { return staticFieldChar; }
    public char fieldChar = 'あ';
    public void setFieldChar(char value) { fieldChar = value; }
    public char getFieldChar() { return fieldChar; }
    @Test public native void testStaticFieldChar() throws Exception;
    @Test public native void testFieldChar() throws Exception;
    @Test public native void testStaticMethodChar() throws Exception;
    @Test public native void testMethodChar() throws Exception;

    public static char staticFieldCharArray[] = {'あ', 'い', 'う', 'え', 'お'};
    public static void setStaticFieldCharArray(char[] value) { staticFieldCharArray = value; }
    public static char[] getStaticFieldCharArray() { return staticFieldCharArray; }
    public char fieldCharArray[] = {'か', 'き', 'く', 'け', 'こ'};
    public void setFieldCharArray(char[] value) { fieldCharArray = value; }
    public char[] getFieldCharArray() { return fieldCharArray; }
    // @Test public native void testStaticFieldCharArray() throws Exception;
    // @Test public native void testFieldCharArray() throws Exception;
    // @Test public native void testStaticMethodCharArray() throws Exception;
    // @Test public native void testMethodCharArray() throws Exception;

    public static boolean staticFieldBoolArray[] = {false, true, false, true, false};
    public static void setStaticFieldBoolArray(boolean[] value) { staticFieldBoolArray = value; }
    public static boolean[] getStaticFieldBoolArray() { return staticFieldBoolArray; }
    public boolean fieldBoolArray[] = {true, false, true, false};
    public void setFieldBoolArray(boolean[] value) { fieldBoolArray = value; }
    public boolean[] getFieldBoolArray() { return fieldBoolArray; }
    @Test public native void testStaticFieldBoolArray() throws Exception;
    @Test public native void testFieldBoolArray() throws Exception;
    @Test public native void testStaticMethodBoolArray() throws Exception;
    @Test public native void testMethodBoolArray() throws Exception;

    public static String staticFieldStringArray[] = { "abc", "defg", "hijk", "lmnopqr" };
    public static void setStaticFieldStringArray(String[] value) { staticFieldStringArray = value; }
    public static String[] getStaticFieldStringArray() { return staticFieldStringArray; }
    public String fieldStringArray[] = { "ABC", "DEFG", "HIJK", "LMNOPQR" };
    public void setFieldStringArray(String[] value) { fieldStringArray = value; }
    public String[] getFieldStringArray() { return fieldStringArray; }
    @Test public native void testStaticFieldStringArray() throws Exception;
    @Test public native void testFieldStringArray() throws Exception;
    @Test public native void testStaticMethodStringArray() throws Exception;
    @Test public native void testMethodStringArray() throws Exception;

    public static String staticFieldString16Array[] = { "こんにちは", "世界", "Hello", "World!" };
    public static void setStaticFieldString16Array(String[] value) { staticFieldString16Array = value; }
    public static String[] getStaticFieldString16Array() { return staticFieldString16Array; }
    public String fieldString16Array[] = { "Hello", "World!", "こんにちは", "世界" };
    public void setFieldString16Array(String[] value) { fieldString16Array = value; }
    public String[] getFieldString16Array() { return fieldString16Array; }
    @Test public native void testStaticFieldString16Array() throws Exception;
    @Test public native void testFieldString16Array() throws Exception;
    @Test public native void testStaticMethodString16Array() throws Exception;
    @Test public native void testMethodString16Array() throws Exception;


    @Before
    public void setUp() throws Exception {
        staticFieldString = "Hello World!";
        staticFieldStringJp = "こんにちは、世界！";

        Point p = new Point(10, 10);
    }

    @After
    public void tearDown() throws Exception {
    }

    @Test public native void samplePoint() throws Exception;


    @Test public void testCppException() throws Exception
    {
        try {
            throwRuntimeError();
            fail();
        } catch (final RuntimeException e) {
            assertEquals("std::runtime_error", e.getMessage());
        }
        try {
            throwBadAlloc();
            fail();
        } catch (final OutOfMemoryError e) {
            Log.d("UcJniTest", e.getMessage());
        }
        try {
            throwInt();
            fail();
        } catch (final Error e) {
            Log.d("UcJniTest", e.getMessage());
        }
//        String s = new String("aiueo");
//        Object o = s;
//        UcJniTest l = (UcJniTest)o;
    }
    public native void throwRuntimeError() throws Exception;
    public native void throwBadAlloc() throws Exception;
    public native void throwInt() throws Exception;

    @Test public native void testResolveClass() throws Exception;
    @Test public native void testFindClassInNativeThread() throws Exception;

    @Test public void testRef() throws Exception
    {
        testGlobalRef();
        testWeakRef();
    }
    public native void testGlobalRef() throws Exception;
    public native void testWeakRef() throws Exception;


    @Test public native void testArrayTransformBoolArray() throws Exception;
    @Test public native void testArrayTransformIntArray() throws Exception;
    @Test public native void testArrayTransformCharArray() throws Exception;
    @Test public native void testArrayTransformStringArray() throws Exception;
    @Test public native void testArrayTransformString16Array() throws Exception;

    @Test public native void testPrimitiveElements() throws Exception;
    @Test public native void testObjectElements() throws Exception;
    @Test public native void testStringBuffer() throws Exception;

    @Test public void monitor() throws Exception
    {
        Point p = new Point(100, 200);
        synchronized (p) {
            testMonitor(p);
            assertEquals(101, p.x);
            Log.d("UcJniTest", " Point.x=" + p.x);
            testMonitorAsync(p);
            Log.d("UcJniTest", " Point.x=" + p.x);
            assertEquals(101, p.x);
            Log.d("UcJniTest", " Point.x=" + p.x);
            Thread.sleep(100);
            assertEquals(101, p.x);
            Log.d("UcJniTest", " Point.x=" + p.x);
        }
        Log.d("UcJniTest", " Point.x=" + p.x);
        Thread.sleep(100);
        synchronized (p) {
            assertEquals(102, p.x);
        }
    }
    public native void testMonitor(Point p) throws Exception;
    public native void testMonitorAsync(Point p) throws Exception;

    public native boolean returnTrueMethod();
    public native String returnStringMethod(String str);
    public native int plusMethod(int a, int b);

    @Test public void testRegisteredMethod() throws Exception
    {
        testRegisterNatives();
        assertEquals(true, returnTrueMethod());
        assertEquals("[Message from Java] received.", returnStringMethod("Message from Java"));
        assertEquals(26, plusMethod(7, 19));
    }
    public native void testRegisterNatives() throws Exception;

    @Test public native void testDirectBuffer() throws Exception;

    @Test public native void testCustomTraits() throws Exception;
    @Test public native void testCustomTraits2() throws Exception;

    HashMap getHashMap()
    {
        HashMap<String, Integer> v = new HashMap<>();
        v.put("big", 3);
        v.put("long", 4);
        v.put("good", 5);
        return v;
    }
    void putHashMap(HashMap<String, Integer> hashMap)
    {
        for (Map.Entry<String, Integer> v : hashMap.entrySet()) {
            Log.d("UcJniTest", v.getKey() + " : " + v.getValue());
        }
    }

    void testDoNothing(String string) {
        // do nothing.
    }
}