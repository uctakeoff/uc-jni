package com.example.uc.ucjnitest;

/**
 * Created by ushi on 2018/09/26.
 */

public class Point3 {
    private final long native_ptr;
    private Point3(long p)
    {
        native_ptr = p;
    }
    public native int x();
    public native int y();
    public native int z();
    public native void add(Point3 obj);
    public native void add(int x, int y, int z);
    public native void set(int x, int y, int z);
}
