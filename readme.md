# EventOS

A cooperative, event-driven operating system for microcontrollers.
EventOS sits between a superloop and an RTOS: it gives you the structure of a
real OS — actions, timers, pub/sub, a bulletin board — without the complexity
and footprint of preemption, scheduling priorities, or per-task stacks. Actions
run to completion in FIFO order on a single stack, and the runtime never
interrupts one piece of work to start another.

## A first taste

The `Blinky-posix` example runs on any POSIX host — no hardware required — and
is the whole program:

```c
#include "Led.h"
#include "os/os.h"
#include "posix.h"

void Blinky(os_context_t context) {
    (void)context;
    led_Toggle(LED);
    os_DoAfter(Blinky, OS_NO_CONTEXT, OS_NO_KEY, 200);
}

int main(void) {
    posix_Init();
    os_Do(Blinky, OS_NO_CONTEXT);
    for (;;) {
        posix_Tick();
        os_Exec();
    }
}
```

`os_Do` posts the first invocation; each call to `Blinky` toggles the LED and
re-posts itself 200 ms later through the timer queue. There are no threads to
create, no locks to take, no priorities to tune.

## When EventOS fits

- Your firmware is already event-shaped: things happen in response to timers,
  interrupts, button presses, incoming messages.
- You want **deterministic, in-order execution** — no priority inversions, no
  "which task ran first" or "why is my task being starved" surprises.
- You want a small footprint (~4 KB text, ~1.2 KB RAM on STM32F103 with every
  feature in-use).
- You're willing to write non-blocking actions and break long-running work into
  steps.

## When to reach for an RTOS instead

- You need **preemptive** scheduling — a high-priority task must be able to
  interrupt a lower-priority one mid-execution to meet hard real-time
  deadlines.
- Your application is naturally thread-shaped, with long sequential workflows
  and blocking APIs (`recv`, `pthread_cond_wait`, etc.).
- You need a rich IPC surface (counting semaphores, blocking mutexes, mailboxes
  with blocking reads).

## Try it

On a laptop:

```bash
cd examples/Blinky-posix
make
./build/Blinky-posix        # Ctrl-C to stop
```

You'll see `LED` state changes printed with millisecond timestamps — the same
semantics the firmware produces on real hardware, just with stdout instead of a
GPIO pin.

On hardware (STM32F103 "BluePill", `arm-none-eabi-gcc`):

```bash
cd examples/Blinky
make
# flash build/Blinky.elf with your tool of choice
```

## Learn more

- **[examples/](examples/)** — runnable code for every feature. Start with
  `Blinky-posix`, then `Blinky` (bluepill). The rest cover contexts, pub/sub,
  the bulletin board, cancellable timers, ISR handoff, software debounce, chained
  actions, and a combined all-features demo.
- **[EventOS.md](EventOS.md)** — the design tour: concepts, patterns, concurrency model,
  configuration, how to port to a new platform.
- **`doxygen eventos.doxy`** — generates the Doxygen API reference from the
  headers into `docs/`.
- **`./build.sh`** (from the repo root) — runs the full unit test suite and
  regenerates the docs.

## License

[BSD Zero Clause License (0BSD)](LICENSE) — use, modify, and redistribute
freely; no attribution or warranty.

© 2022–2026 NAND Gate Technologies, LLC
