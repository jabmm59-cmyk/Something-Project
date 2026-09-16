#include "kernel.h"
#include "keyboard.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static volatile unsigned short *video_memory =
    (unsigned short *)0xB8000;

static int cursor = 0;


static void scroll(void)
{
    int y;
    int x;

    for (y = 1; y < VGA_HEIGHT; y++)
    {
        for (x = 0; x < VGA_WIDTH; x++)
        {
            video_memory[(y - 1) * VGA_WIDTH + x] =
                video_memory[y * VGA_WIDTH + x];
        }
    }


    for (x = 0; x < VGA_WIDTH; x++)
    {
        video_memory[(VGA_HEIGHT - 1) * VGA_WIDTH + x] =
            (unsigned short)' ' | 0x0700;
    }


    cursor = (VGA_HEIGHT - 1) * VGA_WIDTH;
}


static void check_scroll(void)
{
    if (cursor >= VGA_WIDTH * VGA_HEIGHT)
    {
        scroll();
    }
}


void clear(void)
{
    int i;

    for (i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    {
        video_memory[i] =
            (unsigned short)' ' | 0x0700;
    }

    cursor = 0;
}


void print(const char *text)
{
    while (*text)
    {
        if (*text == '\n')
        {
            cursor += VGA_WIDTH -
                      (cursor % VGA_WIDTH);
        }

        else if (*text == '\b')
        {
            if (cursor > 0)
            {
                cursor--;

                video_memory[cursor] =
                    (unsigned short)' ' | 0x0700;
            }
        }

        else
        {
            video_memory[cursor] =
                (unsigned short)(*text) | 0x0700;

            cursor++;
        }

        check_scroll();

        text++;
    }
}


void input(char *buffer)
{
    int position = 0;

    while (1)
    {
        char c = keyboard_getchar();


        if (c == '\n')
        {
            buffer[position] = '\0';

            print("\n");

            return;
        }


        if (c == '\b')
        {
            if (position > 0)
            {
                position--;

                buffer[position] = '\0';

                print("\b");
            }

            continue;
        }


        if (c != 0 && position < 99)
        {
            buffer[position] = c;

            position++;


            char temporary[2];

            temporary[0] = c;
            temporary[1] = '\0';

            print(temporary);
        }
    }
}


void print_int(int number)
{
    char buffer[12];

    int i = 0;


    if (number == 0)
    {
        print("0");
        return;
    }


    if (number < 0)
    {
        print("-");

        number = -number;
    }


    while (number > 0)
    {
        buffer[i] =
            '0' + (number % 10);

        number /= 10;

        i++;
    }


    while (i > 0)
    {
        char character[2];

        i--;

        character[0] = buffer[i];
        character[1] = '\0';

        print(character);
    }
}


void input_int(int *number)
{
    char buffer[20];

    int i = 0;


    input(buffer);

    *number = 0;


    while (buffer[i] >= '0' &&
           buffer[i] <= '9')
    {
        *number =
            (*number * 10) +
            (buffer[i] - '0');

        i++;
    }
}


int strcmp(const char *a, const char *b)
{
    int i = 0;


    while (a[i] != '\0' &&
           b[i] != '\0')
    {
        if (a[i] != b[i])
        {
            return 1;
        }

        i++;
    }


    if (a[i] == b[i])
    {
        return 0;
    }


    return 1;
}
