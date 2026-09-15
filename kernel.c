#include "kernel.h"
#include "keyboard.h"

void kernel_main()
{
    while (1)
    {
        int com;

        print("\nSomething Project v0.2\n");
        print("1 - Calculator");     
        print("live@user S~/: \n\n");

        input_int(&com);

        if (com == 1)
        {
            int n1;
            int n2;
            int rslt;
            char op[2];

            print("\n=== Calculator v0.1 ===\n\n");

            print("Type the first number: ");
            input_int(&n1);

            print("Choose the operation (+, -, *, /): ");
            input(op);

            print("Type the last number: ");
            input_int(&n2);

            if (op[0] == '+')
            {
                rslt = n1 + n2;
            }
            else if (op[0] == '-')
            {
                rslt = n1 - n2;
            }
            else if (op[0] == '*')
            {
                rslt = n1 * n2;
            }
            else if (op[0] == '/')
            {
                if (n2 == 0)
                {
                    print("Cannot divide by zero!\n");
                    continue;
                }

                rslt = n1 / n2;
            }
            else
            {
                print("Invalid operator!\n");
                continue;
            }

            print("Result: ");
            print_int(rslt);
            print("\n");
        }
        else
        {
            print("Invalid command!\n");
        }
    }
}
