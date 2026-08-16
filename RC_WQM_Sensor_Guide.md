# RC Water Quality Boat — Full Sensor Guide (Stop-and-Sample Method)

**Method assumed:** Boat navigates to a waypoint, halts for ~15–20 seconds while sensors settle and take readings, then moves to the next point. This is the standard approach used by real hydrographic survey boats and is the cheapest way to get accurate readings on a moving platform without expensive fast-response sensors.

---

## 1. pH Sensor

**Recommended: DFRobot SEN0161 (Analog pH Meter Kit, glass bulb electrode)**

| Spec | Value |
|---|---|
| Range | 0–14 pH |
| Accuracy | ±0.1 pH |
| Interface | Analog (needs ADC) |
| Response time | 30–60 sec to stabilize (fine for stop-and-sample) |
| Cost | ₹800–1,200 |

**Pros**
- Cheap, widely available (Robu/Robocraze), well-documented with Arduino/STM32 libraries.
- Two-point calibration is simple and doesn't need lab equipment — just standard pH 4 and pH 7 buffer solutions.
- Response time is a non-issue once you're doing stop-and-sample (you weren't going to beat 30 sec anyway).

**Cons**
- Glass bulb is physically fragile — a knock against a rock, weed, or the hull during transit can crack it.
- Drifts over weeks; needs periodic recalibration to stay accurate.
- Slight temperature sensitivity — pair with your DS18B20 reading for compensation in firmware.

**Mounting requirements**
- Mount on a **rigid probe mast** extending below the hull, not a free-hanging wire — free-hanging probes swing and hit the hull/prop during acceleration and turns.
- Recess the bulb slightly inside a perforated PVC guard tube so it's protected from lateral impact but still fully wetted.
- Keep it forward of the prop, away from wake turbulence, ideally near the bow-centerline where flow is cleanest at a stop.
- Route cable up through a sealed cable gland into the hull; use marine epoxy or silicone at the entry point.

**Human intervention needed?**
- Yes — recalibrate every 3–4 weeks with buffer solutions (this is unavoidable with any glass-bulb probe at this price point).
- Visual inspection each deployment for cracks/debris on the bulb.
- No intervention needed *during* a run — it operates autonomously once mounted.

---

## 2. Turbidity Sensor

**Recommended: DFRobot SEN0189 (Analog Turbidity Sensor, IR optical)**

| Spec | Value |
|---|---|
| Range | 0–3000 NTU (clips beyond this — acceptable for river monitoring, flag as a known limitation for extreme flash-flood turbidity) |
| Interface | Analog |
| Response time | Near-instant (optical) |
| Cost | ₹700–950 |

**Pros**
- Fast response — genuinely not a problem even outside stop-and-sample.
- Cheap, simple analog output, easy polynomial-curve calibration against known NTU standards.
- No moving parts, nothing to wear out chemically.

**Cons**
- Optical (IR scatter) sensors are fooled by air bubbles — prop wash and bow wave will read as false turbidity spikes if sampled while moving.
- Lens can accumulate a biofilm/silt coating over days, causing slow drift.
- No self-cleaning at this price point.

**Mounting requirements**
- Mount on the same probe mast as pH, but add a **3D-printed or PVC shroud/baffle** with side slots around the optical window — this blocks direct bubble ingress from the bow wave while still letting river water flow through.
- Because you're stopping to sample, wait 3–5 seconds after the boat comes to rest before logging the turbidity reading, to let residual bubbles clear — this is a free firmware fix (just a delay in your sampling routine).
- Keep the sensor face oriented downward/sideways, not directly into oncoming flow.

**Human intervention needed?**
- Periodic manual wipe of the lens (every 1–2 weeks depending on water conditions) — no automated wiper needed if you're doing stop-and-sample with the bubble-delay trick, since fouling accumulates slower without continuous flow scouring debris onto the lens.
- No intervention during a run.

---

## 3. TDS (Total Dissolved Solids) Sensor

**Recommended: Gravity Analog TDS Sensor Kit**

| Spec | Value |
|---|---|
| Range | 0–1000 ppm |
| Interface | Analog |
| Requires | Temperature compensation (pair with DS18B20) |
| Cost | ₹300–450 |

**Pros**
- Very cheap, plug-and-play with common Arduino/STM32 libraries.
- Direct ppm reading is more intuitive for your dashboard/report than a derived value.
- Fast response — works fine even without stop-and-sample, but benefits from the still-water reading like the others.

**Cons**
- TDS and conductivity measure overlapping information (TDS is derived from conductivity via a conversion factor in most real instruments) — so you're paying for a second sensor that's electrically similar to your conductivity sensor. This is a **legitimate engineering redundancy** if your budget allows it (having both gives you a cross-check/validation point, which is actually a nice thing to mention in a report), but know that if budget tightens again, this is the first sensor to cut.
- Like other analog probes, needs temperature compensation for accuracy — uncompensated readings can be off by several percent per °C deviation from 25°C.

**Mounting requirements**
- Can share the same probe mast/mount as conductivity since they sense similar things — keep them a few cm apart to avoid electrical cross-talk between the two analog circuits.
- Standard cable gland + silicone seal at hull entry.

**Human intervention needed?**
- Occasional probe cleaning (mineral buildup on the electrode) every few weeks.
- No intervention during a run.

---

## 4. Temperature Sensor

**Recommended: DS18B20 (Digital, waterproof, stainless steel probe)**

| Spec | Value |
|---|---|
| Range | -55°C to +125°C |
| Accuracy | ±0.5°C |
| Interface | Digital (OneWire) — no ADC channel needed |
| Response time | Fast (few seconds) |
| Cost | ₹150–200 |

**Pros**
- Best sensor on this whole list in terms of cost-to-reliability ratio. Digital output means no analog noise issues, no calibration curve needed, no drift concerns.
- Fast enough that it works fine even if you ever move away from stop-and-sample.
- Does double duty — feeds temperature compensation into your pH, DO, and conductivity readings.

**Cons**
- Genuinely none significant for this application at this price point.

**Mounting requirements**
- Simplest sensor to mount — just needs to be in direct water contact. Can be zip-tied or bracket-mounted to the same probe mast as the others, no special protection needed beyond a normal cable gland.

**Human intervention needed?**
- None. Fit-and-forget.

---

## 5. Dissolved Oxygen (DO) Sensor

**Recommended: DFRobot Gravity Analog DO Sensor Kit (galvanic probe)**

| Spec | Value |
|---|---|
| Range | 0–20 mg/L |
| Interface | Analog |
| Response time | Slowest sensor in the suite (30–90 sec) — this is the single biggest reason you need stop-and-sample |
| Cost | ₹3,500–4,200 |

**Pros**
- Cheapest DO technology available; optical/luminescence DO sensors (the "better" alternative) cost 3–4x more and would blow your budget.
- Accurate enough for environmental trend monitoring (not lab-grade, but sufficient for a WQI proxy).

**Cons**
- Slowest-responding sensor in your whole suite — this alone is the main justification for the stop-and-sample method over continuous sampling.
- Membrane consumes electrolyte over time and needs periodic replacement — this is a genuine recurring maintenance cost and the biggest reliability risk in the entire system.
- Vulnerable to physical damage/snagging from weeds, branches, or impact since the membrane tip is delicate.

**Mounting requirements**
- **Rigid guard cage** (bent wire mesh or 3D-printed cage) around the membrane tip — non-negotiable for a moving platform, since this is your most fragile and most expensive sensor.
- Mount deepest on the probe mast (DO sensors read best fully submerged and away from surface aeration/bubbles).
- Increase your stop-and-sample dwell time specifically for this sensor if needed — consider logging DO 10–15 seconds *after* the other sensors have already taken their readings, so the boat has been stationary longer by the time DO stabilizes. This is a free firmware/timing fix.

**Human intervention needed?**
- Yes — the most maintenance-heavy sensor in the system. Membrane/electrolyte replacement every 2–3 weeks depending on water conditions (silty Himalayan rivers may foul it faster than clean water).
- Calibration in air-saturated water before each deployment session is good practice.
- No intervention needed mid-run.

---

## 6. Conductivity Sensor

**Recommended: Gravity Analog Conductivity Sensor, K=1 cell constant**

| Spec | Value |
|---|---|
| Range | 0–20 ms/cm |
| Interface | Analog |
| Response time | Fast (ms-scale) — least affected by movement of any sensor here |
| Cost | ₹1,200–1,800 |

**Pros**
- Fast, stable, minimal maintenance compared to pH/DO.
- Cross-validates against your TDS sensor.
- Handles the stop-and-sample cadence with margin to spare — could even be sampled continuously if you ever wanted higher-resolution spatial data along the route.

**Cons**
- K=1 cell constant is tuned for mid-range conductivity; Himalayan glacial meltwater tends to be low-ionic-strength (low conductivity), so you may be operating toward the lower-resolution end of the sensor's range. A software calibration curve fitted with a few known-conductivity reference solutions (cheap, DIY) mitigates this without needing to buy a K=0.1 variant.
- Electrode can accumulate mineral/biofilm buildup over weeks.

**Mounting requirements**
- Same probe mast as TDS, kept a few cm apart electrically.
- Standard cable gland seal.

**Human intervention needed?**
- Periodic electrode cleaning (monthly) and occasional recalibration against reference solutions.
- No intervention mid-run.

---

## 7. GPS Module (new addition for mobile platform)

**Recommended: NEO-6M or NEO-M8N GPS Module**

| Spec | Value |
|---|---|
| Interface | UART |
| Accuracy | 2.5m (NEO-6M) / 2m (NEO-M8N, faster lock) |
| Cost | ₹350–500 (6M) / ₹700–900 (M8N) |

**Pros**
- Essential once you're mobile — geotags every stop-and-sample reading so your WQI values plot correctly on the dashboard heatmap instead of just being a time-series with no spatial meaning.
- NEO-6M is budget-friendly and sufficient for river-width-scale positioning accuracy.
- Enables autonomous waypoint navigation later if you want the boat to self-navigate a sampling grid instead of manual RC control.

**Cons**
- Needs clear sky view — under dense tree canopy in a river valley, lock time and accuracy can degrade. Mount the antenna on the highest point of the hull, unobstructed.
- NEO-6M has slower cold-start lock (30s–1min) — budget this into your pre-mission checklist, not your sampling loop.

**Mounting requirements**
- External antenna (usually included) mounted on top of the hull, away from the ESC/motor to avoid EMI.
- Weatherproof housing or conformal coating on the board itself if not already enclosed.

**Human intervention needed?**
- None during operation. Occasional firmware check that lock is achieved before starting a run.

---

## 8. Depth/Flow Sensor (optional but recommended addition)

**Recommended: JSN-SR04T Waterproof Ultrasonic Sensor**

| Spec | Value |
|---|---|
| Range | 20cm–600cm |
| Interface | Digital (trigger/echo, like standard ultrasonic) |
| Cost | ₹250–400 |

**Pros**
- Very cheap way to log relative water depth/flow context at each stop point, which helps distinguish a genuine pollution event from a fouled-sensor artifact (e.g., turbidity spike + flow surge = probably real; turbidity spike + no flow change = probably a fouled lens).
- No consumables, minimal maintenance.

**Cons**
- Not a precision flow meter — gives depth/relative change, not true flow rate in m/s. Fine for context, not for hydrological-grade measurement.

**Mounting requirements**
- Mount facing downward from the hull bottom or a side bracket, clear of the prop wash.

**Human intervention needed?**
- None.

---

## Cross-Cutting Notes for the Whole Suite

**Electrical isolation (free, but easy to forget):**
Put the sensor ADC/MCU on a separate regulated rail from the ESC/drive motor, with a proper star-ground topology rather than daisy-chained grounds. Your analog sensors (pH, turbidity, TDS, DO, conductivity) are all low-signal and will pick up ESC switching noise if grounding is sloppy — this is a design decision to make before the hull is sealed up, since it's painful to retrofit afterward.

**Probe mast — one shared mechanical structure:**
Rather than mounting each sensor independently, build a single rigid mast/rack (PVC or 3D-printed) that hangs below the hull and carries all the wetted sensors (pH, turbidity, DO, conductivity, TDS, temperature) in fixed relative positions, with the ultrasonic sensor and GPS mounted separately on/above the hull. This is both cheaper (one mounting structure, not six) and mechanically stronger against snagging/impact than individual probes.

**Stop-and-sample firmware sequencing (suggested order per stop):**
1. Boat halts, wait 3–5 sec for bubble/wake clearance
2. Read: temperature, conductivity, TDS, turbidity, GPS fix (fast sensors first)
3. Wait remaining time for pH to stabilize (~30–60 sec total dwell)
4. Read pH
5. Read DO last (slowest sensor — by now it's had the most settling time)
6. Log ultrasonic depth
7. Package payload, resume navigation

**Human intervention summary table:**

| Sensor | Intervention frequency | Type |
|---|---|---|
| pH | Every 3–4 weeks | Buffer recalibration + bulb inspection |
| Turbidity | Every 1–2 weeks | Manual lens wipe |
| TDS | Every few weeks | Electrode cleaning |
| Temperature | None | — |
| DO | Every 2–3 weeks | Membrane/electrolyte replacement + calibration |
| Conductivity | Monthly | Electrode cleaning + recalibration |
| GPS | None | — |
| Ultrasonic | None | — |

DO is your maintenance bottleneck — if you're ever deploying for an extended unattended period, that's the sensor that will fail first. For an RC-boat use case where a human is present at every deployment anyway (unlike the static buoy), this is much less of a concern than it would've been for the autonomous buoy version.

**Approximate total sensing subsystem cost:** ₹8,000–10,500 (includes GPS and ultrasonic additions, keeps you well within the ₹50k overall project budget).
