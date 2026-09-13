#include "keyboard.h"

#define KEYBOARD_DATA   0x60
#define KEYBOARD_STATUS 0x64

static unsigned char read_port(unsigned short port)
{
    unsigned char value;

    __asm__ volatile (
        "inb %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );

    return value;
}

char keyboard_getchar(void)
{
    unsigned char scancode;

    while (!(read_port(KEYBOARD_STATUS) & 1))
    {
    }

    scancode = read_port(KEYBOARD_DATA);

    static const char keyboard[] =
        "\0"
        "\x1B"
        "1234567890-="
        "\b"
        "\t"
        "qwertyuiop[]"
        "\n"
        "\0"
        "asdfghjkl;'"
        "`"
        "\0"
        "\\"
        "zxcvbnm,./"
        "\0"
        "*"
        "\0"
        " ";

    if (scancode < sizeof(keyboard) - 1)
        return keyboard[scancode];

    return 0;
}
