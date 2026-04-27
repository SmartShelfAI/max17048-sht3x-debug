/*
 * DEBUG: SHT3x + MAX17048 combined initialization
 * Platform: AmebaPRO2-mini (AMB82-mini)
 *
 * I2C bus: Pin 12 (SDA), Pin 13 (SCL)
 * Both devices share the same bus.
 *
 * This sketch tests different initialization orders and delays
 * to find why MAX17048 fails when SHT3x is initialized first.
 */

#include <Wire.h>

#define SHT3X_ADDR       0x44
#define MAX17048_ADDR    0x36

#define SHT3X_CMD_SOFT_RESET   0x30A2
#define MAX17048_REG_VER       0x08
#define MAX17048_REG_STATUS    0x00

uint16_t readReg16(uint8_t addr, uint8_t reg);
bool sht3x_softReset();
bool max17048_check();

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println(F("\n========================================"));
  Serial.println(F("  DEBUG: SHT3x + MAX17048 Combined"));
  Serial.println(F("========================================\n"));

  Wire.begin();
  Wire.setClock(100000);
  delay(100);

  // ── Test 1: MAX17048 ONLY (baseline) ──
  Serial.println(F("--- TEST 1: MAX17048 only (baseline) ---"));
  bool ok1 = max17048_check();
  Serial.print(F("MAX17048: "));
  Serial.println(ok1 ? F("OK") : F("FAIL"));
  Serial.println();

  // ── Test 2: SHT3x soft reset, then MAX17048 (short delay) ──
  Serial.println(F("--- TEST 2: SHT3x softReset -> delay 50ms -> MAX17048 ---"));
  bool ok2a = sht3x_softReset();
  Serial.print(F("SHT3x softReset: "));
  Serial.println(ok2a ? F("OK") : F("FAIL"));

  delay(50);

  bool ok2b = max17048_check();
  Serial.print(F("MAX17048 after SHT3x reset: "));
  Serial.println(ok2b ? F("OK") : F("FAIL"));
  Serial.println();

  // ── Test 3: SHT3x soft reset, then MAX17048 (long delay) ──
  Serial.println(F("--- TEST 3: SHT3x softReset -> delay 500ms -> MAX17048 ---"));
  bool ok3a = sht3x_softReset();
  Serial.print(F("SHT3x softReset: "));
  Serial.println(ok3a ? F("OK") : F("FAIL"));

  delay(500);

  bool ok3b = max17048_check();
  Serial.print(F("MAX17048 after 500ms delay: "));
  Serial.println(ok3b ? F("OK") : F("FAIL"));
  Serial.println();

  // ── Test 4: Reverse order ─ MAX17048 first, then SHT3x, then MAX17048 again ──
  Serial.println(F("--- TEST 4: MAX17048 -> SHT3x softReset -> MAX17048 again ---"));
  bool ok4a = max17048_check();
  Serial.print(F("MAX17048 before SHT3x: "));
  Serial.println(ok4a ? F("OK") : F("FAIL"));

  delay(100);

  bool ok4b = sht3x_softReset();
  Serial.print(F("SHT3x softReset: "));
  Serial.println(ok4b ? F("OK") : F("FAIL"));

  delay(100);

  bool ok4c = max17048_check();
  Serial.print(F("MAX17048 after SHT3x: "));
  Serial.println(ok4c ? F("OK") : F("FAIL"));
  Serial.println();

  // ── Test 5: I2C address ping ──
  Serial.println(F("--- TEST 5: I2C address ping ---"));
  for (uint8_t a = 0x36; a <= 0x45; a++) {
    Wire.beginTransmission((int)a);
    uint8_t err = Wire.endTransmission();
    Serial.print(F("  0x"));
    if (a < 0x10) Serial.print('0');
    Serial.print(a, HEX);
    Serial.print(F(" -> "));
    Serial.println(err == 0 ? F("ACK") : F("NACK/ERR"));
  }
  Serial.println();

  // ── Test 6: MAX17048 register dump ──
  Serial.println(F("--- TEST 6: MAX17048 register dump ---"));
  uint16_t ver = readReg16(MAX17048_ADDR, MAX17048_REG_VER);
  uint16_t sta = readReg16(MAX17048_ADDR, MAX17048_REG_STATUS);
  Serial.print(F("VERSION = 0x")); Serial.println(ver, HEX);
  Serial.print(F("STATUS  = 0x")); Serial.println(sta, HEX);
  Serial.println();

  Serial.println(F("=== Done ==="));
  while (1) delay(1000);
}

void loop() {}

// ── MAX17048: check presence ────────────────────────────────────────────────
bool max17048_check() {
  uint16_t ver = readReg16(MAX17048_ADDR, MAX17048_REG_VER);
  Serial.print(F("  VERSION read = 0x"));
  Serial.println(ver, HEX);
  return ((ver & 0xFFF0) == 0x0010);
}

// ── SHT3x: soft reset only ──────────────────────────────────────────────────
bool sht3x_softReset() {
  Wire.beginTransmission((int)SHT3X_ADDR);
  Wire.write((SHT3X_CMD_SOFT_RESET >> 8) & 0xFF);
  Wire.write(SHT3X_CMD_SOFT_RESET & 0xFF);
  uint8_t err = Wire.endTransmission();
  Serial.print(F("  endTransmission err = "));
  Serial.println(err);
  return (err == 0);
}

// ── Read 16-bit register ────────────────────────────────────────────────────
uint16_t readReg16(uint8_t addr, uint8_t reg) {
  Wire.beginTransmission((int)addr);
  Wire.write(reg);
  uint8_t err = Wire.endTransmission(false);
  if (err != 0) {
    Serial.print(F("  [read err="));
    Serial.print(err);
    Serial.println(F("]"));
    return 0xFFFF;
  }

  Wire.requestFrom((int)addr, 2);
  delay(1);  // Ameba workaround

  if (Wire.available() < 2) {
    Serial.println(F("  [no data]"));
    return 0xFFFF;
  }

  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read();
  return ((uint16_t)msb << 8) | lsb;
}
