/*
** Agartha-Software, 2023
** C++evy
** File description:
** Color
*/

#include "Color.hpp"

cevy::engine::Color::Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
    : r(r), g(g), b(b), a(a) {}

// cevy::engine::Color::operator ::Color() { return ::Color{r, g, b, a}; };
// cevy::engine::Color::operator const ::Color() const { return ::Color{r, g, b, a}; };
