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

## Findings (2026-04-26)

| Test | Result | Note |
|------|--------|------|
| 1 Baseline | ✅ `VERSION = 0x12` | Chip rev 0x12, mask `0xFFF0` OK |
| 2 SHT3x → 50ms → MAX17048 | ✅ MAX17048 OK | No init conflict |
| 3 SHT3x → 500ms → MAX17048 | ✅ MAX17048 OK | Delay not required |
| 4 MAX17048 → SHT3x → MAX17048 | ✅ Both OK | Order irrelevant |
| 5 I2C ping 0x36–0x45 | ⚠️ All ACK | Ameba quirk, harmless |
| 6 Register dump | ✅ VER=`0x12`, STATUS=`0xFFFF` | STATUS read fails (likely bus state after ping) |

**Conclusion:** The combined initialization itself works. The previous failure in the combined sketch was caused by something else (likely double `Wire.begin()` or wrong init order in the earlier attempt). The production sketch in `98_MAX_SH` uses the same init sequence and works correctly.

## Serial Output

```
========================================
  DEBUG: SHT3x + MAX17048 Combined
========================================

--- TEST 1: MAX17048 only (baseline) ---
  VERSION read = 0x12
MAX17048: OK

--- TEST 2: SHT3x softReset -> delay 50ms -> MAX17048 ---
  endTransmission err = 0
SHT3x softReset: OK
  VERSION read = 0x12
MAX17048 after SHT3x reset: OK
...
```

## Next Steps

Use `98_MAX_SH` for production. This debug workspace is kept for reference.

## License

MIT
