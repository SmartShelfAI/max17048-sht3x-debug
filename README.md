# DEBUG: MAX17048 + SHT3x Combined I2C Initialization

Debug workspace for combined battery monitor (MAX17048) and temperature/humidity sensor (SHT3x) on shared **I2C bus (Pin 12/13)**.

## Problem Statement

- Standalone MAX17048 sketch → **works**
- Standalone SHT3x sketch → **works**
- Combined sketch (SHT3x init first) → **MAX17048 fails** (`VERSION` read returns `0xFFFF`)

This sketch isolates and tests different initialization orders and delays.

## Hardware

| Device     | I2C Address | Pin 12 | Pin 13 |
|------------|-------------|--------|--------|
| MAX17048   | 0x36        | SDA    | SCL    |
| SHT3x      | 0x44        | SDA    | SCL    |

## Tests Performed

1. **Baseline** — MAX17048 only, verify VERSION register
2. **SHT3x softReset → 50ms → MAX17048** — short delay
3. **SHT3x softReset → 500ms → MAX17048** — long delay
4. **Reverse order** — MAX17048 first, then SHT3x soft reset, then MAX17048 again
5. **I2C ping** — scan addresses 0x36–0x45 for ACK/NACK
6. **Register dump** — read MAX17048 VERSION and STATUS

## Serial Output

```
========================================
  DEBUG: SHT3x + MAX17048 Combined
========================================

--- TEST 1: MAX17048 only (baseline) ---
  VERSION read = 0x001F
MAX17048: OK

--- TEST 2: SHT3x softReset -> delay 50ms -> MAX17048 ---
  endTransmission err = 0
SHT3x softReset: OK
  VERSION read = 0xFFFF
MAX17048 after SHT3x reset: FAIL
...
```

## Next Steps

Depending on test results, apply fixes to the main combined sketch.

## License

MIT
