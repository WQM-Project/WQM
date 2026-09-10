# WQM Sensor Configuration Variables

This document details the configuration constants (macros) defined in `main.c` that govern the behavior, calibration, and hardware interfaces for the various sensors used in the Water Quality Monitoring (WQM) project.

## pH Sensor (SEN0161)
These parameters govern the conversion of analog voltage to pH using a standard linear equation: `pH = (Voltage * Slope) + Offset`.
*   **`PH_CAL_SLOPE (-5.70f)`**: The slope of the calibration curve. Represents how much the voltage changes per unit of pH.
*   **`PH_CAL_OFFSET (21.34f)`**: The theoretical pH value when the sensor outputs 0V, calculated via a standard two-point calibration using buffer solutions (pH 4.0 and 7.0).

## Turbidity Sensor (SEN0189)
Turbidity (water cloudiness) is calculated from the voltage drop across an optical sensor using a polynomial equation curve-fit to the sensor's characteristics.
*   **`TURB_COEFF_A (-1120.4f)`**: The quadratic coefficient in the turbidity calculation polynomial.
*   **`TURB_COEFF_B (5742.3f)`**: The linear coefficient.
*   **`TURB_COEFF_C (-4352.9f)`**: The constant/intercept coefficient.

## TDS Sensor (SEN0244)
Total Dissolved Solids (TDS) is also calculated using a polynomial curve fit against the measured analog voltage, factoring in a generic multiplier.
*   **`TDS_COEFF_A (133.42f)`**, **`TDS_COEFF_B (-255.86f)`**, **`TDS_COEFF_C (857.39f)`**: The polynomial coefficients for translating the temperature-compensated voltage curve into a TDS value.
*   **`TDS_FACTOR (0.5f)`**: An approximate empirical conversion factor used to relate electrical conductivity to TDS (specifically for sodium chloride solutions).

## DO (Dissolved Oxygen) Sensor (SEN0237-A)
Dissolved oxygen measurements are highly dependent on temperature. These variables support single-point calibration and temperature compensation.
*   **`DO_CAL1_V (1600)`**: The baseline analog voltage (in millivolts) read from the sensor when submerged in 100% air-saturated water during calibration.
*   **`DO_CAL1_T (25)`**: The temperature (in °C) at which the calibration `DO_CAL1_V` was recorded.
*   **`DO_TEMP_COMP_MV (35)`**: The estimated voltage drift (in mV) per degree Celsius, used to correct the DO reading as water temperature fluctuates.

## Electrical Conductivity (EC) Sensor (DFR0300)
*   **`EC_K_CAL (1.0f)`**: The cell constant (K-factor) calibration multiplier. This is adjusted based on testing against a known standard EC solution (e.g., 1413 µS/cm).
*   **`EC_TEMP_COEFF (0.02f)`**: The temperature compensation coefficient (2% per °C), which normalizes the EC reading to a standard 25°C baseline.

## ORP Sensor (SEN0165)
*   **`ORP_OFFSET (0.0f)`**: A flat millivolt offset applied to the Oxidation-Reduction Potential reading to account for minor hardware biases or probe aging.

## DS18B20 Digital Temperature Sensor
This sensor communicates over the 1-Wire protocol.
*   **`OW_PORT (GPIOE)`** & **`OW_PIN (GPIO_PIN_6)`**: The hardware GPIO pin assigned to the 1-Wire data bus.
*   **Command Codes** (`0xCC`, `0x55`, `0xF0`, `0x44`, `0xBE`): Standard 1-Wire hex commands used by the driver to skip ROM addressing, search for devices, trigger temperature conversions, and read the internal scratchpad memory.

## JSN-SR04T Ultrasonic Distance Sensor
Used for measuring water level via time-of-flight acoustics.
*   **`TRIG_PORT` / `TRIG_PIN`** (`GPIOE`, `Pin 4`): The pin used to trigger the ultrasonic pulse.
*   **`ECHO_PORT` / `ECHO_PIN`** (`GPIOE`, `Pin 5`): The pin used to measure the length of the returning acoustic echo.
*   **`US_TIMEOUT (30000)`**: The maximum time (in microseconds) to wait for an echo before assuming out-of-range (~5 meters).

## BME280 Environmental Sensor
*   **`BME280_ADDR (0x76 << 1)`**: The I2C hardware address of the BME280. Shifted left by 1 bit because the STM32 HAL requires 8-bit aligned I2C addresses.
*   **`BME280_REG_ID (0xD0)`** & **`BME280_REG_CTRL_HUM (0xF2)`**: Key internal register addresses for reading the chip ID and configuring humidity oversampling.
