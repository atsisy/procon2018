#include "include/types.hpp"
#include <cmath>
#include <limits>

u8 Field::ac_shift_offset;
u64 Field::field_size;

Field::Field()
        : field(field_size)
{}

Panel Field::at(u8 x, u8 y)
{
        return field.at(x + (y << this->ac_shift_offset));
}

FieldBuilder::FieldBuilder(u8 width, u8 height)
{
        Field::ac_shift_offset = (u64)(std::log2(width) + 0.5);
        Field::field_size = height << Field::ac_shift_offset;
}
