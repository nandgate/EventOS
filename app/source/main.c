#include "hal/Led.h"
#include "os/os.h"

void SystemCoreClockUpdate(void);
extern uint32_t SystemCoreCLock;

void Blinky(os_context_t context) {
    (void)context;
    led_Toggle(LED);
    os_DoAfterWith(Blinky, context, 250);
}

int main(void) {
    SystemCoreClockUpdate();
    led_Init();     // Uses for testing the OS

    os_Init(SystemCoreCLock / OS_CLOCKS_PER_TICK);
    os_Do(Blinky, OS_NO_CONTEXT);
    while (1) {
        os_Exec();
    }
}
