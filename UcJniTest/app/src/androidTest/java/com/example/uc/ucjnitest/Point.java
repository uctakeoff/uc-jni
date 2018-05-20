package com.example.uc.ucjnitest;

import java.util.Objects;

/**
 * Created by ushi on 2018/05/20.
 */

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

    @Override
    public String toString()
    {
        return "(" + x + "," + y + ")";
    }
    @Override
    public int hashCode()
    {
        return Objects.hash(x, y);
    }
    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Point p = (Point) o;
        return (p.x == x) && (p.y == y);
    }
}
