
#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdlib.h>
#include <math.h>
#include <assert.h>

template <class T>
class Vector3
{
public:
    //******************************************************************
    //* Constructors
    //******************************************************************
    // Default sets all components to zero.
    //------------------------------------------------------------------
    Vector3 ();
    //------------------------------------------------------------------
    Vector3 (T x, T y, T z);
    //------------------------------------------------------------------
    Vector3 (const Vector3 &v);
    //******************************************************************

    //******************************************************************
    //* Operators
    //******************************************************************
    Vector3<T> &operator = (const Vector3<T> &v);
    Vector3<T> &operator += (const Vector3<T> &v);
    Vector3<T> &operator -= (const Vector3<T> &v);
    Vector3<T> &operator *= (T scalar);
    Vector3<T> &operator /= (T scalar);
    //------------------------------------------------------------------
    // Unary Minus Operator
    //------------------------------------------------------------------
    // Negate the components of this Vector3.
    //------------------------------------------------------------------
    Vector3 &operator - ();
    //------------------------------------------------------------------
    // Array Subscript Operator
    //------------------------------------------------------------------
    // Allows access to the x, y and z components through an array 
    // subscript notation.
    //------------------------------------------------------------------
    float &operator [] (int i);
    //******************************************************************

    //******************************************************************
    //* Methods
    //******************************************************************
    // IsZero
    //------------------------------------------------------------------
    // Returns true if all components equal zero. 
    //------------------------------------------------------------------
    bool IsZero ();
    //------------------------------------------------------------------
    // LengthSquared
    //------------------------------------------------------------------
    // Returns the magnitude of the vector squared.
    //------------------------------------------------------------------
    float LengthSquared ();
    //------------------------------------------------------------------
    // Length
    //------------------------------------------------------------------
    // Returns the magnitude of the vector.
    //------------------------------------------------------------------
    float Length ();
    //------------------------------------------------------------------
    // Normalize
    void Normalize ();
    //------------------------------------------------------------------
    // IsNormalized
    //------------------------------------------------------------------
    // Compares the magnitude to one.
    //------------------------------------------------------------------
    bool IsNormalized ();
    //******************************************************************

    T x;
    T y;
    T z;
};

template <class T>
inline Vector3<T>::Vector3() : x(0.0f), y(0.0f), z(0.0f) {}

template <class T>
inline Vector3<T>::Vector3(T x, T y, T z) : x(x), y(y), z(z) {}

template <class T>
inline Vector3<T>::Vector3(const Vector3<T> &v) : x(v.x), y(v.y), z(v.z) {}

template <class T>
inline bool operator==(const Vector3<T> &v, const Vector3<T> &u)
{
    return (v.x == u.x &&
            v.y == u.y &&
            v.z == u.z);
}

template <class T>
inline Vector3<T> operator+(const Vector3<T> &v, const Vector3<T> &u)
{
    return Vector3<T>(v.x+u.x, v.y+u.y, v.z+u.z);
}

template <class T>
inline Vector3<T> operator-(const Vector3<T> &v,const Vector3<T> &u)
{
    return Vector3<T>(v.x-u.x, v.y-u.y, v.z-u.z);
}

template <class T>
inline Vector3<T> operator*(const Vector3<T> &v, T scalar)
{
    return Vector3<T>(v.x*scalar, v.y*scalar, v.z*scalar);
}

template <class T>
inline Vector3<T> operator*(T scalar, const Vector3<T> &v)
{
    return Vector3<T>(v.x*scalar, v.y*scalar, v.z*scalar);
}

template <class T>
inline float operator*(const Vector3<T> &v, const Vector3<T> &u)
{
    return v.x*u.x + v.y*u.y + v.z*u.z;
}

template <class T>
inline Vector3<T> operator/(const Vector3<T> &v, T scalar)
{
    assert(scalar != 0);
    scalar = 1.0f / scalar;
    return Vector3<T>(v.x*scalar, v.y*scalar, v.z*scalar);
}

template <class T>
inline Vector3<T> CrossProduct(const Vector3<T> &v, const Vector3<T> &u)
{
    return Vector3<T>(v.y*u.z - v.z*u.y,
                  v.z*u.x - v.x*u.z,
                  v.x*u.y - v.y*u.x);
}

template <class T>
inline Vector3<T> Lerp(const Vector3<T> &v, const Vector3<T> &u, float t)
{
    return Vector3<T>(v.x + (u.x - v.x) * t,
                   v.y + (u.y - v.y) * t,
                   v.z + (u.z - v.z) * t);
}

template <class T, class P, class Q>
inline Vector3<T> Clamp(const Vector3<T> &v, P min, Q max)
{
    return Vector3<T>(v.x < min ? min : (v.x > max ? max : v.x),
                      v.y < min ? min : (v.y > max ? max : v.y),
                      v.z < min ? min : (v.z > max ? max : v.z));
}

template <class T>
inline Vector3<T> Min(const Vector3<T> &v, const Vector3<T> &u)
{
    return Vector3<T>(v.x < u.x ? v.x : u.x,
                      v.y < u.y ? v.z : u.y,
                      v.z < u.z ? v.z : u.z);
}

template <class T>
inline Vector3<T> Max(const Vector3<T> &v, const Vector3<T> &u)
{
    return Vector3<T>(v.x > u.x ? v.x : u.x,
                      v.y > u.y ? v.z : u.y,
                      v.z > u.z ? v.z : u.z);
}

template <class T>
inline float DistanceBetween(const Vector3<T> &v1, const Vector3<T> &v2)
{
    Vector3<T> distance = v1 - v2;
    return distance.Length();
}

template <class T>
inline float DistanceBetweenSquared (const Vector3<T> &v, const Vector3<T> &u)
{
    Vector3<T> distance = v - u;
    return distance.LengthSquared();
}

template <class T>
inline Vector3<T> &Vector3<T>::operator=(const Vector3<T> &v)
{
    x = v.x;
    y = v.y;
    z = v.z;
    return *this;
} 

template <class T>
inline Vector3<T> &Vector3<T>::operator+=(const Vector3<T> &v)
{
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

template <class T>
inline Vector3<T> &Vector3<T>::operator-=(const Vector3<T> &v)
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

template <class T>
inline Vector3<T> &Vector3<T>::operator*=(T scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

template <class T>
inline Vector3<T> &Vector3<T>::operator/=(T scalar)
{
    assert(scalar != 0);
    scalar = 1.0f / scalar;
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

template <class T>
inline Vector3<T> &Vector3<T>::operator-()
{
    x = -x;
    y = -y;
    z = -z;
    return *this;
}

template <class T>
inline float &Vector3<T>::operator[](int i)
{
    if (i == 0) {
        return x;
    } else if (i == 1) {
        return y;
    } else if (i == 2) {
        return z;
    } else {
        assert("[] Access error!");
    }
}

template <class T>
inline bool Vector3<T>::IsZero()
{
    return !(x || y || z);
}

template <class T>
inline float Vector3<T>::LengthSquared()
{
    return x*x + y*y + z*z;
}

template <class T>
inline float Vector3<T>::Length()
{
    return sqrtf(LengthSquared());
}

template <class T>
inline void Vector3<T>::Normalize()
{
    float magnitude = Length();
    assert(magnitude != 0);

    magnitude = 1.0f / magnitude;

    x *= magnitude;
    y *= magnitude;
    z *= magnitude;
}

template <class T>
inline bool Vector3<T>::IsNormalized()
{
    return Length() == 1.0f;
}

#endif
