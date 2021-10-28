#pragma once

#include <stdint.h>

template<class T = float>
class Vector
{
	Vector<T>(const T& x_, const T& y_, const T& z_) : x{ x_ }, y{ y_ }, z{ z_ } {};
	Vector<T>(const T& v) : x{ v }, y{ v }, z{ v } {};
	Vector() : x{ 0 }, y{ 0 }, z{ 0 } {};
	T x; T y; T z;
	Vector operator+(const Vector& other) { return Vector{ x + other.x, y + other.y, z + other.z }; }
	Vector operator-(const Vector& other) { return Vector{ x - other.x, y - other.y, z - other.z }; }
	Vector operator*(const Vector& other) { return Vector{ x * other.x, y * other.y, z * other.z }; }
	Vector operator/(const Vector& other) { return Vector{ x / other.x, y / other.y, z / other.z }; }
};

template<class T = float>
class Vector2D 
{
public:
	Vector2D<T>(const T& x_, const T& y_) : x{ x_ }, y{ y_ } {};
	Vector2D<T>(const T& v) : x{ v }, y{ v } {};
	Vector2D() : x{ 0 }, y{ 0 } {};
	T x; T y;
	Vector2D operator+(const Vector2D& other) { return Vector2D{ x + other.x, y + other.y }; }
	Vector2D operator-(const Vector2D& other) { return Vector2D{ x - other.x, y - other.y }; }
	Vector2D operator*(const Vector2D& other) { return Vector2D{ x * other.x, y * other.y }; }
	Vector2D operator/(const Vector2D& other) { return Vector2D{ x / other.x, y / other.y }; }
	friend bool operator==(const Vector2D& lh, const Vector2D& rh) { return lh.x == rh.x && lh.y == rh.y; }
	friend bool operator!=(const Vector2D& lh, const Vector2D& rh) { return !(lh == rh); }
};

class VectorInt
{
public:
	VectorInt(const uint32_t& x_, const uint32_t& y_, const uint32_t& z_) : x{ x_ }, y{ y_ }, z{ z_ } {};
	VectorInt(const uint32_t& v) : x{ v }, y{ v }, z{ v } {};
	VectorInt() : x{ 0 }, y{ 0 }, z{ 0 } {};
	uint32_t x; uint32_t y; uint32_t z;
	VectorInt operator+(const VectorInt& other) { return VectorInt{ x + other.x, y + other.y, z + other.z }; }
	VectorInt operator-(const VectorInt& other) { return VectorInt{ x - other.x, y - other.y, z - other.z }; }
};
