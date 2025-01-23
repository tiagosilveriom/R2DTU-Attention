#pragma once

struct Color {
    double x;
    double y;
    double z;

    enum Type {
        RGB,
        BGR,
        HSV
    };

    Type type;

    static Color background() { return { 0x1d, 0x1f, 0x21}; }
    static Color foreground() { return { 0xc5, 0xc8, 0xc6}; }
    static Color white() { return { 0xff, 0xff, 0xff}; }
    static Color black() { return { 0x28, 0x2a, 0x2e}; }
    static Color light_black() { return { 0x37, 0x3b, 0x41}; }
    static Color bright_red() { return { 0xdd, 0x00, 0x00}; }
    static Color red() { return { 0xa5, 0x42, 0x42}; }
    static Color light_red() { return { 0xcc, 0x66, 0x66}; }
    static Color bright_green() { return { 0x00, 0xdd, 0x00}; }
    static Color green() { return { 0x8c, 0x94, 0x40}; }
    static Color light_green() { return { 0xb5, 0xbd, 0x68}; }
    static Color bright_blue() { return { 0x00, 0x00, 0xdd}; }
    static Color blue() { return { 0x5f, 0x81, 0x9d}; }
    static Color light_blue() { return { 0x81, 0xa2, 0xbe}; }
};
