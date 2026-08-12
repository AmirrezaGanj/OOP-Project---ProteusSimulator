#pragma once

#include <cmath>

// ============================================================
// Vector2D — a simple 2D point / vector used everywhere
// in the project for positions, offsets, and pin locations.
// Header-only, no .cpp needed.
// ============================================================

struct Vector2D {
    float x;
    float y;

    // Default constructor — initializes to origin (0, 0)
    Vector2D() : x(0.0f), y(0.0f) {}

    // Parameterized constructor
    Vector2D(float x, float y) : x(x), y(y) {}

    // ---- Arithmetic operators ----

    Vector2D operator+(const Vector2D& other) const {
        return Vector2D(x + other.x, y + other.y);
    }

    Vector2D operator-(const Vector2D& other) const {
        return Vector2D(x - other.x, y - other.y);
    }

    // Scalar multiplication (e.g. scaling by zoom factor)
    Vector2D operator*(float scalar) const {
        return Vector2D(x * scalar, y * scalar);
    }

    // Scalar division (e.g. converting screen coords to world coords)
    Vector2D operator/(float scalar) const {
        return Vector2D(x / scalar, y / scalar);
    }

    // Compound assignment operators
    Vector2D& operator+=(const Vector2D& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vector2D& operator-=(const Vector2D& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    // ---- Comparison ----

    bool operator==(const Vector2D& other) const {
        return (x == other.x && y == other.y);
    }

    bool operator!=(const Vector2D& other) const {
        return !(*this == other);
    }

    // ---- Utility methods ----

    // Euclidean length of this vector
    float length() const {
        return std::sqrt(x * x + y * y);
    }

    // Returns a normalized (unit) version of this vector
    Vector2D normalized() const {
        float len = length();
        if (len == 0.0f) return Vector2D(0.0f, 0.0f);
        return Vector2D(x / len, y / len);
    }
};