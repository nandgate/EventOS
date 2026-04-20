# EventOS — Design Tour

This is the long-form guide to EventOS. The [README](readme.md) is the
one-page pitch and the [examples](examples/) are runnable code for every
feature; this document is the model underneath both.

Read it front to back if you're evaluating EventOS against an RTOS or getting
your first event-driven system working. Skim for the section titles if you're
already fluent in event-driven design and only need to see what EventOS's
specific vocabulary is.

---
<!-- TOC -->
* [EventOS — Design Tour](#eventos--design-tour)
  * [Core concepts](#core-concepts)
    * [Actions](#actions)
    * [Contexts](#contexts)
    * [Typing the context](#typing-the-context)
    * [Context lifecycle](#context-lifecycle)
    * [The FIFO (ready queue)](#the-fifo-ready-queue)
  * [The main loop](#the-main-loop)
  * [Time](#time)
    * [ASAP (As Soon As Possible): `os_Do` and `os_DoWith`](#asap-as-soon-as-possible-os_do-and-os_dowith)
    * [Delayed: `os_DoAfter` and `os_DoAfterWith`](#delayed-os_doafter-and-os_doafterwith)
  * [Patterns](#patterns)
    * [Action chains ("threads")](#action-chains-threads)
    * [Iteration — long-running work without loops](#iteration--long-running-work-without-loops)
    * [Pub/Sub](#pubsub)
    * [The blackboard](#the-blackboard)
    * [Cancellable work](#cancellable-work)
    * [ISR → action handoff](#isr--action-handoff)
  * [Concurrency model](#concurrency-model)
    * [Action vs action: no races](#action-vs-action-no-races)
    * [Action vs ISR: critical sections](#action-vs-isr-critical-sections)
    * [Why this is simpler than an RTOS](#why-this-is-simpler-than-an-rtos)
  * [Performance and observability](#performance-and-observability)
    * [The load metric is queue depth, not idle time](#the-load-metric-is-queue-depth-not-idle-time)
    * [Instrumentation hooks](#instrumentation-hooks)
  * [Failure handling](#failure-handling)
  * [Internal pools](#internal-pools)
    * [Entry pool](#entry-pool)
    * [Subscription pool](#subscription-pool)
    * [Blackboard pool](#blackboard-pool)
    * [Context pool](#context-pool)
  * [Porting](#porting)
    * [1. The HAL](#1-the-hal)
    * [2. The tick source](#2-the-tick-source)
    * [3. Failure](#3-failure)
  * [Footprint](#footprint)
  * [When *not* to use EventOS](#when-not-to-use-eventos)
<!-- TOC -->

## Core concepts

Three data structures carry the whole system: **actions**, **contexts**, and
the **FIFO**.

### Actions

An action is a C function with this shape:

```c
void MyAction(os_context_t context);
```

That's the unit of work EventOS schedules and runs. Any piece of your
application's behavior — toggling an LED, parsing a packet, advancing a state
machine — lives inside an action.

Actions have three rules:

1. **They run to completion.** Once an action starts executing, no other
   action can preempt it. The only thing that can interrupt it is a hardware
   ISR, and the ISR yields back to the same action it interrupted.
2. **They don't block.** No infinite loops, no busy-waits on peripherals, no
   sleeps. If a piece of work takes a long time, break it into steps and
   re-enqueue the next step through the OS (see the iteration pattern below).
3. **They never retain their context past return.** The context pointer
   passed to an action is valid only for the duration of that call. Stashing
   it in a global, a static, or any other action's captured state is a
   use-after-free waiting to happen — see [Contexts](#contexts) below for
   why, and for the OS-aware handoffs that let you share state safely.

The first two rules are what let EventOS drop most of what an RTOS has to do:
there are no stacks to context-switch, no priorities to balance, no preemption
points to guard against. The third rule is how the context allocator can
recycle memory without tracking anything the application does on its own.

One action is built-in: `os_NullAction`, a public no-op. EventOS APIs
never accept NULL as an action pointer — pass `os_NullAction` anywhere
the API requires an action pointer but you don't actually need work done.

### Contexts

A context is a small block of memory that the OS manages and passes to an
action when it runs. It's how an action gets input, carries state across
steps, and communicates with the next action in a chain.

- You ask for a context when you schedule work (`os_Do(MyAction, sizeof(state_t))`)
  and the OS hands you back a pointer you can fill in.
- When the action runs, it receives that pointer as its parameter.
- When every consumer of the context has returned, the OS automatically frees
  it — no explicit free. Contexts are reference-counted under the hood.

You can pass the same context to several actions (`os_DoWith`,
`os_DoAfterWith`, `os_PublishWith`, `os_PutWith`) and each will see the same
block. Those are the *only* ways to hand a context off — each of them bumps
the reference count so the OS knows another consumer is now holding it. If
you bypass them by stashing the pointer in a global or a static, the last
action holding a refcount will return, the OS will decrement to zero, and
the memory will be recycled under your feet.

In practice this means treat the context pointer the way you'd treat a stack
address: valid while you're using it in the current call, gone as soon as
you return.

### Typing the context

The OS has no idea what's inside a context — it hands you an untyped block
of the size you asked for. Casting the pointer to the right type and making
sure every action that touches it agrees on that type is the application's
job, not the OS's. By convention, the cast is the first line of the action
body:

```c
void MyAction(os_context_t context) {
    myState_t *s = (myState_t *)context;
    // ... use s ...
}
```

Putting it at the top keeps each action readable as a function of its state
type, and if you ever want to change an action's expected state, the
conflict shows up in exactly one visible place per action. All the examples
follow this convention; do the same in your own code.

### Context lifecycle

**You never call `free` on a context.** The OS owns context memory and
recycles it automatically off a reference count. The full cycle:

1. **Allocation.** `os_Do(action, size)` or
   `os_DoAfter(action, size, key, ticks)` asks the context allocator for a
   slot of at least `size` bytes and returns a pointer into it. The
   refcount starts at 1, held by the just-scheduled action. `os_Publish(topic, size)` does
   the same for a publish-delivery fan-out; `os_Put(key, size, ...)` does
   the same for a blackboard post (the blackboard holds the reference
   until retrieval, timeout, or overwrite).
2. **Sharing.** Every `os_DoWith` / `os_DoAfterWith` / `os_PublishWith` /
   `os_PutWith` that takes a live context bumps its reference count.
   The slot is now "held" by every in-flight consumer.
3. **Release.** When an action returns, the OS decrements the reference
   count for that action's context. A pub/sub delivery decrements when the
   subscriber's action returns. A blackboard entry decrements when it's
   retrieved by `os_Get`, when its TTL expires, or when a new
   `os_Put` / `os_PutWith` overwrites the same key.
4. **Collection.** When the reference count reaches zero, the slot is
   returned to the context allocator; the next allocation request can
   reuse it.

No explicit free, no ownership tracking. The only thing you have to do is
respect the rule from above — don't bypass the OS-aware handoffs by
stashing a pointer in a global, a static, or a captured value, because
that sidesteps the refcount and leaves you with a dangling pointer the
moment the last tracked consumer returns.

### The FIFO (ready queue)

Every scheduled action lives on a single, shared FIFO queue. `os_Exec()` pops
the head and runs it. New work goes on the tail.

There is no prioritization. There is no "which task should run next?"
computation. The system is strictly first-in, first-out: if you posted A
before B, A runs first. That determinism is the main thing EventOS gives you
that an RTOS can't — the order of events is exactly the order you posted them.

---

## The main loop

Every EventOS application looks like this at its center:

```c
os_Init();
// ... application setup: subscribe to topics, post the first actions ...
while (1) {
    os_Exec();
}
```

`os_Init()` is called exactly once at startup, before any other OS call,
to clear the pools and queues. `os_Exec()` pops one entry off the FIFO,
runs the action, decrements the context's reference count, and returns.
If the FIFO is empty, it returns immediately. That's the whole scheduler.

The timer queue is advanced by `os_Tick()`, which the platform must call
at a regular interval. On real hardware the SysTick ISR calls `os_Tick()`
and promotes expired delayed actions onto the FIFO for the next
`os_Exec()`. On the POSIX port a `posix_Tick()` call in the loop body
watches wall-clock time and calls `os_Tick()` cooperatively. See the
[Blinky](examples/Blinky/main.c) and
[Blinky-posix](examples/Blinky-posix/main.c) examples side by side to
compare, and [Porting](#porting) for the tick-source contract.

---

## Time

EventOS has two ways to schedule future work: **ASAP** and **delayed**. Time
in EventOS is always relative. The public API has no notion of absolute
time — every delay is expressed as "N ticks from now," there's no
"wake me at timestamp T" primitive, and there's no uptime counter to
query. If your application needs absolute time, you have to maintain it
yourself from ISR-observed ticks or another source.

### ASAP (As Soon As Possible): `os_Do` and `os_DoWith`

ASAP puts an action directly on the FIFO. It runs as soon as `os_Exec()`
reaches it — after every action already queued ahead of it, before anything
posted after. There is no time attached; it's just "next available slot, in
order."

ASAP is the default for everything that isn't explicitly time-bound: an ISR
dispatching work into cooperative context, one stage of an action chain
kicking off the next, a publisher fanning out to its subscribers.

### Delayed: `os_DoAfter` and `os_DoAfterWith`

These put an action on a **timer queue**, with a delay expressed in
**ticks**. Every SysTick interrupt, the OS decrements the head entry's
delta; when the delta reaches zero, that entry — and any trailing
zero-delta entries behind it — move from the timer queue to the FIFO.
The naming is deliberate: you ask for an action *after* a duration, not
*at* a time.

The tick period is chosen by the platform — it's whatever rate the platform
programs into its tick source. In the bluepill port the default is 1 ms:
the STM32F103 runs at 64 MHz (HSI × PLL) and `OS_CLOCKS_PER_TICK = 1000`
produces a 64000-cycle SysTick reload, giving a 1 kHz tick. That's why the
examples happen to talk in milliseconds. On a faster processor it's common
to run ticks at 100 µs or 50 µs to get finer scheduling granularity; all
timer APIs still take "ticks," and the unit is just whatever that platform's
tick is.

The timer queue is a delta-encoded linked list — each entry stores the delta
from the previous entry, not an absolute deadline — so only the head needs to
be decremented on each tick. Insertion is O(n) on the depth of the timer
queue; firing is O(1).

Two practical consequences:

- **Delays are lower bounds, not guarantees.** A delayed action fires no
  *sooner* than its deadline, but it joins the FIFO behind any ASAP work
  that's already waiting. If your FIFO is backed up, delayed actions wait.
- **Delay zero is an ASAP post.** `os_DoAfter` with `ticks == 0` takes the
  FIFO path directly — no timer-queue round trip. If your intent is to run
  an action ASAP, then use the `os_Do` forms.

---

## Patterns

EventOS's public API is small (a handful of `os_Do*` variants, pub/sub, a
bulletin board). The power comes from a few idioms these primitives compose
into. Each pattern below has a self-contained example in `examples/`.

### Action chains ("threads")

A "thread" in EventOS is a sequence of actions that walk a single piece of
work through stages, forwarding *the same* context (not a copy) from one
stage to the next. State written by Stage 1 is visible verbatim to Stage 2
— no copy, no repacking, no queue-specific serialization. Stage 1 schedules
Stage 2 with `os_DoWith`, Stage 2 schedules Stage 3, and so on. Multiple
threads run concurrently — their stages interleave on the FIFO — and each
thread owns its own context, so their states don't collide.

See [examples/Thread](examples/Thread/main.c) for the canonical form:
`Stage1 → Stage2 → Stage3`, spawned by a debounced button press, with a
shared heartbeat toggling LED in the background the whole time.

### Iteration — long-running work without loops

The "you can't loop in an action" objection has one clean answer: the
iteration pattern. An action does one step of work, re-enqueues itself (or a
successor) carrying loop state in the context, and yields. Other actions
interleave between steps.

The [Morse](examples/Morse/main.c) example uses two nested iteration threads
to emit "HELLO WORLD " in Morse code on LED_0 while an unrelated heartbeat on
LED keeps blinking every 500 ms — visible proof that the iteration never
blocked the system.

### Pub/Sub

Topics are `uint32_t` values. The OS neither defines nor owns the topic
namespace — pick whatever value you like for each topic (small integer
enums, 4-character tags like `'TICK'`, hashes, anything) with the one
exception that `OS_NO_KEY` (= 0) is reserved as a sentinel elsewhere in
the OS, so avoid it. Publishers call `os_Publish(topic, contextSize)` to
allocate a fresh context for the delivery, or
`os_PublishWith(topic, context)` to forward a context already in hand;
subscribers call `os_Subscribe(topic, action)`. Each publish enqueues the
subscriber's action+context on the FIFO, once per subscriber. Fan-out
(many subscribers on one topic) and fan-in (one subscriber on many topics)
both work out of the box.

Subscriptions are dynamic. `os_Subscribe`, `os_Unsubscribe`, and
`os_UnsubscribeAll` (clears every subscriber on a topic in one call) can
be invoked at any point from any action — wiring up at startup is the
typical pattern, but you are free to add and remove subscribers at runtime
(e.g. turn on verbose telemetry when a debug flag trips, then turn it off
again). When the last subscriber on a topic is removed — either by the
final `os_Unsubscribe` or by `os_UnsubscribeAll` — the topic's slot in
the subscription pool is freed and immediately available for a different
topic.

Publisher and subscriber are decoupled — neither side knows the other exists.
[examples/PubSub](examples/PubSub/main.c) shows both fan-out and fan-in in
one program.

### The blackboard

A key/value slot with a time-to-live.
`os_Put(key, contextSize, timeoutAction, ticks)` allocates a fresh
context and posts it under `key` for `ticks` ticks, returning a data
pointer the caller fills in. `os_PutWith(key, context, timeoutAction,
ticks)` does the same with a context already in hand (typically forwarded
from an in-flight action). `os_Get(key)` retrieves the posted
context (once) if it's still live, or returns NULL if the slot is empty
or expired.

`os_Get` hands ownership of the returned context to the calling action:
read the data and drop the pointer — the OS frees the context when the
action returns — or forward it through any OS-aware function (`os_DoWith`,
`os_DoAfterWith`, `os_PublishWith`, `os_PutWith`) to keep it alive. Only
one context can be "held" by an action at a time; a second `os_Get` in
the same action releases the first result before handing over the
second, so if you need data from both keys, copy the values you want
out of the first pointer (into a local variable or into a context you're
about to forward) before making the second call.

As with pub/sub topics, key values are entirely in the application's gift:
any `uint32_t` works except `OS_NO_KEY` (= 0), which is reserved as a
sentinel elsewhere in the OS. Note also that blackboard keys share the
same namespace as `os_DoAfter` / `os_CancelPending` keys, so if an
application uses both, pick distinct values to avoid accidental crossing.

Use it when a producer wants to make state available "if anyone happens to
want it within the next second." If nothing consumes the value before its TTL
expires, a timeout callback fires so the producer can react. When the timeout
notification is not needed, use `os_NullAction` for the `timeoutAction`.

[examples/Blackboard](examples/Blackboard/main.c) walks the
`os_PutWith` path through a hit / miss / timeout cycle;
[examples/Demo](examples/Demo/main.c) shows the `os_Put` path for a
producer that has no in-flight context to forward.

### Cancellable work

`os_DoAfter(..., key, ticks)` and `os_DoAfterWith(..., key, ticks)` tag a 
delayed action with a `uint32_t` cancel key. Calling `os_CancelPending(key)`
removes every pending entry with that key from both the timer queue and the
FIFO. This gives you many idioms common to embedded systems, for example:

- **Dynamic timers.** Cancel the current blink rate, post a new one at a
  different interval ([examples/CancelPending](examples/CancelPending/main.c)).
- **Software debounce.** Every ISR edge cancels the pending "settled" action
  and reposts it at the end of a fresh settle window; only the last edge
  actually produces an event ([examples/Debounce](examples/Debounce/main.c)).
- **Protocol timeout.** Send a request, then post a timeout action with a
  cancel key. When the response arrives, cancel the key and the timeout is
  disarmed; if the response never comes, the timeout action runs and you
  handle the failure. That's a complete request/response timeout coordination
  — no flags, no watcher task, no state machine — using just the primitives
  the OS already gives you.

Cancel keys live in the same namespace as blackboard keys — pick distinct
values if you use both in one program.

### ISR → action handoff

ISRs should be short. EventOS's idiom is: the ISR calls `os_Do(MyAction, ...)`
to post work onto the FIFO, populates required state from the hardware into
the context, and returns. `MyAction` runs later, in cooperative context, with
full OS facilities available. The handoff is the only thing that needs to
happen inside the ISR itself.

An ISR may use `os_Do`, `os_DoAfter`, `os_Publish`, and `os_Put` freely — all
four allocate their own context internally, so the ISR just fills in the
returned pointer and returns. A single call is typical, but an ISR may
enqueue as many of these as the design needs. What an ISR *cannot* use is any
`_With` variant (`os_DoWith`, `os_DoAfterWith`, `os_PublishWith`,
`os_PutWith`) — those forward an existing context to the new action, and an
ISR isn't running inside an action, so there's no context in scope to hand off.

[examples/Button](examples/Button/main.c) shows the bare handoff;
[examples/Debounce](examples/Debounce/main.c) adds the debounce pattern
on top.

---

## Concurrency model

This is the part most worth understanding before you write an EventOS
application, because it's where it diverges hardest from an RTOS.

### Action vs action: no races

Because actions run to completion in FIFO order, two actions *cannot*
interleave with each other. When action A is running, action B is sitting on
the FIFO waiting. Shared state between A and B needs no locks, no atomics, no
critical sections — the serialization is built into the scheduler.

This eliminates entire categories of bugs that dominate RTOS debugging
(priority inversion, deadlock, most TOCTOU races between tasks).

### Action vs ISR: critical sections

ISRs *can* preempt actions, so any state shared between them needs protection.
EventOS exposes `hal_CriticalBegin()` and `hal_CriticalEnd()` as a public HAL
API for this — on Cortex-M they disable and re-enable interrupts, on POSIX
they're no-ops (the single-threaded demo has no ISR). Critical sections
nest — interrupts stay masked from the outermost `Begin` through the
matching outermost `End`; inner nested pairs are invisible to the
hardware.

The OS uses these itself to protect the FIFO and timer queue from tick-time
modifications, but they're part of the public HAL precisely because
applications have the same need. The typical case is an ISR writing into a
small buffer (UART bytes, sensor samples, scan codes) that an action drains
later — the action's read of the write-index plus the data it references
needs to happen inside a `hal_CriticalBegin/End` pair so it can't observe a
half-updated state. Use them anywhere you have state that an ISR and an
action both touch.

Keep critical sections short. A long one holds ISRs off indefinitely, which
is exactly what you're trying to avoid by having a fast scheduler in the
first place.

### Why this is simpler than an RTOS

An RTOS gives you *mechanism* (preemption, priorities, mutexes) and hands you
the *policy* choices (which task is high-priority, which mutex wraps which
resource). EventOS removes the mechanism — there is no preemption — so the
policy problems don't exist either. You give up the ability to force a
high-priority task ahead of a low-priority one; you gain the guarantee that
the order of operations in your code is the order they actually execute.

For a large class of embedded applications — anything that's fundamentally
reactive, driven by interrupts and timers and messages — the RTOS machinery
was paying for a capability you weren't using.

---

## Performance and observability

### The load metric is queue depth, not idle time

A natural first question from an RTOS background is "what's the CPU
utilization?" That's the wrong number for a cooperative RTC system. The right
number is **FIFO depth over time**.

- Average FIFO depth × average action runtime = average event latency
  (Little's Law).
- A system that's 90% "idle" with a backed-up FIFO is still missing
  deadlines.
- A fully busy system with a shallow FIFO is still meeting them.

Sample the FIFO depth periodically and watch its max and trend. That tells
you whether you're keeping up; idle-time percentages tell you nothing
useful.

**How to sample it.** EventOS doesn't publish a pure FIFO-only depth
counter, but the entry pool counters `os_EntryInUse()` and
`os_EntryHighWater()` (see [Internal pools → Entry pool](#entry-pool))
give you the total live count across every scheduled entry — FIFO and
timer queue alike — which is the right thing in practice. Timer-queue
entries (including blackboard-timeout anchors) are waiting on a
deadline, not on the CPU, so they don't contribute to load *pressure*;
when the entry count climbs unexpectedly, it's because the FIFO is
growing. Sample periodically, log the high-water
mark, alert when the count approaches `OS_MAX_ENTRIES`.

If the timer-queue contribution is large enough that you want to back it
out for a pure FIFO number, you can keep an application-side counter —
increment it at every post site (`os_Do`, `os_Publish`, etc.) and
decrement it in an [`os_ActionBegin`](#instrumentation-hooks) override. A
caveat: timer-queue entries promoting to FIFO on tick don't fire a hook,
so you'd either need to also count posts from `os_DoAfter` and match
cancellations, or accept that the counter drifts by the number of
currently-pending delayed actions. For most applications, the aggregate
entry count is accurate enough and cheaper to maintain.

### Instrumentation hooks

`os_Exec()` brackets every action invocation with two weak-no-op hooks:

- `os_ActionBegin(action)` — called just before the action runs.
- `os_ActionEnd(action)` — called just after the action returns.

Override either (or both) to install whatever measurement fits your platform:
a cycle counter (DWT on Cortex-M), `clock_gettime` on POSIX, a GPIO toggle so
a logic analyzer can measure action duration, a trace log keyed by action
pointer. The OS itself takes no opinion on units or storage; if you don't
override, the defaults are empty functions — a few bytes of call overhead
per dispatch, nothing more.

The hooks receive the action's function pointer so attribution ("which action
was the expensive one?") is trivial.

**Discipline — treat these like ISR bodies.** They run on the hot path of
every dispatch, so keep them short: anything expensive you do here adds
latency to every action in the system. Don't loop, don't block, don't
write to a slow bus.

The hooks receive only the action pointer — no context. They can't touch
the context the dispatching action is about to receive or has just
released, which rules out an entire class of observability mistakes by
construction.

Calling `os_Do`, `os_DoAfter`, `os_Publish`, etc. from inside a hook is
*safe* (the OS's cooperative invariants still hold), but almost never the
right choice. Instrumentation that schedules new work muddies the metric
you're trying to measure. Prefer an in-memory counter, a GPIO toggle, or
a pre-allocated trace buffer; do any heavy post-processing outside the
hook path.

---

## Failure handling

When EventOS runs out of a resource it depends on — a context slot, an
entry slot, a subscriber slot on a topic, a bulletin-board entry — the
operation you asked for cannot complete. Rather than silently corrupt
state or return a hard-to-interpret error, EventOS calls a user-provided
handler, `os_Fail(os_fail_t reason)`, to report that something has gone
wrong.

`os_Fail` is the one correctness-critical hook the application is expected
to supply. Each shipped platform provides a weak default that halts the
system (`while (1);` on the bluepill, `exit(1)` on POSIX); replace it with
anything that fits your target — log and reset, drive an error-state LED
pattern, write a post-mortem snapshot then reboot. The `os_fail_t` enum in
`os/os.h` has one distinct reason code per failure site, so a handler that
logs the code can pinpoint exactly which allocation or subscription
request ran out of room.

**The critical invariant: do not call any OS function after `os_Fail` has
been called.** The OS reached `os_Fail` because an internal update could
not complete; lists may be half-spliced, counters may be stale, reference
counts may not balance. Once the handler runs, further calls into the OS
have undefined behavior. There are two practical shapes this takes:

- If your `os_Fail` **does not return** (an infinite loop, a reset, or
  `exit`), no further OS calls happen and the invariant is satisfied by
  construction. This is the safe default and the recommended shape.
- If your `os_Fail` **does return** (for example, to let you log before
  rebooting from the main loop), the OS function that called it will
  unwind and return to its caller without completing the original
  operation. The application must treat the system as unrecoverable from
  that moment on: no more `os_Do`, `os_Publish`, `os_Put`, or any
  other OS primitive. Get to a reset as quickly as you can, and keep the
  path from "handler returned" to "system resets" free of OS calls.

---

## Internal pools

EventOS uses four fixed-size pools instead of a runtime heap. All four are
statically allocated, deterministic, and sized at compile time via macros.
Override them by passing `-D<NAME>=<value>` from your build system, or — if
you prefer a single discoverable config file — copy
`app/include/os/os_config.h.template` into your app tree as `os_config.h`,
add that directory to the include path, and compile with `-DOS_USE_CONFIG_H`.
The template lists every knob with its default and short description; any
knob you leave commented keeps its default, and a `-D` on the command line
still wins over the config file. Running out of any pool is a failure site
— see [Failure handling](#failure-handling).

Every pool ships with an `InUse` / `HighWater` API you can sample for
observability; those are the numbers to watch when deciding whether a
default is big enough.

### Entry pool

Every call that schedules an action (`os_Do`, `os_DoAfter`, `os_DoWith`,
`os_DoAfterWith`, and each subscriber delivery inside `os_Publish` /
`os_PublishWith`) consumes one entry. A single entry simultaneously serves as the FIFO
element and the timer-queue element, so the pool must cover the sum of
*FIFO depth + timer queue depth + in-flight pub/sub deliveries*.

| Macro             | Default |
|-------------------|---------|
| `OS_MAX_ENTRIES`  | 32      |

Exhaustion trips one of `OS_FAIL_DO_ALLOCATION`,
`OS_FAIL_DO_AFTER_ALLOCATION`, `OS_FAIL_DO_WITH_ALLOCATION`, or
`OS_FAIL_DO_AFTER_WITH_ALLOCATION` depending on which primitive asked.

Observability: `os_EntryInUse()`, `os_EntryHighWater()`.

### Subscription pool

One slot per *unique topic* that anything subscribes to (not one per
subscriber). Each slot carries up to `OS_NUMBER_OF_SUBS` subscriber
actions; subscribing the same action twice is silently deduplicated.

| Macro                   | Default |
|-------------------------|---------|
| `OS_MAX_SUBSCRIPTIONS`  | 8       |
| `OS_NUMBER_OF_SUBS`     | 4       |

Exhaustion: `OS_FAIL_SUBSCRIBE_ALLOCATION` (no free slot for a new topic)
or `OS_FAIL_SUBSCRIBE_FULL` (the topic has already used its
`OS_NUMBER_OF_SUBS` subscriber positions).

Observability: `os_SubInUse()`, `os_SubHighWater()`.

### Blackboard pool

One slot per active key posted via `os_Put` or `os_PutWith`. A slot is freed when
the context is retrieved with `os_Get`, when its TTL expires, or
when the key is overwritten.

| Macro               | Default |
|---------------------|---------|
| `OS_MAX_BB_ENTRIES` | 8       |

Exhaustion: `OS_FAIL_PUT_ALLOCATION` or `OS_FAIL_PUT_WITH_ALLOCATION`
depending on which primitive asked.

Observability: `os_BbInUse()`, `os_BbHighWater()`.

### Context pool

The context allocator is pluggable — you pick one at link time with
`make CTX_ALLOC=<name>`:

- **`ctxAllocMalloc`** (default) — a thin wrapper around `malloc`/`free`.
  Useful on a host, and fine for early development before you've sized
  your application. Unsuitable for bare-metal production on two counts:
  the libc heap fragments over time, and `malloc` / `free` have
  implementation-defined and generally unbounded per-call runtime — both
  of which are exactly what you're trying to avoid by going to an RTC
  event system in the first place.
- **`ctxAllocPool`** — a fixed, statically-allocated two-bucket pool. No
  heap, no fragmentation, deterministic allocation time.

The two-bucket pool carves context memory into a *small* pool for
short contexts and a *large* pool for everything up to a hard ceiling. 
A request of size ≤ the small-slot size goes to the small pool;
≤ the large-slot size goes to the large pool; anything bigger trips
`OS_FAIL_CONTEXT_ALLOCATION`.

| Macro                | Default |
|----------------------|---------|
| `OS_CTX_SMALL_SLOT`  | 16      |
| `OS_CTX_SMALL_COUNT` | 4       |
| `OS_CTX_LARGE_SLOT`  | 64      |
| `OS_CTX_LARGE_COUNT` | 4       |

**Single-pool configuration.** If your contexts cluster around one size,
set the `COUNT` of the bucket you don't want to `0` and the `SLOT` and
`COUNT` of the bucket you do want to your chosen slot size and count.
At `COUNT=0` the bucket's array becomes zero-sized (no RAM cost) and the
init, alloc, and free paths short-circuit past it at runtime — the
optimizer folds the dead branches away under `-Os`.

The "small" and "large" labels are just names for two buckets; when
only one is live, the two choices are functionally equivalent. Pick
whichever reads better in your config and stick with it. Requests that
exceed the remaining bucket's slot size trip `OS_FAIL_CONTEXT_ALLOCATION`.

Observability: `os_ctxInUseCount` and `os_ctxHighWaterMark` are
`uint32_t` globals the pool allocator exposes (the malloc allocator
does not publish anything equivalent).

**Bring your own allocator.** The interface is three functions —
`os_CtxAllocInit` (once at boot), `os_CtxAlloc(size)`, `os_CtxFree(ctx)`
— with no static state on the OS side. Drop a replacement `.c` into the
build, point `CTX_ALLOC` at it, and you're done.

---

## Porting

Moving EventOS to a new platform is three pieces of work. Both existing
platforms — [examples/bluepill](examples/bluepill/) (STM32F103) and
[examples/posix](examples/posix/) — demonstrate the seams.

### 1. The HAL

Implement `hal_CriticalInit` / `hal_CriticalBegin` / `hal_CriticalEnd`
(`app/include/hal/Critical.h`) however your platform wants. On a preemption-
capable MCU, disable and re-enable interrupts; on a host-side demo, no-op;
on a pthread build, take and release a mutex.

### 2. The tick source

Something needs to call `os_Tick()` at a regular interval. The period is
the platform's choice — 1 ms on the bluepill, faster (100 µs, 50 µs) on
platforms with cycles to spare, or slower if the application can tolerate
it. Whatever you pick becomes the unit that every `os_DoAfter` / `os_Put`
call is expressed in.

The tick period is programmed via the tunable macro `OS_CLOCKS_PER_TICK`
(default `1000`), which the port uses to compute its hardware reload
value. Override it from the build system with `-DOS_CLOCKS_PER_TICK=…`
or through `os_config.h`, same as any other knob — see [Internal
pools](#internal-pools) for the full override story.

On Cortex-M a SysTick handler is the
usual tick source; on POSIX a polling loop watching wall-clock time is the
equivalent; on a platform with no hardware timer, anything that fires at a
known interval will do.

### 3. Failure

A port also typically supplies the weak default of `os_Fail` — see
[Failure handling](#failure-handling) above for the contract. If you also
need a non-default context allocator (heapless targets, shared-memory
builds, etc.), see [Internal pools → Context pool](#context-pool).

---

## Footprint

Measured on STM32F103 with `arm-none-eabi-gcc -Os`, nano/nosys specs, every
feature exercised (pub/sub, blackboard, CancelPending, heartbeat) — the
[Demo](examples/Demo/main.c) example:

| section | bytes |
|---------|-------|
| text    | 4120  |
| data    | 92    |
| bss     | 1396  |

Individual single-feature examples are smaller (Blinky is 2760 text). The
Demo number is the one to compare against when you're evaluating whether the
"full OS" fits your budget; if it doesn't, `--gc-sections` will strip whatever
you don't use.

---

## When *not* to use EventOS

Honest counter-pitch. Reach for an RTOS, or a different approach, when:

- **You need preemption.** A hard real-time deadline that requires a
  high-priority task to interrupt a low-priority one mid-execution is exactly
  what EventOS gives up. A missed-deadline-means-fatal system wants
  priorities and preemption; EventOS is wrong for it.
- **Your workflow is naturally thread-shaped.** If your application reads
  like `read(); parse(); validate(); write();` with blocking calls throughout,
  forcing it into an action-per-stage pipeline is a lot of rewriting for
  no gain. Threads are the right shape for that code.
- **You need a rich IPC surface.** EventOS has pub/sub and a bulletin board.
  If you want counting semaphores, blocking mutexes, message queues with
  timeouts, or a filesystem-like API — use a library that provides them.
- **You want someone else to have made the hard choices.** EventOS is small
  enough that you can read the whole implementation in an afternoon. If
  you'd rather point at a vendor and say "it's their OS, they support it",
  FreeRTOS / Zephyr / ThreadX are well-supported and will never be the thing
  that gets blamed in your bug report.

For the large remaining category — reactive embedded systems driven by
timers, ISRs, and messages, where determinism and footprint matter more than
preemption does — EventOS is a good fit.
