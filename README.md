# IoT River Water Quality Monitor with Pollution Prediction

**Technical Domain:** Embedded Systems / IoT + AI/ML  
**Theme Fit:** Build for Himalaya / Health Tech

---

## Detailed Description

Himalayan rivers — Beas, Uhl, Suketi — face degradation from construction runoff, tourism waste, and glacial melt. This project deploys a solar-powered floating IoT buoy for continuous, real-time water quality monitoring with predictive analytics.

### Core Functionality

**Sensing:** The buoy node continuously samples six parameters — pH, turbidity, TDS, temperature, dissolved oxygen (DO), and conductivity — at configurable intervals (default: every 5 minutes). Raw analog/digital readings are preprocessed on-node (median filtering, calibration offset) before transmission.

**Communication Pipeline:** Sensor data is packetized and transmitted via LoRa (868/915 MHz, ~2 km line-of-sight range) to a shore-based gateway. The gateway aggregates packets from multiple buoy nodes, timestamps them, and pushes structured JSON payloads to Firebase Realtime Database over a 4G cellular backhaul. LoRa was chosen over Wi-Fi/BLE for its long range and low power draw — critical for remote riverine deployment.

**Predictive ML:** An LSTM (Long Short-Term Memory) network runs inference on the gateway (or a cloud function) to predict Water Quality Index (WQI) trends 24 hours ahead. The model is trained on historical WQI datasets from CPCB (Central Pollution Control Board) open data, augmented with real-time sensor readings as the system accumulates field data. Prophet is used as a secondary model for trend decomposition and seasonality detection.

**Dashboard:** A public-facing web dashboard renders a geographic heatmap of river health along its length using geo-tagged sensor data. Users can view real-time values, historical trends, and predicted WQI trajectories per station.

**Alerting:** When computed WQI drops below the safe threshold (as defined by IS 10500 / CPCB norms), the system dispatches WhatsApp and SMS alerts to registered local authorities via Twilio API, including the affected location, current parameter values, and predicted recovery time.

### Why It Fits

- Extends prior work on silt-free water systems and lab-scale flume studies into real riverine deployment.
- Directly monitors the rivers IIT Mandi campus sits beside — "Build for Himalaya" in the most literal sense.
- LSTM-based WQI prediction is a publishable research contribution with practical municipal value.

---

## Tech Stack (Detailed)

### 1. Sensor Node (Buoy)

| Layer | Component | Details |
|-------|-----------|---------|
| **MCU** | STM32 (e.g., STM32F405) | High-precision hardware ADC for accurate analog sensor readings. Handles median filtering and LoRa TX. Deep-sleep between sampling cycles for power savings. |
| **pH** | DFRobot SEN0161 (analog) | Industrial-grade glass electrode, 0–14 pH range, ±0.1 accuracy. Requires BNC connector and signal conditioning board. Two-point calibration (pH 4.0 & 7.0 buffer solutions). |
| **Turbidity** | SEN0189 (analog) | Infrared optical sensor, 0–3000 NTU. Output: 0–4.5 V analog mapped to NTU via polynomial curve fit. |
| **TDS** | Gravity Analog TDS Sensor | Measures total dissolved solids (0–1000 ppm). Temperature-compensated using DS18B20 reading. |
| **Temperature** | DS18B20 (OneWire, waterproof) | -55 °C to +125 °C, ±0.5 °C accuracy. Stainless steel probe for submersion. Used both as a standalone parameter and for TDS/pH temperature compensation. |
| **Dissolved Oxygen** | DFRobot Gravity Analog DO Sensor | Galvanic probe, 0–20 mg/L range. Requires single-point calibration in air-saturated water. Most expensive sensor — critical for aquatic ecosystem health assessment. |
| **Conductivity** | Gravity Analog Conductivity Sensor (K=1) | 0–20 ms/cm range. Cross-validates TDS readings (TDS ≈ 0.5 × Conductivity). |

### 2. Communication

| Layer | Component | Details |
|-------|-----------|---------|
| **Node → Gateway** | LoRa SX1278 (433/868 MHz) | SPI-interfaced to ESP32. Spread factor SF7–SF12 configurable (trade range vs. data rate). Packet format: `[node_id][timestamp][pH][turb][TDS][temp][DO][cond][checksum]`. Range: ~2 km LoS, ~500 m NLOS in valley terrain. |
| **Gateway MCU** | ESP32 (dedicated) | Runs LoRa RX, packet validation, JSON serialization, and Firebase push. Also hosts the ML inference engine. |
| **Gateway → Cloud** | UART 4G LTE Module (SIM7600E) | Interfaced to ESP32 hardware serial via AT commands. Provides cellular backhaul where Wi-Fi is unavailable. SIM with 1.5 GB/day plan is sufficient for sensor telemetry. |

### 3. Power System

| Component | Details |
|-----------|---------|
| **Solar Panel** | 6V 3W monocrystalline. Mounted on buoy mast, angled for max insolation. |
| **Charge Controller** | CN3791 MPPT (Maximum Power Point Tracking) solar charge controller designed for 6V panels to maximize efficiency. |
| **Battery** | 3.7V 3000 mAh 18650 LiPo cell. Provides ~48h backup in overcast conditions with 5-min sample intervals and ESP32 deep-sleep. |

### 4. Enclosure

| Component | Details |
|-----------|---------|
| **Buoy body** | PVC pipe (110 mm diameter) sealed with end caps and marine silicone. Foam collar for buoyancy. Sensor probes extend below waterline through cable glands. |
| **Shore gateway** | IP65 ABS junction box, pole-mounted. Houses gateway ESP32, 4G dongle, and backup battery. |

### 5. Backend & Cloud

| Layer | Technology | Details |
|-------|------------|---------|
| **Database** | Firebase Firestore | Uses a 'Data Bucketing' pattern (e.g., grouping a day's 288 readings into a single document) to drastically reduce database reads/writes and optimize for time-series charts. |
| **ML Training** | Python 3.10 + TensorFlow/Keras | LSTM architecture: 2 stacked LSTM layers (64 → 32 units), dropout 0.2, dense output for 24h WQI forecast. Trained on CPCB historical WQI CSV data (~5 years, 50+ stations). Feature inputs: 6 sensor params + hour-of-day + day-of-year. |
| **ML Inference** | Firebase Cloud Functions | Cloud Function triggers on new data writes, runs the full LSTM model, and writes prediction back to DB. Offloads heavy time-series computation from the Edge Gateway. |
| **Trend Analysis** | Facebook Prophet (Python) | Used server-side for seasonal decomposition, changepoint detection, and long-horizon (7-day) trend visualization. Complements LSTM's short-term (24h) predictions. |

### 6. Frontend Dashboard

| Layer | Technology | Details |
|-------|------------|---------|
| **Framework** | React 18 (Vite) | SPA with real-time data subscription via Firebase SDK `onValue()` listeners. |
| **Mapping** | Leaflet.js + leaflet-heat plugin | Renders river path as a polyline with color-coded WQI heatmap overlay. Marker popups show per-node real-time values and sparkline charts. |
| **Charts** | Recharts or Chart.js | Time-series plots for each parameter (last 24h, 7d, 30d). Prediction overlay shows LSTM forecast with confidence band. |
| **Hosting** | Firebase Hosting | Free tier, CDN-backed. Custom domain optional. |

### 7. Alerting

| Layer | Technology | Details |
|-------|------------|---------|
| **SMS/WhatsApp** | Twilio API | Free tier supports ~1000 messages/month. Alert payload includes: location, parameter breach, current WQI, predicted recovery time. Firebase Cloud Function triggers alert on WQI threshold breach. |
| **Recipient Management** | Firestore collection | Stores phone numbers, alert preferences, and notification history per authority/user. |

### 8. Firmware & DevOps

| Layer | Technology | Details |
|-------|------------|---------|
| **Firmware** | Arduino framework or STM32Cube (PlatformIO) | Modular codebase: `sensors.h` (precision ADC reads + calibration), `lora.h` (packet TX/RX), `power.h` (sleep scheduling). |
| **Version Control** | Git + GitHub | Monorepo: `/firmware`, `/gateway`, `/ml`, `/dashboard`. |
| **Data Pipeline** | Python scripts (scheduled via cron / Cloud Scheduler) | Periodic CPCB data scraping, model retraining, and accuracy logging. |
