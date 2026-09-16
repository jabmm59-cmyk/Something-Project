#include "kernel.h"
#include "commands.h"


void kernel_main(void)
{
    char command[100];


    clear();


    print("================================\n");
    print("      Something Project v0.3\n");
    print("================================\n\n");


    print("Type 'help' to see available commands.\n\n");


    while (1)
    {
        print("live@user S~/: ");

        input(command);

        execute_command(command);

        print("\n");
    }
}
