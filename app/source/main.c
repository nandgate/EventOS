#include "hal/Led.h"
#include "os/os.h"

void SystemCoreClockUpdate(void);
extern uint32_t SystemCoreCLock;

void blinkyTick(os_context_t context) {
    led_Toggle();
    os_DoAfter(blinkyTick, OS_NO_CONTEXT, 100);
}

int main(void) {
    SystemCoreClockUpdate();
    led_Init();
    os_Init(SystemCoreCLock / OS_CLOCKS_PER_TICK);

    os_Do(blinkyTick, OS_NO_CONTEXT);
    while (1) {
        os_Exec();
    }
}
