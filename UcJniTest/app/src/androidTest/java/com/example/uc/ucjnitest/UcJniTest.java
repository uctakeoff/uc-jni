package com.example.uc.ucjnitest;

import android.graphics.Point;
import android.util.Log;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.*;
/**
 * Created by ushi on 2018/01/07.
 */
public class UcJniTest {
    static {
        System.loadLibrary("native-lib");
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

    public boolean fieldBool   = true;
    public byte    fieldByte   = 1;
    public short   fieldShort  = 2;
    public int     fieldInt    = 3;
    public long    fieldLong   = 4;
    public float   fieldFloat  = 5;
    public double  fieldDouble = 6;
    public String  fieldString = "Hello World!";
    public String  fieldStringJp = "こんにちは、世界！";

    public static int staticFieldIntArray[] = {1, 2, 3, 4, 5};
    public static char staticFieldCharArray[] = {'あ', 'い', 'う', 'え', 'お'};
    public boolean fieldBoolArray[]   = {false, true, false, true, false};
    public String  fieldStringArray[] = { "Hello", "World!", "こんにちは", "世界" };


    public static void setStaticFieldBool    (boolean value) { staticFieldBool = value; }
    public static void setStaticFieldByte    (byte    value) { staticFieldByte = value; }
    public static void setStaticFieldShort   (short   value) { staticFieldShort = value; }
    public static void setStaticFieldInt     (int     value) { staticFieldInt = value; }
    public static void setStaticFieldLong    (long    value) { staticFieldLong = value; }
    public static void setStaticFieldFloat   (float   value) { staticFieldFloat = value; }
    public static void setStaticFieldDouble  (double  value) { staticFieldDouble = value; }
    public static void setStaticFieldString  (String  value) { staticFieldString = value; }
    public static void setStaticFieldStringJp(String  value) { staticFieldStringJp = value; }

    public static boolean getStaticFieldBool()     { return staticFieldBool; }
    public static byte    getStaticFieldByte()     { return staticFieldByte; }
    public static short   getStaticFieldShort()    { return staticFieldShort; }
    public static int     getStaticFieldInt()      { return staticFieldInt; }
    public static long    getStaticFieldLong()     { return staticFieldLong; }
    public static float   getStaticFieldFloat()    { return staticFieldFloat; }
    public static double  getStaticFieldDouble()   { return staticFieldDouble; }
    public static String  getStaticFieldString()   { return staticFieldString; }
    public static String  getStaticFieldStringJp() { return staticFieldStringJp; }

    public void setFieldBool    (boolean value) { fieldBool = value; }
    public void setFieldByte    (byte    value) { fieldByte = value; }
    public void setFieldShort   (short   value) { fieldShort = value; }
    public void setFieldInt     (int     value) { fieldInt = value; }
    public void setFieldLong    (long    value) { fieldLong = value; }
    public void setFieldFloat   (float   value) { fieldFloat = value; }
    public void setFieldDouble  (double  value) { fieldDouble = value; }
    public void setFieldString  (String  value) { fieldString = value; }
    public void setFieldStringJp(String  value) { fieldStringJp = value; }

    public boolean getFieldBool()     { return fieldBool; }
    public byte    getFieldByte()     { return fieldByte; }
    public short   getFieldShort()    { return fieldShort; }
    public int     getFieldInt()      { return fieldInt; }
    public long    getFieldLong()     { return fieldLong; }
    public float   getFieldFloat()    { return fieldFloat; }
    public double  getFieldDouble()   { return fieldDouble; }
    public String  getFieldString()   { return fieldString; }
    public String  getFieldStringJp() { return fieldStringJp; }




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


    @Test public void testRef() throws Exception
    {
        testGlobalRef();
        testWeakRef();
    }
    public native void testGlobalRef() throws Exception;
    public native void testWeakRef() throws Exception;

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

    @Test public native void testFieldBool() throws Exception;
    @Test public native void testFieldByte() throws Exception;
    @Test public native void testFieldShort() throws Exception;
    @Test public native void testFieldInt() throws Exception;
    @Test public native void testFieldLong() throws Exception;
    @Test public native void testFieldFloat() throws Exception;
    @Test public native void testFieldDouble() throws Exception;
    @Test public native void testFieldString() throws Exception;
    @Test public native void testFieldStringJp() throws Exception;

    @Test public native void testArrayField() throws Exception;


    @Test public native void testStaticMethodBool() throws Exception;
    @Test public native void testStaticMethodByte() throws Exception;
    @Test public native void testStaticMethodShort() throws Exception;
    @Test public native void testStaticMethodInt() throws Exception;
    @Test public native void testStaticMethodLong() throws Exception;
    @Test public native void testStaticMethodFloat() throws Exception;
    @Test public native void testStaticMethodDouble() throws Exception;
    @Test public native void testStaticMethodString() throws Exception;
    @Test public native void testStaticMethodStringJp() throws Exception;

    @Test public native void testMethodBool() throws Exception;
    @Test public native void testMethodByte() throws Exception;
    @Test public native void testMethodShort() throws Exception;
    @Test public native void testMethodInt() throws Exception;
    @Test public native void testMethodLong() throws Exception;
    @Test public native void testMethodFloat() throws Exception;
    @Test public native void testMethodDouble() throws Exception;
    @Test public native void testMethodString() throws Exception;
    @Test public native void testMethodStringJp() throws Exception;

    @Test public native void testArray() throws Exception;

}