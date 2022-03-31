#include "hal/Led.h"
#include "os/os.h"

void blinkyTick(os_context_t context) {
    led_Toggle();
    os_DoAfter(blinkyTick, OS_NO_CONTEXT, 100);
}

int main(void) {
    led_Init();
    os_Init(80000);

    os_Do(blinkyTick, OS_NO_CONTEXT);
    while (1) {
        os_Exec();
    }
}
