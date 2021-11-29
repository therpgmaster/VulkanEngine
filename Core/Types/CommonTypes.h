#pragma once

#include <stdint.h>
#include <glm/glm.hpp>

struct Transform
{
	glm::vec3 translation{};
	glm::vec3 scale{ 1.f, 1.f, 1.f };
	glm::vec3 rotation{};

	glm::mat4 mat4() const { return makeMatrix(rotation, scale, translation); }

	glm::mat4 makeMatrix(const glm::vec3& rotationIn, const glm::vec3& scaleIn, 
				const glm::vec3& translationIn = { 0.f, 0.f, 0.f }) const
	{
		/* returns Translation * Rz * Ry * Rx * Scale
		* Tait-bryan angles Z(1)-Y(2)-X(3) rotation order
		* https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix */
		const float c3 = glm::cos(rotationIn.x);
		const float s3 = glm::sin(rotationIn.x);
		const float c2 = glm::cos(rotationIn.y);
		const float s2 = glm::sin(rotationIn.y);
		const float c1 = glm::cos(rotationIn.z);
		const float s1 = glm::sin(rotationIn.z);
		return glm::mat4
		{
			{
				scaleIn.x * (c1 * c2),
				scaleIn.x * (c2 * s1),
				scaleIn.x * (-s2),
				0.0f,
			},
			{
				scaleIn.y * (c1 * s2 * s3 - c3 * s1),
				scaleIn.y * (c1 * c3 + s1 * s2 * s3),
				scaleIn.y * (c2 * s3),
				0.0f,
			},
			{
				scaleIn.z * (s1 * s3 + c1 * c3 * s2),
				scaleIn.z * (c3 * s1 * s2 - c1 * s3),
				scaleIn.z * (c2 * c3),
				0.0f,
			},
			{translationIn.x, translationIn.y, translationIn.z, 1.0f}
		};
	}

	glm::mat4 makeMatrix(const glm::vec3& rotationIn) const { return makeMatrix(rotationIn, { 1.f, 1.f, 1.f }); }

	glm::vec3 getForwardVector() const
	{
		return { sin(rotation.y), 0.f, cos(rotation.y) };
		//return rotateVector(quaternionFromRotation(rotation), glm::vec3{ 1.f, 0.f, 0.f });
	}

	static double degToRad(const double& degrees) { return (degrees * 0.01745329251); }

	static glm::vec3 rotateVector(const glm::vec4& rotQuat, const glm::vec3& v3)
	{
		const glm::vec4 v4 = { v3.x, v3.y, v3.z, 0.f };
		glm::vec4 qw = { rotQuat.w, rotQuat.w, rotQuat.w, rotQuat.w };
		// cross product of vector and quat-rotation, times 2
		glm::vec4 t = crossVec4(rotQuat, v4);
		t = t + t;
		glm::vec4 r = ((qw * t) + v4) + crossVec4(rotQuat, t);
		return { r.x, r.y, r.z };
	}

	// replacement for missing overload of cross() that accepts 4-component vectors (w=0)
	static glm::vec4 crossVec4(const glm::vec4& a, const glm::vec4& b)
	{
		const glm::vec3 a3 = { a.x, a.y, a.z };
		const glm::vec3 b3 = { b.x, b.y, b.z };
		glm::vec3 p = glm::cross(a3, b3);
		return glm::vec4{ p.x, p.y, p.z, 0.f };
	}

	// assumes yaw = z, pitch = y, roll = x
	static glm::vec4 quaternionFromRotation(const glm::vec3& v)
	{
		// abbreviations for the angular functions
		const auto cy = cos(v.z); // in original formula all components were * 0.5 (halved)
		const auto sy = sin(v.z);
		const auto cp = cos(v.y);
		const auto sp = sin(v.y);
		const auto cr = cos(v.x);
		const auto sr = sin(v.x);

		glm::vec4 q{};
		q.w = cr * cp * cy + sr * sp * sy;
		q.x = sr * cp * cy - cr * sp * sy;
		q.y = cr * sp * cy + sr * cp * sy;
		q.z = cr * cp * sy - sr * sp * cy;

		return q;
	}
};

struct ActorTransform
{
	const Transform& get() const { return transform; }
	void set(const Transform& tf) { transform = tf; updated = true; }
	void resetUpdatedFlag() { updated = false; }
	const bool& wasUpdated() const { return updated; }
private:
	Transform transform{};
	bool updated = true;
};

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