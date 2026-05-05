# EventOS — Design Tour

EventOS is a cooperative, event-driven operating system for
microcontrollers. It sits between a superloop and an RTOS: the
structure of a real OS — **actions**, **contexts**, a shared
**FIFO** — without preemption, priorities, or per-task stacks. An
action is behavior (a plain C function); a context is state (a block
of memory the OS hands out and reclaims on its own); EventOS
coordinates the two through a single FIFO queue drained in strict
order. The order your application posts work is the order it runs.
That determinism, plus a small footprint, is what EventOS gives you
in exchange for giving up preemption.

**Runtime contract.** Every feature in this document sits under these
four rules:

- **Run-to-completion.** Once an action starts, it runs until it
  returns. Nothing — no other action, no scheduler tick — can
  preempt it.
- **Strict FIFO order.** Actions run in the order they were posted.
  No priorities, no reordering.
- **ISRs are the only preemption.** An ISR can interrupt a running
  action (and yields back to the same action). Any state shared
  between an action and an ISR needs coordination (§1.8).
- **Actions must not block.** No busy-waits, no sleeps, no infinite
  loops. Long-running work breaks into re-enqueued steps (§2.2).

**When *not* to use EventOS.** EventOS is the wrong tool for:

- **Hard real-time at the core.** If your application's central
  loop must meet a software-scheduled deadline — an oscilloscope's
  capture pipeline, a motor controller at tens of kHz — EventOS
  gives up the priority-and-preemption machinery you'd need.
  Systems with real-time *services* but an event-driven core (a
  thermostat, a logger, a protocol stack) still fit; see §2.4.
- **Thread-shaped workflows.** If your application reads like
  `read(); parse(); validate(); write();` with blocking calls
  throughout, forcing it into an action-per-stage pipeline is a lot
  of rewriting for no gain.
- **Rich IPC surface.** EventOS has pub/sub for broadcast
  notification and a bulletin board for state handoff over time. If
  you want counting semaphores, blocking mutexes, mailboxes with
  timeouts, or a filesystem-like API, use an OS that provides them.
- **Vendor-backed support.** EventOS is small enough to read in an
  afternoon — which is the point — but if you'd rather point at a
  vendor when something breaks, FreeRTOS / Zephyr / ThreadX are
  proven and supported.

---

## 1. Features

<!-- The API surface, walked feature by feature. Each subsection uses
the same discipline we settled on for the bulletin board:
  * feature sentence (what it is, in one line)
  * API signatures with one-line ownership-framed descriptions
  * attributes (types, reserved values, edge cases, set semantics, etc.)
No patterns, no idioms, no implementation — those belong in §2 and §3. -->

### 1.1 Actions

An action is the unit of work in EventOS: behavior you write as a
plain C function, which EventOS calls on your behalf. You don't
invoke your own actions directly — you hand them to EventOS with
`os_Do` or other EventOS facilities, and EventOS runs them later.

**Shape:**

```c
void MyAction(os_context_t context);
```

**Attributes:**

- Run-to-completion and non-blocking per the preamble's runtime
  contract.
- **Borrowed context.** The context pointer is valid only while the
  action is running. Reading it after return is a use-after-free; see
  §1.2 for how to forward state across actions.
- **`os_NullAction`** — a built-in no-op action. EventOS APIs do not
  accept NULL as an action pointer; pass `os_NullAction` when the API
  requires an action but no work needs to happen (e.g. a
  bulletin-board timeout you don't need to be told about).

### 1.2 Contexts

A context is state that flows with behavior: a block of memory the
OS allocates, hands to an action as its argument, and reclaims
automatically when no holder is using it anymore. The application
fills the context in; the application never frees it.

**Types:**

- `os_context_t` — opaque pointer to context memory. Cast to a
  typed struct pointer on first use.
- `OS_NO_CONTEXT` — size sentinel (`= 0`) for "this action doesn't
  need state." Pass wherever a context size is expected when there's
  nothing to carry.

**Typing convention.** The OS knows nothing about what's in a
context. By convention, every action casts the pointer on its first
line:

```c
void MyAction(os_context_t context) {
    myState_t *s = (myState_t *)context;
    // ... use s ...
}
```

**Attributes:**

- **Allocated by other calls.** Every facility that needs a context
  allocates one for you. `os_Do(action, sizeof(state_t))` returns a
  pointer to a buffer for your state, ready to fill in; same shape
  for `os_DoAfter`, `os_Publish`, `os_Put`. The OS owns the buffer;
  you get a pointer to write into.
- **Caller-sized.** The caller specifies the context size at
  allocation. The allocator enforces an upper bound; exceeding it is
  a failure site (§3.2).
- **No free call.** The OS recycles a context when the last holder
  is done with it. The application never frees a context explicitly.
- **Forwarding via `*With`.** To keep a context alive past the
  action that received it, pass it to another action with one of the
  `*With` calls (`os_DoWith`, `os_DoAfterWith`, `os_PublishWith`,
  `os_PutWith`). These are the only paths that extend lifetime; any
  other way of stashing the pointer sidesteps the OS's bookkeeping.

**In practice:** treat the context pointer the way you'd treat a
stack address — valid while you're using it in the current call,
gone as soon as you return.

**Contexts vs globals.** Most feature state in a traditional
embedded program lives in module-level statics — always allocated,
always consuming RAM. Contexts let that state be live-only: it
exists while the feature is running, then the memory recycles. Peak
RAM becomes "working set" rather than "total feature surface."
Globals are still the right choice for configuration, calibration,
and genuinely persistent counters; everything else lives better as
a context.

### 1.3 Dispatch

Dispatch is how you schedule an action to run: immediately or after
a delay, with a fresh context or with one you already hold.

**API:**

```c
os_context_t os_Do         (os_action_t action, uint32_t contextSize);
void         os_DoWith     (os_action_t action, os_context_t context);
os_context_t os_DoAfter    (os_action_t action, uint32_t contextSize, uint32_t key, uint32_t ticks);
void         os_DoAfterWith(os_action_t action, os_context_t context, uint32_t key, uint32_t ticks);
```

- **`os_Do`** — allocate a context and post the action to the FIFO
  for execution in the future. Returns a new context for you to
  fill in.
- **`os_DoWith`** — post the action to the FIFO, forwarding an
  existing context.
- **`os_DoAfter`** — allocate a context and post the action to the
  timer queue; it promotes to the FIFO after `ticks`. Tagged with
  `key` for cancellation (§1.4). Returns a new context for you to
  fill in.
- **`os_DoAfterWith`** — same as `os_DoAfter`, forwarding an
  existing context.

**Attributes:**

- **Ticks unit.** The `ticks` argument is in OS ticks (not seconds).
  The tick period is platform-chosen (§3.6).
- **"No sooner than."** A delayed action fires no sooner than
  `ticks` ticks after scheduling. When the timer expires, the action
  joins the FIFO.
- **Delay zero.** `os_DoAfter` with `ticks == 0` takes the FIFO path
  immediately. For readability, use `os_Do` when you don't need a
  delay.
- **`key` is an application-chosen `uint32_t`.** Small enums,
  four-character tags, hashes — whatever reads well.
- **Keys form a set.** `os_DoAfter` and `os_DoAfterWith` index their
  pending actions by `key`: at most one pending action per key,
  across the timer queue and FIFO combined.
- **Re-using a key replaces.** Scheduling under a key that's already
  in the set replaces the existing pending action. The replaced
  action's claim on its context is released.
- **`OS_NO_KEY` opts out.** Pass `OS_NO_KEY` (= 0) when the action
  doesn't need to be cancelled. Such actions are not in the key set;
  multiple can coexist, and none can be targeted by
  `os_CancelPending`.
- **Failure.** A call that can't be scheduled (or, for the bare
  forms, can't allocate a context) invokes `os_Fail` (§3.2).

### 1.4 Cancellation

Cancellation withdraws a scheduled cancellable action before it
runs. Actions scheduled via `os_DoAfter` / `os_DoAfterWith` with a
non-`OS_NO_KEY` value form a set indexed by key (see §1.3);
cancellation looks up the key and removes the entry.

**API:**

```c
void os_CancelPending(uint32_t key);
```

- **`os_CancelPending`** — look up `key` in the cancellable-actions
  set: if present, remove it; if absent, do nothing. The removed
  action releases its claim on the context it was holding.

**Attributes:**

- **Only affects what's still pending.** An action that's already
  running, or has already returned, is past cancellation.
- **Shared namespace with bulletin board.** Bulletin-board keys
  (§1.6) live in the same `uint32_t` space. Pick distinct values if
  you use both — a successful `os_Get(k)` cancels the pending entry
  tagged `k`, which could un-schedule a user action tagged with the
  same numeric value.

### 1.5 Pub/Sub

Pub/Sub is many-to-many broadcast: publishers post to a topic; any
number of subscribers registered on that topic each receive the
published context.

**API:**

```c
void         os_Subscribe     (uint32_t topic, os_action_t action);
void         os_Unsubscribe   (uint32_t topic, os_action_t action);
void         os_UnsubscribeAll(uint32_t topic);
os_context_t os_Publish       (uint32_t topic, uint32_t contextSize);
void         os_PublishWith   (uint32_t topic, os_context_t context);
```

- **`os_Subscribe`** — register `action` as a subscriber on `topic`.
  Subscribing an already-subscribed action is a silent no-op.
- **`os_Unsubscribe`** — remove a single subscriber from a topic.
- **`os_UnsubscribeAll`** — remove every subscriber from a topic.
- **`os_Publish`** — allocate a fresh context and deliver it to
  every subscriber on `topic`. Returns a new context for you to
  fill in.
- **`os_PublishWith`** — deliver an existing context to every
  subscriber on `topic`, forwarding the context.

**Attributes:**

- **Topics are application-chosen `uint32_t`.** `OS_NO_KEY` (= 0) is
  reserved; don't use it as a topic.
- **Fan-out.** A single publish delivers to every subscriber — each
  subscriber gets its own action+context pair posted to the FIFO,
  all sharing the same context.
- **Fan-in.** An action can be subscribed to multiple topics.
- **Subscriber set is frozen at publish.** A publish delivers to the
  subscribers registered *at the moment `os_Publish` /
  `os_PublishWith` is called.* Subscribers added after the call
  don't receive that publish; subscribers removed after the call
  still get the delivery they were already enqueued for.
- **Decoupled sides.** Publishers and subscribers don't reference
  each other; adding or removing subscribers doesn't affect
  publishers.
- **Dynamic registration.** Subscribe and unsubscribe can be called
  at any time from any action. Runtime changes (telemetry toggles,
  diagnostic modes) are fully supported.
- **No subscribers, no delivery.** `os_Publish` to a topic with no
  registered subscribers still succeeds — you get a valid context to
  fill in, but nothing consumes it.
- **Sizing limit.** The system holds up to `OS_MAX_SUBSCRIPTIONS`
  total subscribers across all topics (default 8) — one slot per
  (topic, action) pair. Exceeding it is a failure site (§3.2).

### 1.6 Bulletin board

A bulletin board is a keyed, time-limited handoff: a poster drops a
context under a key, a getter takes it. If no getter takes it before
the timeout expires, the `timeoutAction` fires.

**API:**

```c
os_context_t os_Put    (uint32_t key, uint32_t contextSize, os_action_t timeoutAction, uint32_t ticks);
void         os_PutWith(uint32_t key, os_context_t context,  os_action_t timeoutAction, uint32_t ticks);
os_context_t os_Get    (uint32_t key);
```

- **`os_Put`** — allocate a context and park it on the bulletin
  board under `key`. Returns a new context for you to fill in; the
  bulletin board holds it under a TTL lease until a getter claims it
  or the lease expires.
- **`os_PutWith`** — park a context already in hand on the bulletin
  board under `key`. Ownership passes from the posting action to the
  bulletin board; same TTL lease.
- **`os_Get`** — take the context posted under `key` off the
  bulletin board. Ownership passes from the bulletin board to the
  getting action. Returns NULL when the key is not present in the
  bulletin board.

**Attributes:**

- **`key` is an application-chosen `uint32_t`.** `OS_NO_KEY` (= 0)
  is reserved.
- **Keys form a set.** At most one context per key. A second Put
  under a live key replaces the existing context; the replaced
  entry's timeout does not fire.
- **Shared namespace with cancellable dispatch.** Same `uint32_t`
  space as `os_DoAfter` cancel keys (§1.4) — pick distinct values if
  you use both.
- **TTL in ticks, starts at Put.** The `ticks` lease begins the
  moment `os_Put` / `os_PutWith` returns. Same unit as `os_DoAfter`.
- **Getter arrives in time.** `os_Get` returns the context and
  disarms the pending timeout.
- **TTL expires without a getter.** The `timeoutAction` is added to
  the FIFO, with the parked context as its argument.
- **`os_NullAction` for no notification.** Pass `os_NullAction` as
  the `timeoutAction` if you don't need to be told about expiry. The
  context is still reclaimed.
- **`ticks == 0` fires the `timeoutAction` immediately.** No entry
  is posted to the bulletin board — a lease with zero TTL is
  already expired, so `os_Put` / `os_PutWith` dispatch the
  `timeoutAction` directly (with the context) via the FIFO. Set
  semantics still apply: any existing entry under the same key is
  removed and its timeout does not fire. A subsequent `os_Get(key)`
  returns NULL because the board never held this entry.
- **Same-tick race is unspecified.** If `os_Get` and TTL expiry
  coincide on the same tick, whether the getter or the timeout wins
  is not defined.
- **Sizing limit.** Each live bulletin-board slot consumes one entry
  from the shared entry pool (`OS_MAX_ENTRIES`, default 32) —
  bulletin-board entries, FIFO entries, and timer-queue entries draw
  from the same pool. Running out is a failure site (§3.2).

### 1.7 The main loop

The main loop is where the application hands control to EventOS:
after one-time setup, the application sits in a tight loop calling
`os_Exec()`, which is the scheduler.

**Skeleton:**

```c
int main(void) {
    os_Init();
    // ... app setup: subscriptions, first posts ...
    while (1) {
        os_Exec();
    }
}
```

NOTE: HAL init (including `hal_CriticalInit`) runs before `os_Init`
in a real port; the skeleton above elides those steps. See
[examples/Blinky](examples/Blinky/main.c) for the full sequence.

**API:**

- **`os_Init()`** — initialize every OS pool and queue. Call once,
  before any other OS call.
- **`os_Exec()`** — pop the next action off the FIFO and run it,
  then return. Returns immediately if the FIFO is empty.
- **`os_Tick()`** — advance time by one tick. Invoked by the
  platform, not by the application.

**Attributes:**

- **Bounded per-call cost.** `os_Exec` runs one action (or returns
  immediately on an empty FIFO) and returns.
- **Only path to dispatch.** The loop is the only way actions run.
  If `main()` ever leaves the loop, dispatch stops.
- **Platform-chosen tick period.** See §3.6 for the porting
  contract.

### 1.8 Critical sections

Critical sections coordinate state shared between actions and ISRs.
Inside a critical section, an ISR cannot interrupt the action;
outside, it can. Use a critical section on the action side when
reading or writing state an ISR might also touch — action-to-action
state needs no coordination (the scheduler's FIFO order does that
job).

**API:**

```c
void hal_CriticalInit (void);
void hal_CriticalBegin(void);
void hal_CriticalEnd  (void);
```

- **`hal_CriticalInit`** — one-time HAL setup; called by the
  application during startup. Must run before any
  `hal_CriticalBegin` / `End` (including those that other HAL code
  may do before `os_Init`).
- **`hal_CriticalBegin`** — enter a critical section; no ISR can
  preempt until the matching `End`.
- **`hal_CriticalEnd`** — exit. Every `Begin` must be paired with
  exactly one `End`.

**Attributes:**

- **Action-vs-ISR only.** Critical sections are only needed where an
  action and an ISR touch the same state. Action-to-action sharing
  is serialized by RTC ordering and needs no lock.
- **Nesting is supported.** `Begin`/`End` pairs nest; only the
  outermost pair actually coordinates with the hardware. This lets
  library code wrap its own state without the caller needing to
  know.
- **Keep them short.** A long critical section holds ISRs off for
  its duration, defeating the point of a fast scheduler. Do the
  minimum: read or write the shared state, exit.
- **Platform-supplied.** The HAL implementation is part of the
  port — disable/enable interrupts on Cortex-M, no-op on POSIX, a
  recursive mutex on a threaded port. See §3.6.

---

## 2. Patterns

EventOS's public API is small and deliberate — a handful of
`os_Do*` variants, pub/sub, and a bulletin board. Complex behaviors
come from composing these primitives, not from adding new ones. A
chain of actions becomes a "thread." A self-re-enqueuing action
becomes an iteration loop. A cancellable delayed action becomes a
debouncer, a timeout, or a rate-switcher. The patterns below are a
few worth naming; each application will find its own. Most are
anchored by a runnable example in `examples/`.

### 2.1 Action chains ("threads")

Many pieces of work have several stages that must run in order —
parse, validate, write. In EventOS you compose them as a chain of
actions: each stage does its piece, schedules the next stage with
`os_DoWith` (or `os_DoAfterWith` for a delayed stage), and returns.
A shared context carries state from one stage to the next, written
verbatim by one stage and read by the next.

We call this composition a **thread** — not a thread in the OS
sense (no stack, no preemption), but in the woven-through-the-FIFO
sense: one logical task walking several actions while other threads'
stages interleave around it. Multiple threads of the same shape can
be in flight at once, each with its own context.

The pattern is the natural answer to "how do I do sequential work
without blocking?" The RTC rule forbids a stage from waiting; the
chain breaks the work into non-blocking stages and lets the FIFO do
the handoff.

**Example:** [examples/Thread](examples/Thread/main.c) — a Stage1 →
Stage2 → Stage3 chain spawned per debounced button press, running
concurrently with a heartbeat blinker.

### 2.2 Iteration

A common objection to event-driven systems is "you can't loop in an
action." The iteration pattern is the answer: the action does one
step of work, re-enqueues itself (or a successor) with its loop
state carried in the context, and returns. The FIFO promotes the
re-enqueued action when its turn comes; other work interleaves
between iterations. The overall effect is a loop that yields the
CPU on every step.

Loop state — an index, a cursor, a remaining count — lives in the
context. One iteration writes to it; the next reads. Forwarding the
context via `os_DoWith` or `os_DoAfterWith` is how the state passes
from one iteration to the next.

Heartbeat / Blinky is the degenerate one-step form: toggle an LED,
post yourself again with a delay, return. No visible loop state,
but the same composition.

**Example:** [examples/Morse](examples/Morse/main.c) — two nested
iteration chains emit "HELLO WORLD " in Morse code on LED_0 while a
heartbeat on LED keeps blinking every 500 ms. Visible proof that the
iteration doesn't block.

### 2.3 ISR → action handoff

An ISR should be short. Any real work — parsing, state-machine
advances, reactions — belongs in cooperative context where the full
OS API is available. The ISR's job is the minimum: capture the
volatile hardware state that's about to be lost, post an action to
handle it, and return. `os_Do`, `os_DoAfter`, `os_Publish`, and
`os_Put` are all callable from an ISR. The `*With` variants are
not — they forward an existing context, and an ISR isn't inside an
action, so there's no context in hand to forward.

Most of what an embedded application does is downstream of
ISR-initiated actions. A timer ISR, a peripheral ISR, a sensor-ready
line — these are the starts of the threads, chains, and publishes
that make up the system's behavior. The ISR → action handoff is how
the outside world enters the FIFO.

The posted action runs later in cooperative context, where it can
publish, chain into a thread, update the bulletin board, and call
the OS freely.

**Shared state with the ISR.** Any state an action reads or writes
that the ISR also touches — a ring buffer, a sample queue, a flag —
must be accessed inside a critical section on the action side
(§1.8). In practice this lives in HAL and driver code (UART write,
ring-buffer push) rather than application actions: the application
calls the HAL and the HAL handles the coordination. The action-side
burden comes from the asymmetric preemption — the ISR can preempt
the action, not the other way around.

**Example:** [examples/Button](examples/Button/main.c) — a
button-press ISR calls `os_Do` to post a handler that toggles an
LED, then returns. See also
[examples/Debounce](examples/Debounce/main.c) for the
cancellable-timer twist (§2.5).

### 2.4 Hard timing at the edge

A common objection to EventOS is "but I have hard real-time
requirements — I need to sample an ADC at 1 kHz, I need a control
loop, I need to stream UART without dropping bytes." Fair concerns,
but the question isn't whether you have real-time constraints
(almost every embedded system does). The question is whether the
*OS* has to meet them.

Often, it doesn't. A timer peripheral triggers the ADC on a hardware
clock; a DMA channel drops samples into a ring buffer with zero CPU
involvement; an ADC-ready ISR captures the batch and moves on in a
handful of instructions. The hard-real-time path lives entirely in
hardware and a short ISR. EventOS doesn't need to be on that path at
all — it picks up when the ISR hands off a buffer, and from there
runs at the pace of cooperative actions.

**Worked example.** Sample an ADC at 1 kHz, low-pass-filter the
samples, drive an actuator from the filtered value (a simple closed
loop), and also publish measurement and alert packets to
subscribers.

- **Hardware:** timer triggers ADC conversions at 1 kHz.
- **ISR:** on ADC-ready (or DMA-half-full), capture the batch,
  `os_Publish(TOPIC_SAMPLES, batch)` and return.
- **Control action** (subscriber on `TOPIC_SAMPLES`): run the LPF
  across the batch, update the moving average, compute the next
  actuator setpoint (PID or simpler), drive the output. Runs once
  per batch.
- **Telemetry action** (independent, scheduled every 2 s via
  `os_DoAfter`): read the accumulated state, publish
  `TOPIC_MEASUREMENT`, and `os_Publish(TOPIC_ALERT, ...)` on
  threshold crossing.

The 1 kHz sampling is deterministic because the *hardware*
guarantees it. The control loop runs once per sample-batch and
closes at whatever cadence the hardware produces — deterministic
enough for thermal, flow, pressure, or any process whose physical
time constants are comfortably slower than the sampling rate.
EventOS guarantees samples are processed in arrival order and never
dropped, which is what the DSP needs.

**Where this stops working.** When the control loop itself needs to
close inside the ISR (tens of kHz control rates, tight phase margins
on a motor drive, an oscilloscope's acquisition pipeline), the loop
is no longer "above" EventOS — it *is* the application, and it
needs bare metal or an RTOS that can schedule the loop itself with
priority. The distinguishing question is whether the tight timing
is a *service* the application uses, or the core of the application
itself.

### 2.5 Cancellable delayed action

Many embedded problems have the shape "do X in N ticks, unless
something else happens first." Software debounce, protocol timeouts,
inactivity watchdogs, rate controls — they all use one primitive: a
delayed action scheduled under a cancel key.

```c
os_DoAfter(HandleTimeout, ..., MY_KEY, DURATION);
// ... later, if circumstances change ...
os_CancelPending(MY_KEY);
```

The set-per-key semantics (§1.3) give you two shapes for the price
of one primitive:

- **Cancel outright.** Call `os_CancelPending(key)` when the
  condition you were waiting for has resolved and no further action
  is needed. The pending action is removed; nothing fires.
- **Cancel and replace.** Re-post with the same key — the old
  pending action is replaced automatically. Useful for "reset the
  timer" idioms: every time a new event arrives, re-post, so the
  action only fires when events stop arriving.

**Examples:**

- [examples/Debounce](examples/Debounce/main.c) — each button-edge
  ISR re-posts the debounce action, so only the last edge (with no
  follow-up) fires.
- [examples/CancelPending](examples/CancelPending/main.c) — a
  controller swaps the blink rate by cancelling the current pending
  blink and starting a new one.
- **Protocol timeout** (not in the repo, but a natural fit): send a
  request, post a timeout action with a cancel key, cancel on
  response; otherwise the timeout runs and you handle the failure.

### 2.6 Queue depth as the load metric

A natural first question from an RTOS background is "what's the CPU
utilization?" That's the wrong number for a cooperative RTC system.
The right number is **FIFO depth over time** — how many actions are
waiting to run at any moment.

The question FIFO depth answers is "is the system keeping up?"
Actions run ASAP when the FIFO gets to them; there's no schedule,
no deadline, no response window. Latency from post to execution is
whatever falls out of the queue. Little's Law makes that concrete:
average depth × average action runtime ≈ average latency. If depth
trends up, latency trends up with it — the system is taking work in
faster than it can drain.

An idle-time percentage tells you nothing about this: a 90% idle
system with a bursty arrival rate can have a deep FIFO and high
latency; a fully busy system draining steadily can have a shallow
FIFO and low latency. Depth, not utilization, is the signal.

**How to sample.** The entry-pool counters track the live count
across the FIFO plus the timer queue:

- `os_EntryInUse()` — current live entry count.
- `os_EntryHighWater()` — running maximum since boot.
- `os_EntryHighWaterReset()` — clears the high-water mark to zero,
  starting a fresh measurement window.

Timer-queue entries (including bulletin-board timeouts) aren't
waiting on the CPU — they're waiting on time — so they don't
contribute to pressure in the Little's-Law sense. In practice their
count is stable at a given workload, so the combined number tracks
FIFO growth closely. When it climbs unexpectedly, it's the FIFO.

If you need a pure FIFO number (lots of long-TTL bulletin-board
entries distorting the aggregate), instrument via the
`os_ActionBegin` / `os_ActionEnd` hooks (§3.5) or maintain an
application-side counter.

**Alerting.** Track `os_EntryHighWater()` and alert as it approaches
`OS_MAX_ENTRIES`. Either the arrival rate has grown past the pool's
sizing or actions are taking longer than expected to drain — both
are worth catching before the pool exhausts and `os_Fail` trips.

---

## 3. How it works

<!-- Implementation, internals, and anything a port author or deep
debugger needs. Reader-facing features don't appear here except as
forward references. -->

### 3.1 Concurrency contract (details)

§1.8 gives the user-facing rules; this section covers the mechanism
behind them.

**Why action-to-action needs no coordination.** EventOS has a
single-threaded execution model above the hardware: `os_Exec()` pops
one entry from the FIFO and runs one action to completion. No
context switch happens mid-action; no other action can observe
partial state. A shared variable read in one action cannot race
with a write in another, because there is no overlap in which to
race. The scheduler serializes them by construction.

**Why action-vs-ISR needs critical sections.** ISRs are the
asymmetry: an action can be interrupted by an ISR, but an ISR
cannot be interrupted by an action. Any state both touch needs to
be read and written as a single atomic sequence from the action's
point of view. `hal_CriticalBegin()` on a bare-metal port disables
the ISR's ability to fire until the matching `hal_CriticalEnd()`;
inside that bracket, the action can read-modify-write in safety.

**Nesting.** The HAL tracks nest depth. `hal_CriticalBegin`
unconditionally disables interrupts, then increments a counter;
disabling already-disabled interrupts is a hardware no-op, so the
outermost call is the one that actually takes effect.
`hal_CriticalEnd` decrements the counter; if the counter returns to
zero, it re-enables. Middle levels only adjust the counter. This
lets a function that uses critical sections internally be called
from within an outer critical section — the nest reads as one
coordinated region from the hardware's point of view.

**Per-port mechanism examples.** Two reference ports are shipped:

- **Cortex-M** (`hal/Critical.c` in the bluepill port):
  `__disable_irq()` / `__enable_irq()`, guarded by a `uint32_t` nest
  count. All maskable interrupts are held off — priority-aware
  masking (via `BASEPRI`) would require a different HAL
  implementation.
- **POSIX** (`examples/posix/source/Critical.c`): no-op. The shipped
  demo is single-threaded with no ISRs; there's nothing to
  coordinate. A threaded variant of this port is also supported by
  the HAL shape — a recursive `pthread_mutex_t` takes the place of
  the disable/enable pair with the same nest counting. Not shipped
  in the repo, but the contract accommodates it (§3.6).

**The OS's own use.** EventOS internally wraps access to shared
state — FIFO head/tail, timer queue, pool free lists — in critical
sections because ISRs may call `os_Do`,
`os_DoAfter`, `os_Publish`, and `os_Put` directly (§2.3).
Application and OS critical sections share the same HAL; a long
application-side critical section directly delays the OS's internal
coordination and therefore every post happening from any ISR.

**Keep them short.** A critical section that runs for 10 µs holds
ISRs off for 10 µs. On a system with an ISR that fires every 20 µs
(a 50 kHz sample stream), that's half a sample period lost. The
budget is the physical tolerance of the hardest-real-time ISR; plan
accordingly.

### 3.2 Failure handling

When EventOS runs out of a resource it needs — a context slot, an
entry slot, a subscriber slot, a bulletin-board entry — the
operation you asked for cannot complete. Rather than silently
corrupt state or return a hard-to-interpret error, EventOS calls a
user-provided handler:

```c
void os_Fail(os_fail_t reason);
```

`os_Fail` is the one correctness-critical hook the application is
expected to supply. Each shipped platform provides a weak default
that halts the system (`while (1);` on the bluepill, `exit(1)` on
POSIX); override it with whatever fits your target — log and reset,
drive an error-pattern LED, write a post-mortem snapshot then
reboot.

**`os_Fail` does not return.** Treat it as a terminator. The OS
reached `os_Fail` because an internal update could not complete;
lists may be half-spliced, counters may be stale, reference counts
may not balance. Whatever diagnostic, logging, or shutdown work the
application needs happens inside `os_Fail`; when that work is done,
reset, loop, or jump to the bootloader. Never return to the OS.

**Failure codes.** The `os_fail_t` enum in `os/os.h` has one
distinct reason per failure site — `OS_FAIL_DO_ALLOCATION`,
`OS_FAIL_PUT_WITH_ALLOCATION`, `OS_FAIL_SUBSCRIBE_ALLOCATION`, and so on.
A handler that logs the code can pinpoint exactly which request ran
out of room.

### 3.3 Internal pools

EventOS allocates nothing from a heap at run time, unless you opt
into the malloc-backed context allocator below. Every dynamic
object comes from a fixed-size pool sized at compile time. Three
pools cover the whole system, ordered here by how visible they are
to the application: the **context pool**, the **entry pool**, and
the **subscription pool**.

Sizing is the application's responsibility. Shipped defaults are
small so a misconfiguration fails fast rather than quietly wasting
RAM; real applications size each pool against measured peak load.
Every parameter named below is overridable via `os_config.h` or a
compile-line `-D`; see §3.4 for the mechanism. Every pool allocator
runs inside a critical section — ISRs may allocate (§2.3), so the
free-list mutators have to be atomic with respect to the ISRs that
call them.

**Context pool.** The context is the one dynamic object an
application touches directly — every `os_Do` / `os_Publish` /
`os_Put` return value is storage served by the context allocator —
so this pool gets the most attention. The allocator is a link-time
seam, not a fixed pool. The OS declares three functions in
`os_p.h`:

```c
void      os_CtxAllocInit(void);
os_ctx_t *os_CtxAlloc(uint32_t size);
void      os_CtxFree(os_ctx_t *ctx);
```

and leaves the implementation to be selected at link time. Two
implementations ship:

- **`ctxAllocMalloc`** — a thin wrapper over `malloc` / `free`. The
  default. Appropriate when you already have a heap, don't want to
  pre-size, and the non-determinism of a general-purpose allocator
  is acceptable.
- **`ctxAllocPool`** — two fixed-size buckets, small and large, for
  bimodal context sizing (most actions carry a few words; a handful
  carry a longer payload). `OS_CTX_SMALL_SLOT` /
  `OS_CTX_SMALL_COUNT` and `OS_CTX_LARGE_SLOT` /
  `OS_CTX_LARGE_COUNT` size them. A request ≤ small-slot goes to
  the small bucket; otherwise a request ≤ large-slot goes to the
  large bucket; anything larger returns NULL and trips
  `OS_FAIL_CONTEXT_ALLOCATION`. Setting either `_COUNT` to 0 elides
  that bucket — a small-only or large-only pool.

*Why a link-time seam rather than a `#define` or a function
pointer?* Direct calls, no indirection overhead; only the chosen
implementation is linked, so dead code doesn't ride along; and if
you pick `ctxAllocPool`, `malloc` and `free` aren't referenced at
all — on targets that don't ship a libc heap, that's the difference
between "links" and "doesn't."

*Where the three functions plug in.* The application never calls
the allocator directly. `os_Init()` calls `os_CtxAllocInit()` as
part of its cascade; `os_ContextNew` (backing every `os_Do` /
`os_DoAfter` / `os_Publish` / `os_Put`) calls `os_CtxAlloc`;
`os_ContextRelease` (driven by refcount) calls `os_CtxFree`. An
allocator only has to satisfy the three signatures correctly — the
OS decides when each runs.

*The allocator contract.* `os_CtxAlloc(size)` returns a pointer
suitable to be treated as `os_ctx_t *` (32-bit-aligned storage of
at least `sizeof(os_ctx_t) + size` bytes), or NULL on exhaustion —
the OS converts NULL into `os_Fail(OS_FAIL_CONTEXT_ALLOCATION)`.
`os_CtxFree(ctx)` returns a slot to the pool. The allocator does
not manage refcounts or any of the `os_ctx_t` fields — that's the
OS's job; the allocator is just handing out storage.

*Bring your own.* Drop `myCtxAlloc.c` into the build that defines
the three functions and build with `make CTX_ALLOC=myCtxAlloc`. The
shipped example Makefiles glob-exclude all `ctxAlloc*.c` from the
OS source list and then add back the single file named by
`CTX_ALLOC`:

```make
OS_SRCS := $(filter-out $(wildcard .../mem/ctxAlloc*.c), $(SHIPPED))
OS_SRCS += .../mem/$(CTX_ALLOC).c
```

so only the one implementation you asked for ends up in the link.
Default is `ctxAllocMalloc`.

*Observability.* `os_CtxInUse(bucket)`, `os_CtxHighWater(bucket)`,
and `os_CtxHighWaterReset(bucket)` expose live and peak allocation
per bucket when `ctxAllocPool` is linked, where `bucket` is one of
`OS_CTX_BUCKET_SMALL` or `OS_CTX_BUCKET_LARGE`. The buckets are
sized independently (`OS_CTX_SMALL_COUNT` vs `OS_CTX_LARGE_COUNT`)
and exhaust independently, so the alert threshold is per-bucket;
sum the two if you want a combined view. With `ctxAllocMalloc` the
counters return zero for either bucket and the reset is a no-op —
there is no inherent slot count to measure against, so peak
allocation has to come from the heap implementation rather than
from EventOS.

**Entry pool.** An entry is the OS-internal slot that carries an
action + context + key through the system. The application never
holds one, but it sets the pool size and reads the occupancy
counters. Every FIFO position, every timer-queue position, every
bulletin-board posting, and every in-flight pub/sub delivery
consumes one entry. One pool services all callers.

- Sized by `OS_MAX_ENTRIES` (default 32).
- Allocation failures call `os_Fail` with per-site codes in
  `os_fail_t` — `OS_FAIL_DO_ALLOCATION`,
  `OS_FAIL_DO_AFTER_ALLOCATION`, `OS_FAIL_PUT_ALLOCATION`, etc.
  (see `os.h`). `os_Publish` layers on top of `os_Do`, so publish
  failures surface as `OS_FAIL_DO_ALLOCATION`.
- Observability: `os_EntryInUse()`, `os_EntryHighWater()`, and
  `os_EntryHighWaterReset()` are public; §2.6 covers load-metric
  usage.

**Subscription pool.** Fully internal — the application calls
`os_Subscribe` / `os_Unsubscribe` and never sees a subscription
object. A subscription is the link between a topic and a subscribed
action; one slot per (topic, action) pair; topics have no per-topic
cap.

- Sized by `OS_MAX_SUBSCRIPTIONS` (default 8).
- Allocation failure: `OS_FAIL_SUBSCRIBE_ALLOCATION`.
- Observability: `os_SubInUse()`, `os_SubHighWater()`, and
  `os_SubHighWaterReset()` are public — same telemetry shape as the
  entry pool.

### 3.4 Configuration

EventOS's compile-time parameters share one override mechanism with a
three-tier precedence:

1. **Compile-line `-D<parameter> = <value>`** wins over everything.
2. **`os_config.h`** — an application-supplied header — overrides
   built-in defaults but yields to any compile-line `-D`.
3. **Built-in defaults** in the OS source apply when neither of
   the above set the parameter.

**Setting up `os_config.h`.** A commented template ships at
`app/include/os/os_config.h.template`:

1. Copy it into your application's include path as `os_config.h`
   (drop the `.template` suffix).
2. Uncomment the parameters you want to change and edit their values.
   Anything you leave commented keeps the default.
3. Build with `-DOS_USE_CONFIG_H`. When the flag is set the OS
   headers include your `os_config.h`; when it isn't, the file is
   ignored entirely — existing applications that don't configure
   anything keep working.

Every parameter — in both the template and the OS headers — is wrapped
in `#ifndef`, so a compile-line `-D<parameter> = <value>` still wins over
`os_config.h`. That matters for quick experiments (change the
entry-pool size for one debug build without editing the header)
and for per-target variants in a multi-target build.

**The compile-time parameters.**

- **`OS_TICKS_PER_SECOND`** — tick frequency in Hz; §3.6.
- **`OS_MAX_ENTRIES`** (default 32) — entry pool; §3.3.
- **`OS_MAX_SUBSCRIPTIONS`** (default 8) — subscription pool; §3.3.
- **`OS_CTX_SMALL_SLOT`** / **`OS_CTX_SMALL_COUNT`** /
  **`OS_CTX_LARGE_SLOT`** / **`OS_CTX_LARGE_COUNT`** — context
  pool bucket sizes (`ctxAllocPool` only); §3.3.

**Build-level selection: `CTX_ALLOC`.** The context allocator is
the one exception to the `-D` story — `CTX_ALLOC` is a make
variable that selects *which source file is linked*, not a
preprocessor macro. `make CTX_ALLOC=ctxAllocPool` swaps in the
pool allocator; the default is `ctxAllocMalloc`. Bring-your-own
allocators follow the same mechanism (§3.3). This lives in the
Makefile because source-file selection is a link-time decision,
not a preprocessor one.

### 3.5 Observability hooks

EventOS exposes two weak-linked hooks that `os_Exec` calls around
every action dispatch:

```c
void os_ActionBegin(os_action_t action);
void os_ActionEnd(os_action_t action);
```

Each platform port ships a weak no-op definition. An application
overrides one or both by defining a non-weak version in its own
source; the linker picks the strong symbol and the no-op drops out.
Intended use: per-action runtime measurement (start/stop a cycle
counter), lightweight tracing, coverage logging, or live-depth
displays like the [LoadMetric
example](examples/LoadMetric/main.c).

**Where in the dispatch.** Inside `os_Exec`, the hooks bracket the
action call exactly, nothing else:

```c
os_ActionBegin(fifoEntry->action);
fifoEntry->action(ctx != NULL ? ctx->data : NULL);
os_ActionEnd(fifoEntry->action);
```

Context release and entry free happen *after* `os_ActionEnd`
returns — the entry is still in use when the hook runs, which is
why `os_ActionEnd` can read `os_EntryInUse()` and subtract 1 to
report the depth pending behind the just-finished action
(`examples/LoadMetric/main.c`).

**Discipline: treat a hook like an ISR body.** Hooks run on the
main loop, not in an ISR, but the practical constraint is the
same: whatever they do adds directly to the gap between actions. No action is running while a hook is running, and ISRs
that post during a hook sit on their interrupt-entry latency until
the hook returns. Keep hook bodies to O(1) work — a register read,
a GPIO toggle, a circular-buffer append. Anything heavier
(formatting, I/O, arithmetic over arrays) belongs in an action the
hook posts, not in the hook itself.

**Why the hooks see only the action pointer.** The action function
pointer identifies the *function*; the context identifies one
specific *invocation*. Instrumentation almost always wants the
former — per-function time, per-function trace, per-function
coverage. Exposing the context would couple instrumentation to
per-call memory ownership (the context is still refcounted and is
about to be released when the hook returns) and would invite a
hook to reach into opaque application data. A hook that genuinely
needs per-call state can key a side-table by the action pointer;
the function identity is enough.

### 3.6 Porting

A port brings up the target and fulfills a small set of contracts.
The OS itself is platform-agnostic C99 — everything target-specific
lives in the port. The shipped ports are compact: the bluepill port
is eight files (startup, clock, SysTick, critical section, LED,
button, newlib syscalls, init glue); the POSIX demo is three.

**What the port must provide:**

1. **Critical sections.** `hal/Critical.h` declares three
   functions:

   ```c
   void hal_CriticalInit(void);
   void hal_CriticalBegin(void);
   void hal_CriticalEnd(void);
   ```

   They must support nesting — inner `Begin`/`End` pairs must not
   re-enable interrupts mid-region. The bluepill port uses
   `__disable_irq` / `__enable_irq` with a nest counter
   (`examples/bluepill/source/Critical.c`, ~30 lines). The POSIX
   port ships a no-op implementation; its docstring spells out the
   limitation (action-vs-ISR races won't reproduce) and points at
   `pthread_mutex` as the faithful alternative.

2. **A tick source.** The port must call `os_Tick()` at the rate
   the application expects. Bluepill configures the Cortex-M
   SysTick timer at the lowest priority; the handler is just:

   ```c
   void SysTick_Handler(void) {
       os_Tick();
   }
   ```

   POSIX polls `CLOCK_MONOTONIC` from its main-loop helper
   `posix_Tick()` and calls `os_Tick()` once per elapsed
   millisecond, catching up if multiple ticks passed since the last
   call. The OS doesn't care how `os_Tick` is driven — ISR,
   cooperative poll, separate thread — as long as the long-term
   rate is correct.

3. **An `os_Fail` default.** The OS calls `os_Fail(os_fail_t
   reason)` when a resource runs out (§3.2). Each port ships a
   weak default — `while (1);` on the bluepill, `fprintf(stderr,
   ...); exit(1);` on POSIX — so an unconfigured application gets a
   safe halt. Applications override the weak symbol with whatever
   policy they want; the weak stub just ensures the link succeeds.

**What the port may provide.** The observability hooks
`os_ActionBegin` / `os_ActionEnd` (§3.5) are weak no-ops in both
shipped ports. They aren't required to bring a port up, but having
the port ship the weak stubs means applications can override either
one in isolation without every port remembering to provide the
default.

**Reference ports.**

- **`examples/bluepill/`** — STM32F103 (Cortex-M3), bare-metal.
  Startup assembly, vector table, clock bring-up (HSI → 64 MHz),
  SysTick driver, GPIO LED and button drivers, newlib syscalls.
  This is the real target; all feature verification runs here (see
  `examples/captures/` for the Saleae captures).
- **`examples/posix/`** — host, single-threaded. Monotonic-clock
  tick polling, stdout LED pseudo-driver, no ISR simulation.
  Useful for running demos on a workstation without flashing a
  board, with the caveat that action-vs-ISR concurrency is not
  exercised.

**A more complete POSIX port.** The shipped POSIX port is
deliberately minimal: `os_Tick` runs inline from the main loop,
critical sections are no-ops, and action-vs-ISR concurrency is
unexercised (§3.1's caveat). A faithful host port closes those
gaps by running `os_Tick` on a dedicated thread —
`timer_create` with `SIGEV_THREAD`, or any periodic source — and
implementing `hal_CriticalBegin` / `End` with a recursive
`pthread_mutex_t` to coordinate between the tick thread and the
main loop. The HAL contract supports it directly; the shipped
`examples/posix/source/Critical.c` docstring is the starting point
if you want to build it.

### 3.7 Footprint

The canonical footprint measurement is the `Demo` example — it
exercises every feature surface (Do / DoAfter / CancelPending /
PubSub / bulletin board) in one image, which is a reasonable "all
features on" upper bound for a real application. Measured on
STM32F103 (Cortex-M3) with `arm-none-eabi-gcc` 14.2.1 at `-Os`:

| Build                      | text (flash) | data (RAM) | bss (RAM) |  total |
|----------------------------|-------------:|-----------:|----------:|-------:|
| default (`ctxAllocMalloc`) |       3880 B |       92 B |    1156 B | 5128 B |
| `ctxAllocPool`             |       3516 B |       12 B |    1160 B | 4688 B |

On an STM32F103C8 (128 KB flash, 20 KB RAM), the malloc build takes
roughly 3% of flash and 6% of RAM. The pool build takes less flash
(no `malloc` / `free` pulled from newlib) and essentially the same
RAM.

**What lives in each segment.**

- **text** — OS code (dispatch, timer queue, pub/sub, bulletin
  board, entry pool, context allocator) plus the Demo application
  and the bluepill port (startup, clock, SysTick, GPIO).
- **data** — initialized globals. Most OS state starts at zero and
  lives in bss; data is mostly newlib init tables. The pool
  variant's smaller data reflects not pulling malloc's tracking
  state into the image.
- **bss** — uninitialized globals. The largest contributors are
  the entry pool (`OS_MAX_ENTRIES` × `sizeof(os_entry_t)` = 32 ×
  20 = 640 B at default) and, for the pool allocator, the context
  buckets (`OS_CTX_SMALL_COUNT` × `OS_CTX_SMALL_SLOT`
  + `OS_CTX_LARGE_COUNT` × `OS_CTX_LARGE_SLOT` = 4×16 + 4×64 =
  320 B at default).

**What changes the numbers.** Pool sizes (§3.4) drive bss linearly:
doubling `OS_MAX_ENTRIES` from 32 to 64 adds 640 bytes. Text is
dominated by the feature set the application actually exercises —
with `-ffunction-sections` and `-Wl,--gc-sections` (both shipped in
the Demo Makefile), unused OS functions are linker-pruned, so a
Blinky-only application lands well under these numbers.

**Rule of thumb.** "Does EventOS fit?" is rarely the interesting
question on Cortex-M — the answer is yes, with margin, from
STM32F0 upward. The meaningful footprint conversations are about
pool sizing (peak live actions, peak subscriptions, peak contexts)
rather than OS code size.

---

<!-- Open work items tracked here until the sections absorb them:

- Same-tick race: Get and TTL expiry on the same tick has unspecified
  ordering. Mention once, in §1.6, and leave it unspecified.
- Audit of implementation complexity vs feature set is a separate
  activity; the sections above describe the *intended* contract, and
  if the audit later simplifies the code the contract doesn't
  change. Note that concrete API names — `os_EntryInUse`,
  `os_EntryHighWater`, the entry-pool framing in §2.6, etc. — are
  the places where a post-audit rename is most likely to show up.
  The *capability* (reliable, actionable load data from the OS) is
  what §2.6 commits to, not the specific spellings.
- Consider elevating "most behavior is downstream of ISR-initiated
  actions" (currently in §2.3) to the document preamble or §2
  preamble when we write those. It's a foundational framing point —
  shapes how a reader thinks about every pattern, not just
  ISR-handoff.
-->
