#ifndef VEC_H
#define VEC_H

#include <ostream>

class Vec
{
public:
    int x, y;

    Vec(const int &new_x = 0, const int &new_y = 0) : x(new_x), y(new_y)
    {

    }

    Vec operator- () const
    {
        return Vec(-x, -y);
    }
};

inline Vec operator+ (const Vec &lhs, const Vec &rhs)
{
    return Vec(lhs.x + rhs.x, lhs.y + rhs.y);
}

inline Vec operator- (const Vec &lhs, const Vec &rhs)
{
    return Vec(lhs.x - rhs.x, lhs.y - rhs.y);
}

inline bool operator< (const Vec &lhs, const Vec &rhs)
{
    if (lhs.x < rhs.x)
        return true;
    else
        return lhs.y < rhs.y;
}

inline std::ostream& operator<< (std::ostream &out, const Vec &v)
{
    out << "(" << v.x << ", " << v.y << ")";
    return out;
}

/*template<typename T>
class Vec_
{
public:
    T x, y;

    Vec_(const T &new_x = 0, const T &new_y = 0) : x(new_x), y(new_y)
    {

    }

    auto operator- () const
    {
        return Vec_<T>(-x, -y);
    }
};

template<typename T, typename U>
auto operator+ (const Vec_<T> &lhs, const Vec_<U> &rhs)
{
    return Vec_<decltype(lhs.x + rhs.x)>(lhs.x + rhs.x, lhs.y + rhs.y);
}

template<typename T, typename U>
auto operator- (const Vec_<T> &lhs, const Vec_<U> &rhs)
{
    return Vec_<decltype(lhs.x - rhs.x)>(lhs.x - rhs.x, lhs.y - rhs.y);
}

template<typename T, typename U>
bool operator< (const Vec_<T> &lhs, const Vec_<U> &rhs)
{
    if (lhs.x < rhs.x)
        return true;
    else
        return lhs.y < rhs.y;
}

template<typename T>
std::ostream& operator<< (std::ostream &out, const Vec_<T> &v)
{
    out << "(" << v.x << ", " << v.y << ")";
    return out;
}

typedef Vec_<int> Vec;*/

#endif // VEC_H
