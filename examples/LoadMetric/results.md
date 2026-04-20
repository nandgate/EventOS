# LoadMetric — Test Results

Live demonstration of the load-metric pattern from EventOS.md §2.5:
drive the button-input ISR with an FM-modulated carrier from a function
generator, watch the FIFO depth track the integral of
(arrival rate − service rate).

## Setup

### Hardware

- **Target:** STM32F103 BluePill, 64 MHz (HSI × PLL).
- **Input:** function generator driving PB1 (pulled up internally,
  EXTI1 triggers on falling edge).
- **Capture:** Saleae Logic 16, 10 MS/s, 4 channels:
  - Ch0 — LED_0 (PA0), FIFO depth bit 0
  - Ch1 — LED_1 (PA1), FIFO depth bit 1
  - Ch2 — LED_2 (PA2), FIFO depth bit 2
  - Ch4 — PB1 direct probe, arrival edges from the function generator
  - Ch3 (LED / PC13) unused by this example.

### Firmware

- `examples/LoadMetric/main.c`, `arm-none-eabi-gcc -Os`.
- `OS_MAX_ENTRIES = 128` (Makefile `-DOS_MAX_ENTRIES=128`) to give the
  queue room to ride without tripping `os_Fail` during normal
  modulation.
- `os_ActionEnd` override emits `os_EntryInUse() − 1` as 3-bit binary
  on LED_0/LED_1/LED_2 after every action dispatch. The `−1` excludes
  the currently-running entry (which isn't freed until after
  `os_ActionEnd` returns), so the display shows *pending* depth.
- `os_Fail` override emits value 7 (`111`, all LEDs ON) and halts.
- `OnEdge` action is empty — depth reflects arrival rate, not
  per-action work.

### Display encoding

`led_On` drives the pin LOW; `led_Off` drives HIGH. So on the scope a
set bit reads as a LOW channel. Decoded value:

| Ch2 | Ch1 | Ch0 | Decoded depth |
|:---:|:---:|:---:|:---:|
| 1 | 1 | 1 | 0 (empty) |
| 1 | 1 | 0 | 1 |
| 1 | 0 | 1 | 2 |
| 1 | 0 | 0 | 3 |
| 0 | 1 | 1 | 4 |
| 0 | 1 | 0 | 5 |
| 0 | 0 | 1 | 6 (saturated; actual depth ≥ 7) |
| 0 | 0 | 0 | 7 — `os_Fail` tripped, system halted |

## Step 1 — Service-rate measurement

Unmodulated square wave, stepped up until the FIFO fails to keep up.

| Carrier | Debugger `os_entryInUseCount` | Result |
|---|---|---|
| 80 kHz | 0 | service comfortably above |
| 90 kHz | 1 | steady; just the current action, pool idle |
| 100 kHz | 128 (`OS_MAX_ENTRIES` exhausted) | `os_Fail` tripped |

**Measured service rate: ≈ 95 kHz.** That is, ≈ 10.5 µs per dispatch
cycle for the trivial-action + 3-LED-emit path.

Consistent with the ~10 µs os_Exec cycle time measured independently
in PubSub (double-toggle gap between back-to-back actions) and
BulletinBoard (same-tick Put/Get gap).

## Step 2 — FM ride

Carrier chosen below service rate so the queue drains every cycle.
Deviation chosen so the peak crosses service briefly.

| Parameter | Value |
|---|---|
| Carrier | 80 kHz |
| FM deviation | ±20 kHz (sweep 60–100 kHz) |
| Modulation waveform | Sine |
| Modulation frequency | 100 Hz (10 ms period) |
| Capture duration | 1 s |

### Result — time at each depth (stable ≥ 10 µs)

Over the 1 s capture:

| Displayed depth | Time (ms) | Fraction |
|---|---:|---:|
| 0 | 766.9 | 73.2% |
| 1 | 31.6 | 3.0% |
| 2 | 21.9 | 2.1% |
| 3 | 23.7 | 2.3% |
| 4 | 20.9 | 2.0% |
| 5 | 23.9 | 2.3% |
| 6 (saturated) | 158.9 | 15.2% |

The system spent ~73% of its time with an empty queue (valley of the
mod cycle), and ~15% pegged at the 3-bit saturation (peak of the mod
cycle, actual depth likely 7–12).

### Per-cycle shape

One representative 10 ms cycle:

```
Phase           Duration    Notes
──────────────────────────────────────────────────────────────────
valley (depth 0)   7.35 ms   arrival < service, queue idle
rise (0→6)         0.80 ms   arrival crosses service upward
saturation (6)     1.49 ms   display pegged; actual depth ≥ 7
fall (6→0)         0.35 ms   arrival < service, queue drains
total              9.99 ms   (10 ms mod period)
```

Rise is ~2× slower than fall: during the rise, arrival is just barely
above service (small excess); during the fall, arrival has dropped
well below service and service has no resistance from new arrivals.

### Depth over 4 cycles (40 ms window)

```
6│             ███████                       ███████                       ███████                       ███████
5│            █████████                     █████████                     █████████                     █████████
4│           ██████████                    ██████████                    ██████████                    ██████████
3│           ██████████                    ██████████                    ██████████                    ██████████
2│          ████████████                  ████████████                  ████████████                  ████████████
1│         █████████████                 █████████████                 █████████████                 █████████████
0│────────                 ────────────                ────────────                ────────────                ────────
 └────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
  100       105       110       115       120       125       130       135  (ms)
```

## Conclusion

- **Service rate of this build on this target is ≈ 95 kHz** (≈ 10.5 µs
  per `os_Exec` cycle for a trivial action).
- **FIFO depth tracks (arrival − service) as an integrator** exactly
  as §2.5 predicts. Sinusoidal FM input produces a repeating
  rise / saturate / fall / idle pattern locked to the modulation
  frequency.
- **The 3-bit binary display is a legible real-time load indicator.**
  Modulation at 100 Hz is visible on a scope; the saturation at 6 is
  an honest "queue is above the display's range" signal, and a
  sustained value of 7 unambiguously signals `os_Fail`.
- **`OS_MAX_ENTRIES = 128` was adequate headroom** for the 80 kHz /
  ±20 kHz / 100 Hz configuration. Any tightening of those parameters
  risks tripping fail — peak depth is inversely proportional to mod
  frequency and proportional to deviation amplitude.

## Reference artifacts

Saleae captures saved in `../captures/`:

- `../captures/LoadMetric-baseline/` — 20 kHz unmodulated, depth
  steady at 0.
- `../captures/LoadMetric-ride-80/` — 80 kHz carrier, ±20 kHz
  deviation, 100 Hz modulation — the data analyzed in Step 2 above.
