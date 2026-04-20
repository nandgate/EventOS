# UnitTest

UnitTest is a simple unit testing framework for C code. UnitTest provides facilities for
making assertions, mocking (faking) functions and a means for running unit tests. UnitTest
is minimalistic in its approach to unit testing. UnitTest is intended to be used with
embedded systems.

## Documentation
See the [documentation](docs/unittest.adoc) page for details on how to setup and use the framework.

## Dependencies
* C99 compatible C compiler
* bash version 3.1 or higher

## Quick Reference

### File Scope Declarations
| Macro | Description |
|---|---|
| `Mock_Vars(depth)` | Declare framework state; `depth` = max calls tracked per mock |
| `Mock_Void(fn)` / `Mock_VoidN(fn, ...)` | Declare a void mock (0–5 args) |
| `Mock_Value(ret_t, fn)` / `Mock_ValueN(ret_t, fn, ...)` | Declare a value-returning mock (0–5 args) |

### Life Cycle (in `main`)
| Macro | Description |
|---|---|
| `Assert_Init()` | Load running assertion count from `assertions.txt` |
| `Assert_Save()` | Write assertion count back to `assertions.txt` |

### setUp Convention (call at top of every test)
| Macro | Description |
|---|---|
| `Test_Init()` | Reset call-order tracking and notes |
| `Mock_Reset(fn)` | Reset call count, argument history, and return value for a mock |

### Mock Control
| Macro | Description |
|---|---|
| `Mock_Returns(fn, val)` | Return `val` on every call |
| `Mock_ReturnsSequence(fn, len, seq)` | Return values in sequence; repeat last when exhausted |
| `Mock_Custom(fn, impl)` | Delegate to a custom implementation function |

### Value Assertions
| Macro | Description |
|---|---|
| `Assert_Equals(expected, actual)` | Integer equality |
| `Assert_NotEquals(expected, actual)` | Integer inequality |
| `Assert_EqualsLong(expected, actual)` | 64-bit integer equality |
| `Assert_EqualsFloat(expected, actual, delta)` | Float equality within tolerance |
| `Assert_ArrayEquals(len, expected, actual)` | Element-wise array equality |
| `Assert_StrEquals(expected, actual)` | String equality |
| `Assert_True(actual)` | Non-zero |
| `Assert_False(actual)` | Zero |
| `Assert_IsNull(actual)` | Pointer is NULL |
| `Assert_IsNotNull(actual)` | Pointer is not NULL |
| `Assert_Fail(msg)` | Unconditional failure |
| `Assert_Note(msg)` | Set a label shown on the next failure |

### Mock Call Assertions
| Macro | Description |
|---|---|
| `Assert_CalledOnce(fn)` | Called exactly once |
| `Assert_NotCalled(fn)` | Never called |
| `Assert_CallCount(n, fn)` | Called exactly `n` times |
| `Assert_CallOrder(fn1st, fn2nd)` | `fn1st` was called before `fn2nd` |
| `Assert_Returned(fn, expected)` | Mock returned `expected` at least once |
| `Assert_CalledN(fn, ...)` | At least one call had these args (N = arg count) |
| `Assert_CalledFirstN(fn, ...)` | First call had these args |
| `Assert_CalledLastN(fn, ...)` | Last call had these args |
| `Assert_CalledNN(n, fn, ...)` | Nth call (1-based) had these args |
| `Assert_AllCallsEqualsN(fn, val)` | Every call's last arg == `val` |
| `Assert_AllCallsLessThanN(fn, val)` | Every call's last arg < `val` |
| `Assert_AllCallsGreaterThanN(fn, val)` | Every call's last arg > `val` |

## License
UnitTest is published under the [BSD Zero Clause License (0BSD)](../LICENSE) —
use, modify, and redistribute freely; no attribution or warranty.

(C) 2017-2026 NAND Gate Technologies LLC
