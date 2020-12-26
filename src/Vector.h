
#ifndef __VECTOR_H__
#define __VECTOR_H__

//**********************************************************************
//* 3 dimensional vector (X,Y,Z)
//**********************************************************************
template <class T>
class Vector3
{
public:
    //******************************************************************
    //* Constructors
    //******************************************************************
    // Default sets all components to zero.
    //------------------------------------------------------------------
    Vector3() : x(0), y(0), z(0) {}
    Vector3(T x, T y, T z) : x(x), y(y), z(z) {}
    Vector3(const Vector3<T> &v) : x(v.x), y(v.y), z(v.z) {}
    //******************************************************************

    //******************************************************************
    //* Operators
    //******************************************************************
    Vector3<T> &operator = (const Vector3<T> &v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }

    template<class U>
    Vector3<T> &operator += (const Vector3<U> &v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    template<class U>
    Vector3<T> &operator -= (const Vector3<U> &v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    template<class U>
    Vector3<T> &operator *= (U scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    template<class U>
    Vector3<T> &operator /= (U scalar)
    {
        assert(scalar != 0);
        scalar = 1.0f / scalar;
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    Vector3<T> &operator - ()
    {
        x = -x;
        y = -y;
        z = -z;
        return *this;
    }

    float &operator [] (int i)
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
    //******************************************************************

    //******************************************************************
    //* Methods
    //******************************************************************
    bool IsZero ()
    {
        return !(x || y || z);
    }

    float LengthSquared ()
    {
        return x*x + y*y + z*z;
    }

    float Length ()
    {
        return sqrtf(LengthSquared());
    }

    void Normalize ()
    {
        float magnitude = Length();
        assert(magnitude != 0);

        magnitude = 1.0f / magnitude;

        x *= magnitude;
        y *= magnitude;
        z *= magnitude;
    
    }
    bool IsNormalized ()
    {
        return Length() == 1.0f;
    }
    //******************************************************************

    T x;
    T y;
    T z;
};

//**********************************************************************
//* 2 dimensional vector (X,Y)
//**********************************************************************
template <class T>
class Vector2
{
public:
    //******************************************************************
    //* Constructors
    //******************************************************************
    // Default sets all components to zero.
    //------------------------------------------------------------------
    Vector2() : x(0), y(0) {}
    Vector2(T x, T y) : x(x), y(y) {}
    Vector2(const Vector2<T> &v) : x(v.x), y(v.y) {}
    //******************************************************************

    //******************************************************************
    //* Operators
    //******************************************************************
    Vector2<T> &operator = (const Vector2<T> &v)
    {
        x = v.x;
        y = v.y;
        return *this;
    };

    template<class U>
    Vector2<T> &operator+=(const Vector2<U> &v)
    {
        x += v.x;
        y += v.y;
        return *this;
    };

    template<class U>
    Vector2<T> &operator -= (const Vector2<U> &v)
    {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    template<class U>  
    Vector2<T> &operator *= (U scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    template<class U>
    Vector2<T> &operator /= (U scalar)
    {
        assert(scalar != 0);
        scalar = 1.0f / scalar;
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vector2<T> &operator - ()
    {
        x = -x;
        y = -y;
        return *this;
    }

    float &operator [] (int i)
    {
        if (i == 0) {
            return x;
        } else if (i == 1) {
            return y;
        } else {
            assert("[] Access error!");
        }
    }
    //******************************************************************

    //******************************************************************
    //* Methods
    //******************************************************************
    bool IsZero ()
    {
        return !(x || y);
    }

    float LengthSquared ()
    {
        return x*x + y*y;
    }

    float Length ()
    {
        return sqrtf(LengthSquared());
    }

    void Normalize ()
    {
        float magnitude = Length();
        assert(magnitude != 0);

        magnitude = 1.0f / magnitude;

        x *= magnitude;
        y *= magnitude;
    
    }
    bool IsNormalized ()
    {
        return Length() == 1.0f;
    }
    //******************************************************************

    T x;
    T y;
};

//**********************************************************************
//* Vector3
//**********************************************************************
template <class T>
inline bool operator==(const Vector3<T> &v, const Vector3<T> &u)
{
    return (v.x == u.x &&
            v.y == u.y &&
            v.z == u.z);
}

template <class T, class U>
inline Vector3<T> operator+(const Vector3<T> &v, const Vector3<U> &u)
{
    return Vector3<T>(v.x+u.x, v.y+u.y, v.z+u.z);
}

template <class T, class U>
inline Vector3<T> operator-(const Vector3<T> &v,const Vector3<U> &u)
{
    return Vector3<T>(v.x-u.x, v.y-u.y, v.z-u.z);
}

template <class T, class U>
inline Vector3<T> operator*(const Vector3<T> &v, U scalar)
{
    return Vector3<T>(v.x*scalar, v.y*scalar, v.z*scalar);
}

template <class T, class U>
inline Vector3<T> operator*(U scalar, const Vector3<T> &v)
{
    return Vector3<T>(v.x*scalar, v.y*scalar, v.z*scalar);
}

template <class T, class U>
inline float operator*(const Vector3<T> &v, const Vector3<U> &u)
{
    return v.x*u.x + v.y*u.y + v.z*u.z;
}

template <class T, class U>
inline Vector3<T> operator/(const Vector3<T> &v, U scalar)
{
    assert(scalar != 0);
    scalar = 1.0f / scalar;
    return Vector3<T>(v.x*scalar, v.y*scalar, v.z*scalar);
}

template <class T, class U>
inline Vector3<T> CrossProduct(const Vector3<T> &v, const Vector3<U> &u)
{
    return Vector3<T>(v.y*u.z - v.z*u.y,
                  v.z*u.x - v.x*u.z,
                  v.x*u.y - v.y*u.x);
}

template <class T>
inline Vector3<T> Lerp(const Vector3<T> &v, const Vector3<T> &u, float t)
{
    return Vector3<float>(v.x + (u.x - v.x) * t,
                   v.y + (u.y - v.y) * t,
                   v.z + (u.z - v.z) * t);
}

template <class T>
inline Vector3<T> Clamp(const Vector3<T> &v, T min, T max)
{
    return Vector3<T>(v.x < min ? min : (v.x > max ? max : v.x),
                      v.y < min ? min : (v.y > max ? max : v.y),
                      v.z < min ? min : (v.z > max ? max : v.z));
}

template <class T>
inline Vector3<T> min(const Vector3<T> &v, const Vector3<T> &u)
{
    return Vector3<T>(v.x < u.x ? v.x : u.x,
                      v.y < u.y ? v.z : u.y,
                      v.z < u.z ? v.z : u.z);
}

template <class T>
inline Vector3<T> max(const Vector3<T> &v, const Vector3<T> &u)
{
    return Vector3<T>(v.x > u.x ? v.x : u.x,
                      v.y > u.y ? v.z : u.y,
                      v.z > u.z ? v.z : u.z);
}

template <class T>
inline float DistanceBetween(const Vector3<T> &v, const Vector3<T> &u)
{
    Vector3<T> distance = v - u;
    return distance.Length();
}

template <class T>
inline float DistanceBetweenSquared (const Vector3<T> &v, const Vector3<T> &u)
{
    Vector3<T> distance = v - u;
    return distance.LengthSquared();
}

//**********************************************************************
//* Vector2
//**********************************************************************

template <class T>
inline bool operator==(const Vector2<T> &v, const Vector2<T> &u)
{
    return (v.x == u.x &&
            v.y == u.y);
}

template <class T, class U>
inline Vector2<T> operator+(const Vector2<T> &v, const Vector2<U> &u)
{
    return Vector2<T>(v.x+u.x, v.y+u.y);
}

template <class T, class U>
inline Vector2<T> operator-(const Vector2<T> &v,const Vector2<U> &u)
{
    return Vector2<T>(v.x-u.x, v.y-u.y);
}

template <class T, class U>
inline Vector2<T> operator*(const Vector2<T> &v, U scalar)
{
    return Vector2<T>(v.x*scalar, v.y*scalar);
}

template <class T, class U>
inline Vector2<T> operator*(U scalar, const Vector2<T> &v)
{
    return Vector2<T>(v.x*scalar, v.y*scalar);
}

template <class T, class U>
inline float operator*(const Vector2<T> &v, const Vector2<U> &u)
{
    return v.x*u.x + v.y*u.y + v.z*u.z;
}

template <class T, class U>
inline Vector2<T> operator/(const Vector2<T> &v, U scalar)
{
    assert(scalar != 0);
    scalar = 1.0f / scalar;
    return Vector2<T>(v.x*scalar, v.y*scalar);
}

template <class T, class U>
inline Vector2<T> CrossProduct(const Vector2<T> &v, const Vector2<U> &u)
{
    return Vector2<T>(v.y*u.z - v.z*u.y,
                      v.z*u.x - v.x*u.z);
}

template <class T>
inline Vector2<T> Lerp(const Vector2<T> &v, const Vector2<T> &u, float t)
{
    return Vec2f(v.x + (u.x - v.x) * t,
                   v.y + (u.y - v.y) * t);
}

template <class T>
inline Vector2<T> Clamp(const Vector3<T> &v, T min, T max)
{
    return Vector2<T>(v.x < min ? min : (v.x > max ? max : v.x),
                      v.y < min ? min : (v.y > max ? max : v.y));
}

template <class T>
inline Vector2<T> min(const Vector2<T> &v, const Vector2<T> &u)
{
    return Vector2<T>(v.x < u.x ? v.x : u.x,
                      v.y < u.y ? v.z : u.y);
}

template <class T>
inline Vector2<T> max(const Vector2<T> &v, const Vector2<T> &u)
{
    return Vector2<T>(v.x > u.x ? v.x : u.x,
                      v.y > u.y ? v.z : u.y,
                      v.z > u.z ? v.z : u.z);
}

template <class T>
inline float DistanceBetween(const Vector2<T> &v, const Vector2<T> &u)
{
    Vector2<T> distance = v - u;
    return distance.Length();
}

template <class T>
inline float DistanceBetweenSquared (const Vector2<T> &v, const Vector2<T> &u)
{
    Vector2<T> distance = v - u;
    return distance.LengthSquared();
}

typedef Vector3<int32_t> Vec3;
typedef Vector3<float> Vec3f;

typedef Vector2<int32_t> Vec2;
typedef Vector2<int16_t> Vec2h;
typedef Vector2<float> Vec2f;

#endif
