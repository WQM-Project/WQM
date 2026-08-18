# Bill of Materials — WQM Sensor Suite (Recommended Build)

> **Document:** BOM-WQM-SENSOR-2026-R1  
> **Project:** River Water Quality Monitor with Pollution Prediction  
> **Revision:** 1.0  
> **Date:** 2026-08-19  
> **Budget Ceiling:** ₹30,000 (sensor subsystem only)

---

## 1. Sensor Modules

| # | Qty | Part Number | Description | Interface | Unit Price (₹) | Subtotal (₹) | Source |
| --- | ----- | ------------- | ------------- | ----------- | --------------- | -------------- | -------- |
| 1 | 1 | DFRobot SEN0161 | Gravity: Analog pH Meter Kit (glass bulb electrode, BNC) | Analog | 800–1,200 | 800–1,200 | Robu / Robocraze |
| 2 | 1 | DFRobot SEN0189 | Gravity: Analog Turbidity Sensor (IR optical) | Analog | 700–950 | 700–950 | Robu / Robocraze |
| 3 | 1 | DFRobot SEN0244 | Gravity: Analog TDS Sensor Kit | Analog | 300–450 | 300–450 | Robu / Robocraze |
| 4 | 2 | — (generic) | DS18B20 Waterproof Temperature Probe (stainless steel, 1m cable) | Digital (OneWire) | 150–200 | 300–400 | Robu / Amazon IN |
| 5 | 1 | DFRobot SEN0237-A | Gravity: Analog Dissolved Oxygen Sensor Kit (galvanic) | Analog | 3,500–4,200 | 3,500–4,200 | Robu / MakerBazar |
| 6 | 1 | DFRobot DFR0300 | Gravity: Analog Conductivity Sensor (K=1) | Analog | 1,200–1,800 | 1,200–1,800 | Robu / Robocraze |
| 7 | 1 | DFRobot SEN0165 | Gravity: Analog ORP Sensor Meter (platinum electrode, BNC) | Analog | 9,400–10,800 | 9,400–10,800 | Robu / Element14 |
| 8 | 1 | JSN-SR04T | Waterproof Ultrasonic Distance Sensor (20–600 cm) | Digital (Trig/Echo) | 250–400 | 250–400 | Robu / Amazon IN |
| 9 | 1 | GY-BME280 | BME280 Barometric Pressure + Humidity + Temperature Breakout | I2C | 250–400 | 250–400 | Robu / Robocraze |

| | | | | | **Sensor Total** | **₹16,700–20,600** | |

---

## 2. Calibration & Consumables (Initial Kit)

| # | Qty | Description | Est. Price (₹) | Notes |
| --- | ----- | ------------- | ---------------- | ------- |
| C1 | 2 | pH Buffer Solution — pH 4.0 (250 mL) | 150–250 | For two-point calibration |
| C2 | 2 | pH Buffer Solution — pH 7.0 (250 mL) | 150–250 | For two-point calibration |
| C3 | 1 | DO Membrane Cap + Electrolyte Fill (0.5 mol/L NaOH) — spare | 400–600 | First replacement set |
| C4 | 1 | ORP Calibration / Cleaning Solution | 200–400 | For periodic platinum tip maintenance |
| C5 | 1 | Conductivity Calibration Standard (1413 μS/cm, 250 mL) | 200–350 | For K=1 probe calibration |

| | | | **Consumables Total** | **₹1,100–1,850** |

---

## 3. 3D-Printed Parts & Filament

| # | Qty | Part | Material | Est. Print Weight (g) | Notes |
| --- | ----- | ------ | ---------- | ----------------------- | ------- |
| P1 | 1 | Probe mast body (sensor rack, ~30 cm) | PETG | 80–120 | Rigid, UV-resistant, waterproof. Carries all submerged sensors in fixed positions. |
| P2 | 1 | Turbidity sensor shroud/baffle | PETG | 15–25 | Side-slotted cover over SEN0189 optical window. Blocks bubble ingress. |
| P3 | 1 | DO membrane guard cage | PETG | 10–15 | Wire-mesh style cage around galvanic membrane tip. Non-negotiable protection. |
| P4 | 1 | pH bulb guard tube | PETG / PVC | 10–15 | Perforated tube recessing the glass bulb from lateral impact. |
| P5 | 1 | Cable gland mounting plate | PETG | 15–25 | Sealed hull-entry plate with holes for 6–8 cable glands. |
| P6 | 1 | BME280 + electronics mounting bracket | PLA | 5–10 | Internal hull bracket. No water contact — PLA acceptable. |
| P7 | 1 | JSN-SR04T hull-bottom mount | PETG | 10–15 | Angled bracket, faces sensor downward, clear of prop wash. |

| | | | **Est. Total Print Weight** | **145–225 g** | |

**Filament Required:**

| # | Qty | Description | Est. Price (₹) | Notes |
| --- | ----- | ------------- | ---------------- | ------- |
| F1 | 1 | PETG filament — 1 kg spool (white or black) | 800–1,200 | UV-resistant, waterproof, suitable for all submerged parts. ~145–225 g used; rest is spare. |
| F2 | 1 | PLA filament — 250 g or share from existing spool | 0–300 | Only for internal bracket (P6). Skip if PETG is used for everything. |

| | | | **Filament Total** | **₹800–1,500** |

> [!TIP]
> **Why PETG over PLA for submerged parts:** PLA absorbs water over days/weeks and becomes brittle. PETG is hydrophobic, UV-stable, and maintains structural integrity indefinitely underwater. All parts touching water **must** be PETG (or ASA/ABS if available).

---

## 4. Interface Summary

| Bus / Channel | Sensors Connected | Pin Requirement |
| --------------- | ------------------- | ----------------- |
| ADC Ch 0 | pH (SEN0161) | 1 analog input |
| ADC Ch 1 | Turbidity (SEN0189) | 1 analog input |
| ADC Ch 2 | TDS (SEN0244) | 1 analog input |
| ADC Ch 3 | DO (SEN0237-A) | 1 analog input |
| ADC Ch 4 | Conductivity (DFR0300) | 1 analog input |
| ADC Ch 5 | ORP (SEN0165) | 1 analog input |
| OneWire bus | DS18B20 × 2 (primary + backup) | 1 digital pin (shared bus) |
| GPIO (Trig) | JSN-SR04T trigger | 1 digital output |
| GPIO (Echo) | JSN-SR04T echo | 1 digital input |
| I2C (SDA/SCL) | BME280 (addr 0x76 or 0x77) | 2 pins (shared bus) |
| **Total** | **10 sensors** | **6 ADC + 4 digital + 2 I2C** |

> [!NOTE]
> STM32F405 provides 16 ADC channels, 4 I2C buses, and 80+ GPIO — ample headroom for the full suite plus future expansion.

---

## 5. Power Budget (Sensor Subsystem Only)

| Sensor | Active Current | Duty Cycle (Stop-and-Sample) | Avg. Current |
| -------- | --------------- | ------------------------------ | ------------- |
| pH signal board | ~5 mA | 100% (always on during mission) | 5 mA |
| Turbidity (IR LED) | ~30 mA | ~5% (reading only) | 1.5 mA |
| TDS signal board | ~5 mA | 100% | 5 mA |
| DS18B20 × 2 | ~1.5 mA each | ~5% | 0.15 mA |
| DO signal board | ~6 mA | 100% | 6 mA |
| Conductivity board | ~5 mA | 100% | 5 mA |
| ORP signal board | ~5 mA | 100% | 5 mA |
| JSN-SR04T | ~30 mA | ~2% | 0.6 mA |
| BME280 | ~0.3 mA | ~1% | ~0 mA |
| **Total sensor draw** | | | **~28.25 mA** |

> [!TIP]
> Sensor subsystem draws negligible current compared to propulsion motors (10–30 A). No special power management needed for sensors — they can remain powered throughout a mission without impacting battery life.

---

## 6. Mechanical Mounting Summary

```
HULL (top)
├── BME280 ..................... internal, away from motor heat
├── JSN-SR04T ................. hull bottom, facing down
│
PROBE MAST (submerged, rigid PVC/3D-printed)
├── Conductivity (K=1) ........ upper mast (fast, least sensitive)
├── TDS ....................... upper mast, few cm from conductivity
├── DS18B20 × 2 .............. mid mast (zip-tied / bracket)
├── Turbidity (SEN0189) ....... mid mast, shroud w/ side slots
├── ORP (SEN0165) ............. mid mast, BNC cable gland
├── pH (SEN0161) .............. lower mast, PVC guard tube
└── DO (SEN0237-A) ............ deepest, wire cage over membrane
```

> [!IMPORTANT]
> All analog sensor signal boards must share a **separate regulated power rail** from ESC/drive motors with a **star-ground topology**. This is a design decision to make before the hull is sealed — analog sensors will pick up ESC switching noise if grounding is daisy-chained.

---

## 7. Cost Summary

| Category | Min (₹) | Max (₹) |
| ---------- | --------- | --------- |
| Sensor modules (×10) | 16,700 | 20,600 |
| Calibration consumables (initial) | 1,100 | 1,850 |
| 3D printing filament | 800 | 1,500 |
| **Grand Total** | **₹18,600** | **₹23,950** |
| **Remaining from ₹30K budget** | **₹6,050** | **₹11,400** |

> [!NOTE]
> Remaining budget covers spare probes (DO membrane caps, pH replacement bulbs), additional calibration solutions for extended field campaigns, cable glands, waterproof connectors, and mounting hardware.

---

## 8. Procurement Checklist

- [ ] pH sensor kit (SEN0161) + pH 4.0 & 7.0 buffers
- [ ] Turbidity sensor kit (SEN0189)
- [ ] TDS sensor kit (SEN0244)
- [ ] DS18B20 waterproof probes × 2
- [ ] DO sensor kit (SEN0237-A) — verify NaOH fill solution is included or source locally
- [ ] Conductivity sensor kit (DFR0300) + 1413 μS/cm calibration standard
- [ ] ORP sensor kit (SEN0165) + cleaning solution
- [ ] JSN-SR04T ultrasonic sensor
- [ ] BME280 breakout board
- [ ] Spare DO membrane cap + electrolyte
- [ ] PETG filament — 1 kg spool (for probe mast, shrouds, guard cages, mounts)
- [ ] 3D print: probe mast, turbidity shroud, DO cage, pH guard, cable gland plate, mounts

---

*BOM-WQM-SENSOR-2026-R1 | Prepared for WQM-Project/WQM*
