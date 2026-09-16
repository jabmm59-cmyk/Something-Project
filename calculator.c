#include "kernel.h"
#include "calculator.h"


void calculator(void)
{
    int n1;
    int n2;
    int result;

    char op[2];


    print("\n=== Calculator ===\n\n");


    print("First number: ");
    input_int(&n1);


    print("Operation (+, -, *, /): ");
    input(op);


    print("Second number: ");
    input_int(&n2);


    if (op[0] == '+')
    {
        result = n1 + n2;
    }

    else if (op[0] == '-')
    {
        result = n1 - n2;
    }

    else if (op[0] == '*')
    {
        result = n1 * n2;
    }

    else if (op[0] == '/')
    {
        if (n2 == 0)
        {
            print("\nCannot divide by zero!\n");
            return;
        }

        result = n1 / n2;
    }

    else
    {
        print("\nInvalid operator!\n");
        return;
    }


    print("\nResult: ");

    print_int(result);

    print("\n");
}
