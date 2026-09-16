#include "kernel.h"
#include "commands.h"
#include "calculator.h"
#include "sysinfo.h"


void execute_command(char *command)
{
    if (strcmp(command, "help") == 0)
    {
        print("\nAvailable commands:\n\n");

        print("help       - Show this help\n");
        print("calculator - Open calculator\n");
        print("sysinfo    - Show system information\n");
        print("clear      - Clear the screen\n");
        print("about      - About Something Project\n");
    }

    else if (strcmp(command, "calculator") == 0)
    {
        calculator();
    }

    else if (strcmp(command, "sysinfo") == 0)
    {
        sysinfo();
    }

    else if (strcmp(command, "clear") == 0)
    {
        clear();
    }

    else if (strcmp(command, "about") == 0)
    {
        print("\nSomething Project\n");
        print("A small operating system written in C.\n");
        print("Version: 0.3\n");
    }

    else if (command[0] == '\0')
    {
        return;
    }

    else
    {
        print("\nCommand not found.\n");
        print("Type 'help' to see available commands.\n");
    }
}
