#include "kernel.h"
#include "sysinfo.h"


void sysinfo(void)
{
    print("\n=== System Information ===\n\n");

    print("OS: Something Project\n");
    print("Version: 0.3\n");
    print("Kernel: Something Kernel\n");
    print("Architecture: i386\n");
    print("Video: VGA Text Mode\n");
    print("Keyboard: PS/2\n");
}
