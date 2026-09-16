#include "keyboard.h"

#define KEYBOARD_DATA 0x60
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
    static int shift = 0;

    unsigned char scancode;

    while (!(read_port(KEYBOARD_STATUS) & 1))
    {
    }

    scancode = read_port(KEYBOARD_DATA);


    /* Shift esquerdo */
    if (scancode == 0x2A)
    {
        shift = 1;
        return 0;
    }


    /* Shift direito */
    if (scancode == 0x36)
    {
        shift = 1;
        return 0;
    }


    /* Soltar Shift */
    if (scancode == 0xAA || scancode == 0xB6)
    {
        shift = 0;
        return 0;
    }


    /* Tecla liberada */
    if (scancode & 0x80)
    {
        return 0;
    }


    /* Enter */
    if (scancode == 0x1C)
        return '\n';


    /* Backspace */
    if (scancode == 0x0E)
        return '\b';


    /* Números */

    if (scancode == 0x02)
        return shift ? '!' : '1';

    if (scancode == 0x03)
        return shift ? '@' : '2';

    if (scancode == 0x04)
        return shift ? '#' : '3';

    if (scancode == 0x05)
        return shift ? '$' : '4';

    if (scancode == 0x06)
        return shift ? '%' : '5';

    if (scancode == 0x07)
        return shift ? '^' : '6';

    if (scancode == 0x08)
        return shift ? '&' : '7';

    if (scancode == 0x09)
        return shift ? '*' : '8';

    if (scancode == 0x0A)
        return shift ? '(' : '9';

    if (scancode == 0x0B)
        return shift ? ')' : '0';

    if (scancode == 0x0C)
        return shift ? '_' : '-';

    if (scancode == 0x0D)
        return shift ? '+' : '=';


    /* Letras */

    static const char normal[] =
        "qwertyuiop"
        "asdfghjkl"
        "zxcvbnm";


    static const unsigned char codes[] =
    {
        0x10, 0x11, 0x12, 0x13, 0x14,
        0x15, 0x16, 0x17, 0x18, 0x19,

        0x1E, 0x1F, 0x20, 0x21, 0x22,
        0x23, 0x24, 0x25, 0x26,

        0x2C, 0x2D, 0x2E, 0x2F, 0x30,
        0x31, 0x32
    };


    int i;

    for (i = 0; i < sizeof(codes); i++)
    {
        if (scancode == codes[i])
        {
            if (shift)
            {
                return normal[i] - 32;
            }

            return normal[i];
        }
    }


    /* Espaço */

    if (scancode == 0x39)
        return ' ';


    /* Vírgula */

    if (scancode == 0x33)
        return shift ? '<' : ',';


    /* Ponto */

    if (scancode == 0x34)
        return shift ? '>' : '.';


    /* Barra */

    if (scancode == 0x35)
        return shift ? '?' : '/';


    return 0;
}
