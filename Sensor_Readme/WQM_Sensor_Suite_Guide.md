# WQM Sensor Suite — Finalized Selection & Tiered Pricing Guide

> **Project:** River Water Quality Monitor with Pollution Prediction  
> **Sensor Budget Ceiling:** ₹30,000  
> **Finalized Suite Cost:** ₹16,700–20,600  
> **Method:** Stop-and-Sample (15–20 sec dwell per waypoint)  
> **Date:** August 2026

---

## Finalized Sensor Suite (10 Sensors)

### Budget Overview

| # | Parameter | Selected Sensor | Interface | Cost (₹) |
| --- | ----------- | ---------------- | ----------- | ---------- |
| 1 | pH | DFRobot SEN0161 | Analog | 800–1,200 |
| 2 | Turbidity | DFRobot SEN0189 | Analog | 700–950 |
| 3 | TDS | Gravity Analog TDS | Analog | 300–450 |
| 4 | Temperature | DS18B20 (primary) | Digital (OneWire) | 150–200 |
| 5 | Dissolved Oxygen | DFRobot Gravity Analog DO | Analog | 3,500–4,200 |
| 6 | Conductivity | Gravity Analog (K=1) | Analog | 1,200–1,800 |
| 7 | **ORP** | **DFRobot SEN0165** | **Analog** | **9,400–10,800** |
| 8 | Depth | JSN-SR04T Ultrasonic | Digital (Trigger/Echo) | 250–400 |
| 9 | **Environmental** | **BME280** | **I2C** | **250–400** |
| 10 | **Temperature (backup)** | **DS18B20** | **Digital (OneWire)** | **150–200** |
| | **TOTAL** | | | **₹16,700–20,600** |

**ADC channels used:** 6 analog (pH, turbidity, TDS, DO, conductivity, ORP)  
**Digital pins used:** 2 (OneWire bus for DS18B20s, trigger/echo for ultrasonic)  
**I2C devices:** 1 (BME280)  
**STM32F405 capacity:** 16 ADC channels, multiple I2C/UART — ample headroom.

---

## Detailed Sensor Profiles with Tiered Pricing

Each sensor below includes three pricing tiers:

- 🟢 **Budget** — Cheapest functional option (generic/clone)
- 🟡 **Mid (Recommended)** — Best cost-to-reliability for this project
- 🔴 **Premium** — Lab/industrial grade (for reference or Stage 2/3 upgrade)

---

### 1. pH Sensor

**Selected: DFRobot SEN0161 (Mid tier)**

| Spec | Value |
| ------ | ------- |
| Range | 0–14 pH |
| Accuracy | ±0.1 pH |
| Response time | 30–60 sec (fine for stop-and-sample) |
| Interface | Analog (BNC probe + signal board) |
| Calibration | Two-point (pH 4.0 and pH 7.0 buffer solutions) |

| Tier | Option | Price (₹) | Notes |
| ------ | -------- | ----------- | ------- |
| 🟢 Budget | Generic analog pH module (IndiaMART) | 400–700 | Same BNC probe type. Less documentation, shorter probe life. Functional for prototyping. |
| 🟡 **Mid ★** | **DFRobot SEN0161** | **800–1,200** | **Well-documented, Arduino/STM32 libraries, reliable. Standard choice for maker WQ projects.** |
| 🔴 Premium | Atlas Scientific EZO-pH Kit | 17,000–18,000 | Lab-grade (±0.002 pH), I2C/UART, non-volatile calibration storage. Overkill for field screening. |

**Mounting:** Rigid probe mast, recessed inside perforated PVC guard tube. Forward of prop, bow-centerline.  
**Maintenance:** Recalibrate every 3–4 weeks with buffer solutions. Visual inspection each deployment.

---

### 2. Turbidity Sensor

**Selected: DFRobot SEN0189 (Mid tier)**

| Spec | Value |
| ------ | ------- |
| Range | 0–3,000 NTU |
| Response time | Near-instant (optical/IR scatter) |
| Interface | Analog |

| Tier | Option | Price (₹) | Notes |
| ------ | -------- | ----------- | ------- |
| 🟢 Budget | Generic IR turbidity module | 350–550 | Same optical principle. More noise, less consistent polynomial curve. |
| 🟡 **Mid ★** | **DFRobot SEN0189** | **700–950** | **Proven polynomial-curve calibration. No moving parts. Standard choice.** |
| 🔴 Premium | DFRobot SEN0554 (Industrial RS485) | 15,000–20,000 | RS485/Modbus, self-cleaning, IP68. For permanent deployment. |

**Mounting:** Probe mast with 3D-printed shroud/baffle (side slots) around optical window. Sensor face oriented downward/sideways.  
**Firmware trick:** Wait 3–5 sec after boat halts before reading — lets residual bubbles clear. Free accuracy improvement.  
**Maintenance:** Manual lens wipe every 1–2 weeks.

---

### 3. TDS (Total Dissolved Solids) Sensor

**Selected: Gravity Analog TDS Sensor Kit (Budget/Mid tier — same product)**

| Spec | Value |
| ------ | ------- |
| Range | 0–1,000 ppm |
| Interface | Analog |
| Requires | Temperature compensation (DS18B20) |

| Tier | Option | Price (₹) | Notes |
|------|--------|-----------|-------|
| 🟢🟡 **Budget/Mid ★** | **Gravity Analog TDS Kit** | **300–450** | **Cheapest sensor in the suite. Redundant with conductivity but provides cross-validation.** |
| 🔴 Premium | Atlas Scientific EZO-EC (TDS mode) | 25,000–27,000 | Conductivity circuit with TDS output mode. Lab-grade. |

**Mounting:** Probe mast, few cm from conductivity probe to avoid electrical cross-talk.  
**Note:** TDS is derived from conductivity (TDS ≈ EC × conversion factor). Having both is intentional engineering redundancy — if one probe fouls, the other flags the discrepancy.

---

### 4. Temperature Sensor (Primary + Backup)

**Selected: DS18B20 × 2 (Budget/Mid tier — same product)**

| Spec | Value |
| ------ | ------- |
| Range | -55°C to +125°C |
| Accuracy | ±0.5°C |
| Interface | Digital (OneWire) — no ADC channel consumed |
| Response time | Few seconds |

| Tier | Option | Price (₹) | Notes |
|------|--------|-----------|-------|
| 🟢🟡 **Budget/Mid ★** | **DS18B20 waterproof probe** | **150–200 each** | **Best cost-to-reliability ratio in the entire suite. Digital = no analog noise, no calibration drift.** |
| 🔴 Premium | Atlas Scientific EZO-RTD Kit | 8,000–10,000 | Pt1000 RTD, ±0.1°C. Unnecessary for WQ compensation at this precision level. |

**Mounting:** Primary on probe mast (water contact). Backup on separate bracket or zip-tied to mast as failover.  
**Both share one OneWire bus** — each DS18B20 has a unique 64-bit ROM address; firmware reads both on the same data pin.  
**Why two:** Temperature feeds compensation into pH, DO, conductivity, and TDS. If the single probe fails mid-run, every other reading becomes unreliable. ₹150 insurance.

---

### 5. Dissolved Oxygen (DO) Sensor

**Selected: DFRobot Gravity Analog DO Kit — Galvanic (Mid tier)**

| Spec | Value |
| ------ | ------- |
| Range | 0–20 mg/L |
| Response time | 30–90 sec (**slowest sensor — defines your dwell time**) |
| Interface | Analog |
| Technology | Galvanic (electrochemical) |

| Tier | Option | Price (₹) | Notes |
| ------ | -------- | ----------- | ------- |
| 🟢 Budget | Generic galvanic DO probe + board | 2,000–3,000 | Same principle. Shorter membrane life, less documentation. Risky for field work. |
| 🟡 **Mid ★** | **DFRobot Gravity Analog DO (SEN0237-A)** | **3,500–4,200** | **Cheapest reliable DO option. Galvanic = no warm-up needed. Well-documented.** |
| 🔴 Premium | DFRobot Optical DO or Atlas Scientific EZO-DO | 14,500–30,000 | Optical/luminescence quenching. Near-zero maintenance, no membrane, works in still water. **Recommended upgrade for Stage 2 autonomous.** |

**Mounting:** Deepest on probe mast. Rigid guard cage (wire mesh or 3D-printed) around membrane tip — non-negotiable.  
**Firmware:** Log DO **last** in sampling sequence — by then the boat has been stationary longest, giving DO maximum settling time.  
**Critical with BME280:** Apply barometric pressure compensation:

```
DO_compensated = DO_raw × (temp_factor) × (BME280_pressure / 1013.25)
```

This corrects the 10–22% altitude error at Himalayan deployment sites.  
**Maintenance:** Membrane/electrolyte replacement every 2–3 weeks. **This is your #1 maintenance bottleneck.**

---

### 6. Conductivity Sensor

**Selected: Gravity Analog Conductivity Sensor, K=1 (Mid tier)**

| Spec | Value |
| ------ | ------- |
| Range | 0–20 ms/cm |
| Response time | ms-scale (fastest water sensor in the suite) |
| Interface | Analog |

| Tier | Option | Price (₹) | Notes |
| ------ | -------- | ----------- | ------- |
| 🟢 Budget | Generic EC probe + signal board | 600–1,000 | Functional but more noise, less consistent K-constant. |
| 🟡 **Mid ★** | **Gravity Analog Conductivity (K=1)** | **1,200–1,800** | **Fast, stable, minimal maintenance. Cross-validates TDS.** |
| 🔴 Premium | Atlas Scientific EZO-EC Kit | 25,000–27,000 | Lab-grade, I2C, multiple K-constant probes available (K=0.1 for low-ionic glacial water). |

**Note for Himalayan deployment:** Glacial meltwater has low ionic strength — you'll be operating toward the lower-resolution end of K=1 range. Software calibration curve with known-conductivity reference solutions mitigates this.  
**Mounting:** Probe mast, few cm from TDS probe.

---

### 7. ORP (Oxidation-Reduction Potential) Sensor ✨ NEW

**Selected: DFRobot SEN0165 (Mid tier)**

| Spec | Value |
| ------ | ------- |
| Range | -2,000 mV to +2,000 mV |
| Accuracy | ±10 mV (at 25°C) |
| Response time | ≤20 sec |
| Interface | Analog (BNC probe + signal board) |

| Tier | Option | Price (₹) | Notes |
| ------ | -------- | ----------- | ------- |
| 🟢 Budget | Generic analog ORP module (IndiaMART/AliExpress) | 1,500–4,000 | Same platinum-tip electrochemistry. BNC connector = cross-compatible probes. Less documentation, questionable probe longevity. |
| 🟡 **Mid ★** | **DFRobot SEN0165** | **9,400–10,800** | **Well-documented, zero-calibration button, LED indicator, Gravity ecosystem compatible. The safe choice.** |
| 🔴 Premium | DFRobot SEN0464 (Pro) | 16,000–18,000 | Industrial platinum probe, enhanced durability. For continuous submersion / 24×7 deployment. |

**Why ORP matters (from PMC USV paper):**

| Condition | pH | DO | ORP | Interpretation |
| ----------- | ---- | ---- | ----- | ---------------- |
| Organic sewage | Slightly acidic | Low | **Low (< 200 mV)** | Anaerobic decomposition consuming oxygen |
| Industrial oxidizing waste | Variable | Variable | **High (> 400 mV)** | Chemical oxidizers (bleach, chlorinated compounds) |
| Heavy metal leaching | Low | Low | **Negative (< 0 mV)** | Reducing conditions mobilizing metals from sediment |
| Healthy river | 6.5–8.5 | > 6.5 mg/L | **200–400 mV** | Normal aerobic conditions |

Without ORP, you detect "water is degraded." With ORP, you classify **why**.  
**Mounting:** Probe mast, same depth as pH. Standard BNC + cable gland.  
**Maintenance:** Periodic cleaning of platinum tip. Less maintenance than pH (no membrane/electrolyte).

---

### 8. Depth Sensor

**Selected: JSN-SR04T Waterproof Ultrasonic (Budget/Mid tier — same product)**

| Spec | Value |
|------|-------|
| Range | 20 cm–600 cm |
| Interface | Digital (trigger/echo) |

| Tier | Option | Price (₹) | Notes |
|------|--------|-----------|-------|
| 🟢🟡 **Budget/Mid ★** | **JSN-SR04T** | **250–400** | **Waterproof, no consumables. Gives relative depth changes between sample points for context.** |
| 🔴 Premium | Submersible pressure transducer (stainless steel) | 2,600–5,000 | Absolute hydrostatic depth. Needed for hydrological surveying, not for WQ context. |

**Purpose:** Not a precision depth meter. Provides context to distinguish real pollution events from fouled-sensor artifacts (turbidity spike + depth change = probably real; turbidity spike + no depth change = probably fouled lens).  
**Mounting:** Hull bottom, facing down, clear of prop wash.

---

### 9. Environmental Sensor ✨ NEW

**Selected: BME280 (Budget/Mid tier — same product)**

| Spec | Value |
| ------ | ------- |
| Parameters | Air temperature, humidity, barometric pressure |
| Pressure accuracy | ±1 hPa |
| Interface | I2C (shares bus, no ADC channel consumed) |

| Tier | Option | Price (₹) | Notes |
|------|--------|-----------|-------|
| 🟢🟡 **Budget/Mid ★** | **BME280 breakout board** | **250–400** | **3 environmental parameters for the price of a cable gland. I2C, 3 lines of code, zero maintenance.** |
| 🔴 Premium | BME680 (adds gas resistance / VOC) | 800–1,200 | Adds air quality index. Interesting but not useful underwater. |

**Why this is non-optional at altitude:**

- At sea level: 1,013 hPa → DO saturation = 100% baseline
- At Kullu (1,200m): ~880 hPa → DO saturation = **~87%** baseline
- At Manali (2,050m): ~795 hPa → DO saturation = **~78%** baseline

Without pressure compensation, your DO readings are **systematically over-reported by 13–22%** at deployment sites. BME280 fixes this for ₹300.

**ML model bonus:** Pressure drop → incoming rain → runoff → turbidity/TDS spike. The model sees the *cause* arriving before it hits the water.  
**Mounting:** Inside the hull (no waterproofing needed). Away from motor heat.

---

## Complete Suite Architecture

```
                    ┌─────────────────────────────────┐
                    │         HULL (SEALED)            │
                    │                                  │
                    │  ┌─────────┐    ┌──────────┐    │
                    │  │ STM32   │◄──►│ BME280   │    │
                    │  │ F405    │I2C │(air temp, │    │
                    │  │         │    │ humidity, │    │
                    │  │ 6× ADC  │    │ pressure) │    │
                    │  │ OneWire │    └──────────┘    │
                    │  │ Digital │                     │
                    │  └────┬────┘                     │
                    │       │                          │
                    └───────┼──────────────────────────┘
                            │ Cable glands (sealed)
                    ════════╪══════════════════════════
                      WATER │ LINE
                    ════════╪══════════════════════════
                            │
                    ┌───────┴───────────────────┐
                    │      PROBE MAST           │
                    │   (rigid PVC/3D-printed)   │
                    │                            │
                    │  ┌─ Conductivity (K=1)     │ ← fastest
                    │  ├─ TDS                    │
                    │  ├─ DS18B20 × 2 (primary   │
                    │  │    + backup)             │
                    │  ├─ Turbidity (w/ shroud)   │
                    │  ├─ ORP (SEN0165) ✨ NEW    │
                    │  ├─ pH (w/ PVC guard)       │ ← 30-60s
                    │  └─ DO (w/ cage) ← DEEPEST  │ ← 30-90s
                    │                            │
                    └────────────────────────────┘

          JSN-SR04T ultrasonic ← mounted hull bottom, faces down
```

---

## Stop-and-Sample Firmware Sequence (Updated)

```
1.  Boat halts → wait 3–5 sec (bubble/wake clearance)
2.  Read: BME280 (pressure, air temp, humidity)           ← NEW
3.  Read: DS18B20 × 2 (water temp, cross-check)          ← UPDATED
4.  Read: Conductivity, TDS, Turbidity (fast sensors)
5.  Read: ORP                                             ← NEW (~20s)
6.  Wait remaining time for pH to stabilize (~30–60s total dwell)
7.  Read: pH
8.  Read: DO last (slowest — maximum settling time by now)
9.  Read: JSN-SR04T depth
10. Apply compensations:
    - Temp compensation → pH, DO, conductivity, TDS
    - Pressure compensation → DO                         ← NEW
11. Package payload, resume navigation
```

---

## Sensor Suite Comparison: Price Tiers at a Glance

### 🟢 Budget Build (all cheapest options)

| Sensor | Option | Cost (₹) |
| -------- | -------- | ---------- |
| pH | Generic analog module | 400–700 |
| Turbidity | Generic IR module | 350–550 |
| TDS | Gravity TDS Kit | 300–450 |
| Temperature × 2 | DS18B20 | 300–400 |
| DO | Generic galvanic probe | 2,000–3,000 |
| Conductivity | Generic EC probe | 600–1,000 |
| ORP | Generic analog ORP module | 1,500–4,000 |
| Depth | JSN-SR04T | 250–400 |
| Environmental | BME280 | 250–400 |
| **TOTAL** | | **₹5,950–10,900** |

> [!WARNING]
> Less documentation, shorter probe lives, more noise in analog readings. Acceptable for prototype/proof-of-concept but not recommended for field deployment generating data for ML training.

---

### 🟡 Mid Build (Recommended — selected for this project)

| Sensor | Option | Cost (₹) |
| -------- | -------- | ---------- |
| pH | DFRobot SEN0161 | 800–1,200 |
| Turbidity | DFRobot SEN0189 | 700–950 |
| TDS | Gravity TDS Kit | 300–450 |
| Temperature × 2 | DS18B20 | 300–400 |
| DO | DFRobot Gravity Analog DO | 3,500–4,200 |
| Conductivity | Gravity Analog (K=1) | 1,200–1,800 |
| ORP | DFRobot SEN0165 | 9,400–10,800 |
| Depth | JSN-SR04T | 250–400 |
| Environmental | BME280 | 250–400 |
| **TOTAL** | | **₹16,700–20,600** |

> [!TIP]
> Best balance of reliability, documentation, and cost. Well within ₹30K ceiling with room for consumables/spares.

---

### 🔴 Premium Build (reference / future upgrade path)

| Sensor | Option | Cost (₹) |
| -------- | -------- | ---------- |
| pH | Atlas Scientific EZO-pH | 17,000–18,000 |
| Turbidity | DFRobot Industrial RS485 | 15,000–20,000 |
| TDS | Atlas Scientific EZO-EC (TDS mode) | 25,000–27,000 |
| Temperature × 2 | DS18B20 (adequate at any tier) | 300–400 |
| DO | Atlas Scientific EZO-DO (Optical) | 20,000–30,000 |
| Conductivity | Atlas Scientific EZO-EC | 25,000–27,000 |
| ORP | DFRobot SEN0464 Pro | 16,000–18,000 |
| Depth | Submersible pressure transducer | 2,600–5,000 |
| Environmental | BME680 | 800–1,200 |
| **TOTAL** | | **₹1,21,700–1,46,600** |

> [!NOTE]
> Lab-grade accuracy, I2C ecosystem, near-zero maintenance. For funded research deployments or Stage 2/3 autonomous operation. Way beyond current ₹30K budget.

---

## Parameters Achieved vs. CPCB RTWQMS

| CPCB Parameter | Covered? | How |
| ---------------- | ---------- | ----- |
| pH | ✅ | DFRobot SEN0161 |
| Dissolved Oxygen (DO) | ✅ | DFRobot Gravity DO (pressure-compensated via BME280) |
| Temperature | ✅ | DS18B20 × 2 |
| Electrical Conductivity (EC) | ✅ | Gravity K=1 |
| Turbidity | ✅ | DFRobot SEN0189 |
| TDS | ✅ | Gravity TDS (+ cross-validated with conductivity) |
| ORP | ✅ | DFRobot SEN0165 ✨ |
| BOD | ⚠️ Proxy | Not directly measurable at this budget. DO + ORP + temperature trends serve as a proxy. |
| COD | ❌ | Requires UV spectroscopy (₹1,00,000+). Stage 3. |
| Nitrates | ❌ | ISE sensors ₹40,000+. Stage 3. |
| Dissolved Ammonia | ❌ | ISE sensors ₹40,000+. Stage 3. |
| Chlorides | ⚠️ Proxy | Conductivity serves as a rough proxy for dissolved ion concentration. |
| Total Coliform | ❌ | Biological test — cannot be sensor-measured in-situ. |

> [!IMPORTANT]
> **Coverage: 7/10 CPCB parameters directly measured, 2 more via proxies.** This exceeds most published low-cost WQ monitoring systems, which typically cover only 4–6 parameters.

---

## Research References

1. **PMC USV Paper** — Chen et al., "Development of Autonomous Electric USV for Water Quality Detection," *Sensors* 2025. Uses pH + DO + ORP + EC as the core four parameters for pollution classification. Validates that this parameter set can distinguish organic contamination from industrial discharge.

2. **IIT Bombay** — "An Autonomous Water Quality Monitoring System with Sensors," Chemical Engineering Dept. Demonstrates multi-parameter autonomous monitoring for Indian river conditions.

3. **CPCB RTWQMS** — Central Pollution Control Board Real-Time Water Quality Monitoring System guidelines specify pH, DO, BOD, COD, turbidity, EC, temperature, nitrates, chlorides, and dissolved ammonia as the 10 core parameters.

---

## Human Intervention Summary (Updated)

| Sensor | Intervention Frequency | Type |
| -------- | ---------------------- | ------ |
| pH (SEN0161) | Every 3–4 weeks | Buffer recalibration + bulb inspection |
| Turbidity (SEN0189) | Every 1–2 weeks | Manual lens wipe |
| TDS | Every few weeks | Electrode cleaning |
| Temperature (DS18B20 × 2) | None | Fit-and-forget |
| DO (Galvanic) | Every 2–3 weeks | Membrane/electrolyte replacement + calibration |
| Conductivity (K=1) | Monthly | Electrode cleaning + recalibration |
| **ORP (SEN0165)** | **Monthly** | **Platinum tip cleaning** |
| Depth (JSN-SR04T) | None | Fit-and-forget |
| **BME280** | **None** | **Fit-and-forget (hull-internal)** |
| **Backup DS18B20** | **None** | **Fit-and-forget** |

> [!CAUTION]
> **Maintenance bottleneck:** DO sensor (unchanged). ORP adds minimal maintenance burden (monthly wipe vs. DO's biweekly membrane swap).
