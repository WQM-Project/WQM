# IoT Application Specification Report: River Water Quality Monitor with Pollution Prediction

## 1. Link(s) to the IoT application

- **Project Documentation / Source Code**: [WQM-Project/WQM Repository](https://github.com/WQM-Project/WQM)
- **Deployment Domain**: Embedded Systems, Internet of Things (IoT), and Artificial Intelligence/Machine Learning (AI/ML) applied to Health Tech and environmental conservation.

## 2. Problem Statement

Himalayan river ecosystems, encompassing vital water bodies such as the Beas, Uhl, and Suketi rivers, are currently confronting severe ecological degradation. The primary drivers of this environmental crisis include unmanaged construction runoff, escalating tourism waste, and alterations in water flow and sediment due to accelerated glacial melt. This continuous pollution threatens aquatic life, degrades water quality for downstream communities, and disrupts local ecosystems. Currently, manual sampling and lab analysis are inadequate for catching sudden pollution events. There is a critical, unmet need for a robust, real-time, and continuous monitoring system capable of operating in remote, challenging topographies to detect these pollution spikes, analyze water quality trends, and forecast future health indices to empower timely municipal interventions.

## 3. Problem Solution

To effectively combat and monitor the degradation of Himalayan rivers, the project introduces a solar-powered, floating IoT buoy node engineered for continuous, real-time water quality assessment with integrated predictive analytics.

The comprehensive solution consists of:

- **Autonomous On-site Data Collection**: A highly optimized floating sensor buoy is deployed directly into the river, equipped with an array of industrial-grade sensors. It samples essential parameters (pH, turbidity, TDS, temperature, dissolved oxygen, and conductivity) at a customizable interval (defaulting to every 5 minutes). The node performs critical preprocessing, such as median filtering and calibration offsets, directly on the edge.
- **Robust Long-Range Communication**: Recognizing the lack of reliable Wi-Fi or high-bandwidth cellular networks in deep river valleys, the system utilizes LoRa (Long Range) communication for node-to-gateway telemetry, capable of spanning ~2 km line-of-sight.
- **Advanced Predictive ML**: A dedicated gateway (or cloud backend) leverages a Long Short-Term Memory (LSTM) neural network. Trained on extensive Central Pollution Control Board (CPCB) data and augmented with real-time telemetry, this model predicts the Water Quality Index (WQI) 24 hours in advance.
- **Actionable Visualization and Alerting**: A centralized, public-facing web dashboard visualizes the river's health via geographic heatmaps and time-series charts. If the current or predicted WQI breaches established safety thresholds (IS 10500/CPCB norms), the system autonomously dispatches alerts via SMS and WhatsApp to local authorities using the Twilio API.

## 4. System Representation

### System Description

The architecture of this IoT application is a tiered, multi-node infrastructure designed for resilience and scalability in remote environments.

1. **Edge Sensing Layer (The Buoy)**:
   The buoy acts as the primary data acquisition terminal. An internal STM32 MCU governs the system's sleep-wake cycle to conserve power. Upon waking, it reads analog and digital signals from the connected sensor suite, processes the raw signals into physical units, and packages them into a concise payload.
2. **Intermediate Relay Layer (The Gateway)**:
   A shore-mounted gateway, powered by an ESP32 microcontroller, acts as a bridge between the local LoRa network and the global internet. It continuously listens for incoming LoRa packets from one or more deployed buoys, validates the data, appends timestamps, serializes the information into JSON, and uploads it via a 4G LTE cellular connection.
3. **Cloud & Analytics Layer**:
   A Firebase backend securely ingests the data using a cost-effective "Data Bucketing" schema. A Python-based ML pipeline utilizes TensorFlow/Keras for LSTM inference and Facebook Prophet for seasonal trend analysis. These models run on Firebase Cloud Functions, offloading heavy compute from the gateway.
4. **Presentation & Action Layer**:
   A React-based Single Page Application (SPA) subscribes to Firebase's real-time feeds to present the data interactively. Simultaneously, threshold-monitoring algorithms run server-side to trigger Twilio alerts to stakeholders when necessary.

### Block Diagram / Flow Chart Description

- **Data Acquisition**: River Water → [pH, Turbidity, TDS, Temp, DO, Cond Sensors]
- **Edge Processing**: Sensors → [STM32 MCU (ADC Conversion, Median Filtering)]
- **Telemetry Transmission**: [STM32] → [LoRa SX1278 TX] ≈≈≈(868/915 MHz RF)≈≈≈> [LoRa SX1278 RX]
- **Gateway Aggregation**: [LoRa RX] → [ESP32 Gateway] → [SIM7600E 4G LTE Module]
- **Cloud Ingestion**: [4G Module] → [Internet] → [Firebase Firestore DB]
- **Data Analytics Pipeline**: [Firestore DB] ↔ [Cloud Functions: LSTM WQI Prediction & Prophet Trend Analysis]
- **Output 1 (Visualization)**: [Firestore DB] → [React SPA Dashboard (Leaflet.js Heatmaps, Recharts)]
- **Output 2 (Alerting)**: [Cloud Functions] → [Twilio API] → [SMS/WhatsApp to Authorities]

## 5. Tools, Sensors, and Equipment

### Sensors (Transducers)

- **pH Sensor**: DFRobot SEN0161. Analog industrial-grade glass electrode. Range: 0–14 pH. Accuracy: ±0.1. Employs two-point calibration.
- **Turbidity Sensor**: DFRobot SEN0189. Infrared optical sensor. Range: 0–3000 NTU. Outputs an analog voltage mapped to NTU via a polynomial curve.
- **TDS Sensor**: Gravity Analog TDS Sensor. Range: 0–1000 ppm. Features temperature compensation using the DS18B20.
- **Temperature Sensor**: DS18B20. Digital OneWire, waterproof stainless steel probe. Range: -55 °C to +125 °C. Accuracy: ±0.5 °C.
- **Dissolved Oxygen (DO) Sensor**: DFRobot Gravity Analog DO Sensor. Galvanic probe. Range: 0–20 mg/L. Calibrated in air-saturated water.
- **Conductivity Sensor**: Gravity Analog Conductivity Sensor (K=1). Range: 0–20 ms/cm. Cross-references TDS.

### Embedded Hardware & Microcontrollers

- **Edge Node MCU**: STM32 (e.g., STM32F405). Chosen for its high-precision hardware Analog-to-Digital Converter (ADC), ensuring accurate analog sensor readings, and excellent low-power sleep capabilities.
- **Gateway MCU**: ESP32. Handles LoRa reception, packet validation, JSON serialization, and communication with the 4G modem.

### Communication Modules

- **LoRa Module**: SX1278 (433/868 MHz). SPI-interfaced to the MCU. Provides long-range RF telemetry.
- **Cellular Module**: SIM7600E 4G LTE Module. Interfaced via UART AT commands to provide internet backhaul.

### Power & Energy Equipment

- **Solar Panel**: 6V 3W monocrystalline panel, mast-mounted.
- **Charge Controller**: CN3791 MPPT (Maximum Power Point Tracking) optimized for 6V panels.
- **Battery**: 3.7V 3000 mAh 18650 LiPo cell.

### Enclosures & Mechanical

- **Buoy Housing**: 110 mm diameter PVC pipe, sealed with end caps, marine silicone, and equipped with a foam collar for buoyancy and cable glands for probes.
- **Gateway Housing**: IP65-rated ABS junction box, suitable for pole mounting on the shore.

### Software Stack & Cloud Tools

- **Firmware Development**: PlatformIO using Arduino framework or STM32Cube.
- **Cloud Database**: Firebase Firestore.
- **Machine Learning**: Python 3.10, TensorFlow/Keras (LSTM), Facebook Prophet.
- **Frontend Framework**: React 18 (Vite) with Leaflet.js and Recharts.
- **Notification Service**: Twilio API.

## 6. Reported Specifications

### Communication Protocols and Systems

- **Edge-to-Gateway (LoRa Telemetry)**:
  - **Technology**: LoRa (Long Range RF).
  - **Operating Frequency**: 868/915 MHz (or 433 MHz), depending on regional ISM band regulations.
  - **Modulation Details**: Configurable Spread Factor (SF7–SF12) to dynamically trade-off between transmission range and data rate.
  - **Effective Range**: Approximately 2 km Line-of-Sight (LoS) and 500 m Non-Line-of-Sight (NLOS) within valley topographies.
  - **Data Payload**: Custom lightweight packet format: `[node_id][timestamp][pH][turb][TDS][temp][DO][cond][checksum]`.
- **Gateway-to-Cloud (Cellular Backhaul)**:
  - **Technology**: 4G LTE cellular data.
  - **Protocol**: HTTP/HTTPS requests containing JSON payloads.
  - **Bandwidth Requirement**: Low bandwidth; a standard 1.5 GB/day cellular plan is more than sufficient for high-frequency telemetry.

### Power Source and Efficiency Specifications

- **Energy Harvesting**: The system is entirely off-grid, utilizing a 6V 3W monocrystalline solar panel to harvest ambient solar energy.
- **Energy Storage**: Energy is buffered in a 3.7V 3000 mAh 18650 LiPo battery.
- **Power Conditioning**: A CN3791 MPPT charge controller maximizes the energy extracted from the solar panel, regardless of varying light conditions.
- **Power Management & Autonomy**: The system relies heavily on the deep-sleep capabilities of the STM32 and ESP32 microcontrollers. By remaining dormant and only waking every 5 minutes to sample and transmit, the average current draw is drastically minimized. This strategy grants the buoy approximately 48 hours of autonomous operation during heavily overcast conditions or night cycles without active solar charging.

## 7. Extended Architectural and Analytical Insights

### Database Optimization Strategy

The architecture utilizes a "Data Bucketing" pattern in Firebase Firestore. Instead of creating a new database document for every 5-minute reading (which would result in 288 writes per day per node), the gateway aggregates these readings into a single daily document. This dramatically reduces database write operations and costs, and streamlines data retrieval for rendering time-series charts on the frontend.

### Dual-Tier Machine Learning Strategy

The application employs a sophisticated dual-model approach to predictive analytics:

1. **Short-Term Tactical Prediction**: The Long Short-Term Memory (LSTM) network (configured with 2 stacked layers of 64 and 32 units, and a 0.2 dropout rate) is optimized for short-term, non-linear time-series forecasting. It predicts the WQI 24 hours ahead, providing immediate tactical alerts for sudden pollution events (e.g., a toxic spill or sudden runoff).
2. **Long-Term Strategic Analysis**: Facebook Prophet is utilized on the backend for seasonal decomposition and changepoint detection. This provides a macroscopic view of river health trends over weeks or months, identifying gradual degradation and assisting municipal authorities in long-term environmental planning.
