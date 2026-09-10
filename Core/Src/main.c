/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* ── Sensor data aggregate ──────────────────────────────────────────────────
 */
typedef struct {
  float ph;
  float turbidity_ntu;
  float tds_ppm;
  float do_mgl;
  float ec_uscm;
  float orp_mv;
  float water_temp_1; /* DS18B20 primary   */
  float water_temp_2; /* DS18B20 backup    */
  float depth_cm;     /* JSN-SR04T         */
  float air_temp;     /* BME280            */
  float humidity;     /* BME280            */
  float pressure_hpa; /* BME280            */
} WQM_SensorData_t;

/* ── BME280 calibration data (from chip NVM) ────────────────────────────────
 */
typedef struct {
  uint16_t dig_T1;
  int16_t dig_T2;
  int16_t dig_T3;
  uint16_t dig_P1;
  int16_t dig_P2;
  int16_t dig_P3;
  int16_t dig_P4;
  int16_t dig_P5;
  int16_t dig_P6;
  int16_t dig_P7;
  int16_t dig_P8;
  int16_t dig_P9;
  uint8_t dig_H1;
  int16_t dig_H2;
  uint8_t dig_H3;
  int16_t dig_H4;
  int16_t dig_H5;
  int8_t dig_H6;
} BME280_CalibData_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* ── ADC & voltage ──────────────────────────────────────────────────────────
 */
#define ADC_RESOLUTION 4095.0f /* 12-bit ADC                     */
#define VREF 3.3f              /* STM32 ADC reference voltage    */
#define VDIV_SCALE 2.0f        /* Voltage divider 2:1 (10k/10k) */
#define ADC_SAMPLES 16         /* Oversampling count             */

/* ── pH sensor (SEN0161) calibration defaults ───────────────────────────────
 */
/* Two-point calibration: pH 7.0 → ~2.50V, pH 4.0 → ~3.03V (at sensor board) */
#define PH_CAL_SLOPE (-5.70f)
#define PH_CAL_OFFSET (21.34f)

/* ── Turbidity sensor (SEN0189) polynomial coefficients ─────────────────────
 */
#define TURB_COEFF_A (-1120.4f)
#define TURB_COEFF_B (5742.3f)
#define TURB_COEFF_C (-4352.9f)

/* ── TDS sensor (SEN0244) polynomial coefficients ───────────────────────────
 */
#define TDS_COEFF_A (133.42f)
#define TDS_COEFF_B (-255.86f)
#define TDS_COEFF_C (857.39f)
#define TDS_FACTOR (0.5f)

/* ── DO sensor (SEN0237-A) calibration ──────────────────────────────────────
 */
#define DO_CAL1_V 1600     /* Calibration voltage (mV) at cal temp */
#define DO_CAL1_T 25       /* Calibration temperature (°C)         */
#define DO_TEMP_COMP_MV 35 /* ~35 mV per °C compensation           */

/* ── Conductivity sensor (DFR0300) ──────────────────────────────────────────
 */
#define EC_K_CAL 1.0f       /* K-constant calibration factor  */
#define EC_TEMP_COEFF 0.02f /* 2% per °C                     */

/* ── ORP sensor (SEN0165) ───────────────────────────────────────────────────
 */
#define ORP_OFFSET 0.0f /* Calibration offset in mV      */

/* ── DS18B20 OneWire ────────────────────────────────────────────────────────
 */
#define OW_PORT GPIOE
#define OW_PIN GPIO_PIN_6
#define DS18B20_CMD_SKIP_ROM 0xCC
#define DS18B20_CMD_MATCH_ROM 0x55
#define DS18B20_CMD_SEARCH 0xF0
#define DS18B20_CMD_CONVERT 0x44
#define DS18B20_CMD_READ_SCRATCH 0xBE

/* ── JSN-SR04T ultrasonic ───────────────────────────────────────────────────
 */
#define TRIG_PORT GPIOE
#define TRIG_PIN GPIO_PIN_4
#define ECHO_PORT GPIOE
#define ECHO_PIN GPIO_PIN_5
#define US_TIMEOUT 30000 /* Echo timeout in µs (~5m max)  */

/* ── BME280 I2C ─────────────────────────────────────────────────────────────
 */
#define BME280_ADDR (0x76 << 1) /* 7-bit addr left-shifted for HAL */
#define BME280_REG_ID 0xD0
#define BME280_REG_CTRL_HUM 0xF2
#define BME280_REG_CTRL_MEAS 0xF4
#define BME280_REG_CONFIG 0xF5
#define BME280_REG_CALIB_00 0x88 /* Temp/pressure calib start      */
#define BME280_REG_CALIB_26 0xE1 /* Humidity calib start            */
#define BME280_REG_DATA 0xF7     /* Data burst read start           */
#define BME280_CHIP_ID 0x60

/* ── JSON buffer ────────────────────────────────────────────────────────────
 */
#define JSON_BUF_SIZE 512

/* ── Sampling interval ──────────────────────────────────────────────────────
 */
#define SAMPLE_INTERVAL_MS 5000 /* 5 seconds between readings     */

/* ── Device ID ──────────────────────────────────────────────────────────────
 */
#define DEVICE_ID "WQM-001"

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

UART_HandleTypeDef huart3;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* USER CODE BEGIN PV */
ADC_HandleTypeDef hadc1;
I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim6;

static BME280_CalibData_t bme280_calib;
static int32_t bme280_t_fine; /* Shared between T/P/H comp  */
static WQM_SensorData_t sensor_data;
static char json_buf[JSON_BUF_SIZE];
static uint32_t sample_counter = 0;

/* DS18B20 ROM addresses — filled at boot by search */
static uint8_t ds18b20_rom[2][8]; /* ROM codes for up to 2 sensors       */
static uint8_t ds18b20_count = 0; /* Number of sensors discovered        */

/* DO saturation lookup table (mg/L × 100) from 0°C to 40°C */
static const uint16_t DO_Table[41] = {
    14460, 14220, 13820, 13440, 13090, 12740, 12420, 12110, 11810, 11530, 11260,
    11010, 10770, 10530, 10300, 10080, 9860,  9660,  9460,  9270,  9080,  8900,
    8730,  8570,  8410,  8250,  8110,  7960,  7820,  7690,  7560,  7430,  7300,
    7180,  7070,  6950,  6840,  6730,  6630,  6530,  6410};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
/* USER CODE BEGIN PFP */

/* ── Peripheral init (manual — not from CubeMX) ──────────────────────────── */
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM6_Init(void);
static void WQM_GPIO_Init(void);

/* ── Microsecond delay ──────────────────────────────────────────────────────
 */
static void delay_us(uint16_t us);

/* ── ADC helpers ────────────────────────────────────────────────────────────
 */
static uint32_t ADC_Read_Channel(uint32_t channel);
static float ADC_Read_Voltage(uint32_t channel);

/* ── DS18B20 OneWire ────────────────────────────────────────────────────────
 */
static void OW_SetOutput(void);
static void OW_SetInput(void);
static void OW_Write(uint8_t state);
static uint8_t OW_Read(void);
static uint8_t OW_Reset(void);
static void OW_WriteByte(uint8_t data);
static uint8_t OW_ReadByte(void);
static void DS18B20_SearchROM(void);
static void DS18B20_StartConvertAll(void);
static float DS18B20_ReadTemp(uint8_t index);

/* ── JSN-SR04T ──────────────────────────────────────────────────────────────
 */
static float JSNSR04T_ReadDistance(void);

/* ── BME280 ─────────────────────────────────────────────────────────────────
 */
static uint8_t BME280_Init(void);
static void BME280_ReadCalibration(void);
static void BME280_ReadAll(float *temp, float *press, float *hum);
static int32_t BME280_CompensateTemp(int32_t adc_T);
static uint32_t BME280_CompensatePress(int32_t adc_P);
static uint32_t BME280_CompensateHum(int32_t adc_H);

/* ── Analog sensor conversions ──────────────────────────────────────────────
 */
static float Sensor_ReadPH(void);
static float Sensor_ReadTurbidity(void);
static float Sensor_ReadTDS(float water_temp);
static float Sensor_ReadDO(float water_temp, float pressure_hpa);
static float Sensor_ReadConductivity(float water_temp);
static float Sensor_ReadORP(void);

/* ── JSON builder ───────────────────────────────────────────────────────────
 */
static uint16_t WQM_BuildJSON(const WQM_SensorData_t *data, char *buf,
                              uint16_t buf_size);

/* ── Debug UART output ──────────────────────────────────────────────────────
 */
static void Debug_Print(const char *str);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* ═══════════════════════════════════════════════════════════════════════════
 */
/*                          MICROSECOND DELAY (TIM6)                         */
/* ═══════════════════════════════════════════════════════════════════════════
 */

static void delay_us(uint16_t us) {
  __HAL_TIM_SET_COUNTER(&htim6, 0);
  while (__HAL_TIM_GET_COUNTER(&htim6) < us)
    ;
}

/* ═══════════════════════════════════════════════════════════════════════════
 */
/*                         ADC SINGLE-CHANNEL READER                         */
/* ═══════════════════════════════════════════════════════════════════════════
 */

/**
 * @brief  Read a single ADC channel with oversampling (16 samples averaged).
 * @param  channel  ADC channel number (e.g. ADC_CHANNEL_3)
 * @return Raw 12-bit ADC value (averaged)
 */
static uint32_t ADC_Read_Channel(uint32_t channel) {
  ADC_ChannelConfTypeDef sConfig = {0};
  sConfig.Channel = channel;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

  uint32_t sum = 0;
  for (int i = 0; i < ADC_SAMPLES; i++) {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    sum += HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
  }
  return sum / ADC_SAMPLES;
}

/**
 * @brief  Read ADC channel and convert to real-world voltage (accounting
 *         for voltage divider).
 * @param  channel  ADC channel number
 * @return Voltage at the sensor output (before divider), in volts
 */
static float ADC_Read_Voltage(uint32_t channel) {
  uint32_t raw = ADC_Read_Channel(channel);
  float v_adc = ((float)raw / ADC_RESOLUTION) * VREF;
  return v_adc * VDIV_SCALE; /* Scale back through voltage divider */
}

/* ═══════════════════════════════════════════════════════════════════════════
 */
/*                        DS18B20 — ONEWIRE DRIVER                           */
/* ═══════════════════════════════════════════════════════════════════════════
 */

static void OW_SetOutput(void) {
  GPIO_InitTypeDef g = {0};
  g.Pin = OW_PIN;
  g.Mode = GPIO_MODE_OUTPUT_PP;
  g.Pull = GPIO_NOPULL;
  g.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(OW_PORT, &g);
}

static void OW_SetInput(void) {
  GPIO_InitTypeDef g = {0};
  g.Pin = OW_PIN;
  g.Mode = GPIO_MODE_INPUT;
  g.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(OW_PORT, &g);
}

static void OW_Write(uint8_t state) {
  HAL_GPIO_WritePin(OW_PORT, OW_PIN, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t OW_Read(void) { return HAL_GPIO_ReadPin(OW_PORT, OW_PIN); }

/**
 * @brief  OneWire reset pulse. Returns 1 if a device is present, 0 otherwise.
 */
static uint8_t OW_Reset(void) {
  uint8_t presence;
  OW_SetOutput();
  OW_Write(0);
  delay_us(480);
  OW_SetInput();
  delay_us(70);
  presence = !OW_Read(); /* LOW = device present */
  delay_us(410);
  return presence;
}

static void OW_WriteBit(uint8_t bit) {
  OW_SetOutput();
  OW_Write(0);
  if (bit) {
    delay_us(6);
    OW_SetInput();
    delay_us(54);
  } else {
    delay_us(60);
    OW_SetInput();
  }
  delay_us(2);
}

static uint8_t OW_ReadBit(void) {
  uint8_t bit;
  OW_SetOutput();
  OW_Write(0);
  delay_us(2);
  OW_SetInput();
  delay_us(12);
  bit = OW_Read();
  delay_us(46);
  return bit;
}

static void OW_WriteByte(uint8_t data) {
  for (int i = 0; i < 8; i++) {
    OW_WriteBit(data & 0x01);
    data >>= 1;
  }
}

static uint8_t OW_ReadByte(void) {
  uint8_t data = 0;
  for (int i = 0; i < 8; i++) {
    data |= (OW_ReadBit() << i);
  }
  return data;
}

/**
 * @brief  Simple ROM search — discovers up to 2 DS18B20 devices on the bus.
 *         For exactly 2 sensors, this uses the SEARCH ROM algorithm.
 *         Fallback: if only 1 device, READ ROM is used.
 */
static void DS18B20_SearchROM(void) {
  ds18b20_count = 0;

  /* Try READ ROM first (works only with a single device) */
  if (!OW_Reset())
    return;
  OW_WriteByte(0x33); /* READ ROM command */
  for (int i = 0; i < 8; i++) {
    ds18b20_rom[0][i] = OW_ReadByte();
  }

  /* Validate family code (0x28 = DS18B20) */
  if (ds18b20_rom[0][0] == 0x28) {
    ds18b20_count = 1;
  } else {
    return; /* No valid device found */
  }

  /* Attempt to find a second device via SEARCH ROM */
  /* Simplified: try SKIP ROM convert then match each ROM to read.
     For 2 sensors sharing a bus, we do a basic iterative search. */
  uint8_t last_discrepancy = 0;
  uint8_t rom_byte, rom_bit, id_bit, cmp_bit;
  uint8_t search_dir;
  uint8_t temp_rom[8];

  if (!OW_Reset())
    return;
  OW_WriteByte(DS18B20_CMD_SEARCH);

  for (int bit_num = 1; bit_num <= 64; bit_num++) {
    id_bit = OW_ReadBit();
    cmp_bit = OW_ReadBit();

    if (id_bit && cmp_bit) {
      /* No devices on bus */
      return;
    }

    if (id_bit != cmp_bit) {
      search_dir = id_bit;
    } else {
      /* Discrepancy — take the opposite path from ROM[0] */
      rom_byte = (bit_num - 1) / 8;
      rom_bit = (bit_num - 1) % 8;
      search_dir = !(ds18b20_rom[0][rom_byte] & (1 << rom_bit)) ? 1 : 0;
      if (search_dir)
        last_discrepancy = bit_num;
    }

    rom_byte = (bit_num - 1) / 8;
    rom_bit = (bit_num - 1) % 8;
    if (search_dir) {
      temp_rom[rom_byte] |= (1 << rom_bit);
    } else {
      temp_rom[rom_byte] &= ~(1 << rom_bit);
    }

    OW_WriteBit(search_dir);
  }

  /* Validate the second ROM */
  if (temp_rom[0] == 0x28 && memcmp(temp_rom, ds18b20_rom[0], 8) != 0) {
    memcpy(ds18b20_rom[1], temp_rom, 8);
    ds18b20_count = 2;
  }
}

/**
 * @brief  Start temperature conversion on ALL DS18B20 sensors (SKIP ROM).
 */
static void DS18B20_StartConvertAll(void) {
  if (!OW_Reset())
    return;
  OW_WriteByte(DS18B20_CMD_SKIP_ROM);
  OW_WriteByte(DS18B20_CMD_CONVERT);
}

/**
 * @brief  Read temperature from a specific DS18B20 by ROM index.
 * @param  index  0 = primary, 1 = backup
 * @return Temperature in °C, or -127.0 on error
 */
static float DS18B20_ReadTemp(uint8_t index) {
  if (index >= ds18b20_count)
    return -127.0f;

  if (!OW_Reset())
    return -127.0f;

  OW_WriteByte(DS18B20_CMD_MATCH_ROM);
  for (int i = 0; i < 8; i++) {
    OW_WriteByte(ds18b20_rom[index][i]);
  }
  OW_WriteByte(DS18B20_CMD_READ_SCRATCH);

  uint8_t lsb = OW_ReadByte();
  uint8_t msb = OW_ReadByte();

  int16_t raw = (int16_t)((msb << 8) | lsb);
  return (float)raw / 16.0f; /* Default 12-bit resolution: 0.0625°C per LSB */
}

/* ═══════════════════════════════════════════════════════════════════════════
 */
/*                      JSN-SR04T — ULTRASONIC DRIVER                        */
/* ═══════════════════════════════════════════════════════════════════════════
 */

/**
 * @brief  Measure distance using JSN-SR04T (trigger/echo mode).
 * @return Distance in cm, or -1.0 on timeout
 */
static float JSNSR04T_ReadDistance(void) {
  uint32_t timeout;

  /* Send 10µs trigger pulse */
  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
  delay_us(15);
  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);

  /* Wait for echo to go HIGH (with timeout) */
  timeout = 0;
  while (HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) == GPIO_PIN_RESET) {
    delay_us(1);
    if (++timeout > US_TIMEOUT)
      return -1.0f;
  }

  /* Measure echo pulse width */
  __HAL_TIM_SET_COUNTER(&htim6, 0);
  timeout = 0;
  while (HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) == GPIO_PIN_SET) {
    delay_us(1);
    if (++timeout > US_TIMEOUT)
      return -1.0f;
  }
  uint32_t echo_us = __HAL_TIM_GET_COUNTER(&htim6);

  /* distance_cm = echo_us / 58 */
  return (float)echo_us / 58.0f;
}

/* ═══════════════════════════════════════════════════════════════════════════
 */
/*                         BME280 — I2C DRIVER                               */
/* ═══════════════════════════════════════════════════════════════════════════
 */

static uint8_t BME280_ReadReg(uint8_t reg) {
  uint8_t val = 0;
  HAL_I2C_Mem_Read(&hi2c1, BME280_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &val, 1,
                   100);
  return val;
}

static void BME280_WriteReg(uint8_t reg, uint8_t val) {
  HAL_I2C_Mem_Write(&hi2c1, BME280_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &val, 1,
                    100);
}

static void BME280_ReadBurst(uint8_t reg, uint8_t *buf, uint16_t len) {
  HAL_I2C_Mem_Read(&hi2c1, BME280_ADDR, reg, I2C_MEMADD_SIZE_8BIT, buf, len,
                   100);
}

static uint8_t BME280_Init(void) {
  /* Verify chip ID */
  uint8_t id = BME280_ReadReg(BME280_REG_ID);
  if (id != BME280_CHIP_ID)
    return 0;

  /* Read factory calibration data */
  BME280_ReadCalibration();

  /* Configure: humidity oversampling ×1 (must be set before ctrl_meas) */
  BME280_WriteReg(BME280_REG_CTRL_HUM, 0x01);

  /* Config: standby 1000ms, filter coeff 4 */
  BME280_WriteReg(BME280_REG_CONFIG, 0xA0);

  /* ctrl_meas: temp ×1, pressure ×1, normal mode */
  BME280_WriteReg(BME280_REG_CTRL_MEAS, 0x27);

  return 1;
}

static void BME280_ReadCalibration(void) {
  uint8_t buf[26];
  BME280_ReadBurst(BME280_REG_CALIB_00, buf, 26);

  bme280_calib.dig_T1 = (uint16_t)(buf[1] << 8 | buf[0]);
  bme280_calib.dig_T2 = (int16_t)(buf[3] << 8 | buf[2]);
  bme280_calib.dig_T3 = (int16_t)(buf[5] << 8 | buf[4]);
  bme280_calib.dig_P1 = (uint16_t)(buf[7] << 8 | buf[6]);
  bme280_calib.dig_P2 = (int16_t)(buf[9] << 8 | buf[8]);
  bme280_calib.dig_P3 = (int16_t)(buf[11] << 8 | buf[10]);
  bme280_calib.dig_P4 = (int16_t)(buf[13] << 8 | buf[12]);
  bme280_calib.dig_P5 = (int16_t)(buf[15] << 8 | buf[14]);
  bme280_calib.dig_P6 = (int16_t)(buf[17] << 8 | buf[16]);
  bme280_calib.dig_P7 = (int16_t)(buf[19] << 8 | buf[18]);
  bme280_calib.dig_P8 = (int16_t)(buf[21] << 8 | buf[20]);
  bme280_calib.dig_P9 = (int16_t)(buf[23] << 8 | buf[22]);
  bme280_calib.dig_H1 = buf[25];

  /* Humidity calib is at a different register block */
  uint8_t hbuf[7];
  BME280_ReadBurst(BME280_REG_CALIB_26, hbuf, 7);

  bme280_calib.dig_H2 = (int16_t)(hbuf[1] << 8 | hbuf[0]);
  bme280_calib.dig_H3 = hbuf[2];
  bme280_calib.dig_H4 = (int16_t)((hbuf[3] << 4) | (hbuf[4] & 0x0F));
  bme280_calib.dig_H5 = (int16_t)((hbuf[5] << 4) | (hbuf[4] >> 4));
  bme280_calib.dig_H6 = (int8_t)hbuf[6];
}

/**
 * @brief  Bosch BME280 temperature compensation (returns °C × 100).
 *         Also sets bme280_t_fine for pressure/humidity compensation.
 */
static int32_t BME280_CompensateTemp(int32_t adc_T) {
  int32_t var1, var2, T;
  var1 = ((((adc_T >> 3) - ((int32_t)bme280_calib.dig_T1 << 1))) *
          ((int32_t)bme280_calib.dig_T2)) >>
         11;
  var2 = (((((adc_T >> 4) - ((int32_t)bme280_calib.dig_T1)) *
            ((adc_T >> 4) - ((int32_t)bme280_calib.dig_T1))) >>
           12) *
          ((int32_t)bme280_calib.dig_T3)) >>
         14;
  bme280_t_fine = var1 + var2;
  T = (bme280_t_fine * 5 + 128) >> 8;
  return T; /* °C × 100 */
}

static uint32_t BME280_CompensatePress(int32_t adc_P) {
  int64_t var1, var2, p;
  var1 = ((int64_t)bme280_t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)bme280_calib.dig_P6;
  var2 = var2 + ((var1 * (int64_t)bme280_calib.dig_P5) << 17);
  var2 = var2 + (((int64_t)bme280_calib.dig_P4) << 35);
  var1 = ((var1 * var1 * (int64_t)bme280_calib.dig_P3) >> 8) +
         ((var1 * (int64_t)bme280_calib.dig_P2) << 12);
  var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)bme280_calib.dig_P1) >> 33;
  if (var1 == 0)
    return 0;
  p = 1048576 - adc_P;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)bme280_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)bme280_calib.dig_P8) * p) >> 19;
  p = ((p + var1 + var2) >> 8) + (((int64_t)bme280_calib.dig_P7) << 4);
  return (uint32_t)p; /* Pa × 256 */
}

static uint32_t BME280_CompensateHum(int32_t adc_H) {
  int32_t v_x1_u32r;
  v_x1_u32r = (bme280_t_fine - ((int32_t)76800));
  v_x1_u32r = (((((adc_H << 14) - (((int32_t)bme280_calib.dig_H4) << 20) -
                  (((int32_t)bme280_calib.dig_H5) * v_x1_u32r)) +
                 ((int32_t)16384)) >>
                15) *
               (((((((v_x1_u32r * ((int32_t)bme280_calib.dig_H6)) >> 10) *
                    (((v_x1_u32r * ((int32_t)bme280_calib.dig_H3)) >> 11) +
                     ((int32_t)32768))) >>
                   10) +
                  ((int32_t)2097152)) *
                     ((int32_t)bme280_calib.dig_H2) +
                 8192) >>
                14));
  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                             ((int32_t)bme280_calib.dig_H1)) >>
                            4));
  v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
  v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
  return (uint32_t)(v_x1_u32r >> 12); /* %RH × 1024 */
}

/**
 * @brief  Read all BME280 data (temp, pressure, humidity) in a single burst.
 */
static void BME280_ReadAll(float *temp, float *press, float *hum) {
  uint8_t buf[8];
  BME280_ReadBurst(BME280_REG_DATA, buf, 8);

  int32_t adc_P = (int32_t)((buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4));
  int32_t adc_T = (int32_t)((buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4));
  int32_t adc_H = (int32_t)((buf[6] << 8) | buf[7]);

  /* Temperature MUST be calculated first (sets t_fine) */
  int32_t t100 = BME280_CompensateTemp(adc_T);
  uint32_t p256 = BME280_CompensatePress(adc_P);
  uint32_t h1024 = BME280_CompensateHum(adc_H);

  *temp = (float)t100 / 100.0f;
  *press = (float)p256 / 256.0f / 100.0f; /* Pa → hPa */
  *hum = (float)h1024 / 1024.0f;
}

/* ═══════════════════════════════════════════════════════════════════════════
 */
/*                    ANALOG SENSOR CONVERSION FUNCTIONS                     */
/* ═══════════════════════════════════════════════════════════════════════════
 */

/**
 * @brief  Read pH value from SEN0161.
 *         Uses two-point calibration: pH = slope × voltage + offset
 */
static float Sensor_ReadPH(void) {
  float voltage = ADC_Read_Voltage(ADC_CHANNEL_3); /* PA3 */
  float ph = PH_CAL_SLOPE * voltage + PH_CAL_OFFSET;
  /* Clamp to valid pH range */
  if (ph < 0.0f)
    ph = 0.0f;
  if (ph > 14.0f)
    ph = 14.0f;
  return ph;
}

/**
 * @brief  Read turbidity from SEN0189.
 *         Quadratic polynomial: NTU = a×V² + b×V + c
 */
static float Sensor_ReadTurbidity(void) {
  float v = ADC_Read_Voltage(ADC_CHANNEL_10); /* PC0 */
  float ntu = TURB_COEFF_A * v * v + TURB_COEFF_B * v + TURB_COEFF_C;
  if (ntu < 0.0f)
    ntu = 0.0f;
  if (ntu > 3000.0f)
    ntu = 3000.0f;
  return ntu;
}

/**
 * @brief  Read TDS from SEN0244 with temperature compensation.
 *         DFRobot polynomial: TDS = (133.42×Vc³ - 255.86×Vc² + 857.39×Vc) × 0.5
 */
static float Sensor_ReadTDS(float water_temp) {
  float voltage = ADC_Read_Voltage(ADC_CHANNEL_13); /* PC3 */

  /* Temperature compensation */
  float comp_coeff = 1.0f + EC_TEMP_COEFF * (water_temp - 25.0f);
  float comp_v = voltage / comp_coeff;

  /* Polynomial conversion */
  float tds = (TDS_COEFF_A * comp_v * comp_v * comp_v +
               TDS_COEFF_B * comp_v * comp_v + TDS_COEFF_C * comp_v) *
              TDS_FACTOR;
  if (tds < 0.0f)
    tds = 0.0f;
  if (tds > 1000.0f)
    tds = 1000.0f; /* SEN0244 max range */
  return tds;
}

/**
 * @brief  Read dissolved oxygen from SEN0237-A with temperature and
 *         barometric pressure compensation.
 *         Uses DO_Table lookup + single-point voltage calibration.
 */
static float Sensor_ReadDO(float water_temp, float pressure_hpa) {
  float voltage = ADC_Read_Voltage(ADC_CHANNEL_4); /* PA4 */
  uint32_t voltage_mv = (uint32_t)(voltage * 1000.0f);

  /* Clamp temperature index to table bounds */
  uint8_t temp_idx = (uint8_t)water_temp;
  if (temp_idx > 40)
    temp_idx = 40;

  /* Calculate saturation voltage at current temperature (single-point cal) */
  uint16_t v_saturation = (uint32_t)DO_CAL1_V +
                          (uint32_t)DO_TEMP_COMP_MV * temp_idx -
                          (uint32_t)DO_CAL1_T * DO_TEMP_COMP_MV;

  /* DO = (measured_mV × saturation_mgL) / saturation_mV */
  float do_val = 0.0f;
  if (v_saturation > 0) {
    do_val = (float)(voltage_mv * DO_Table[temp_idx]) / (float)v_saturation;
    do_val /= 100.0f; /* DO_Table values are mg/L × 100 */
  }

  /* Barometric pressure compensation: DO × (actual_pressure /
   * standard_pressure) */
  if (pressure_hpa > 0.0f) {
    do_val *= (pressure_hpa / 1013.25f);
  }

  if (do_val < 0.0f)
    do_val = 0.0f;
  if (do_val > 20.0f)
    do_val = 20.0f;
  return do_val;
}

/**
 * @brief  Read electrical conductivity from DFR0300 (K=1).
 *         Linear approximation with temperature normalization to 25°C.
 */
static float Sensor_ReadConductivity(float water_temp) {
  float voltage = ADC_Read_Voltage(ADC_CHANNEL_5); /* PA5 */

  /* Raw EC calculation (µS/cm) */
  float ec_raw =
      (voltage * 1000.0f) / (820.0f * 200.0f) * EC_K_CAL * 1000000.0f;

  /* Temperature compensation: normalize to 25°C */
  float ec_25 = ec_raw / (1.0f + EC_TEMP_COEFF * (water_temp - 25.0f));

  if (ec_25 < 0.0f)
    ec_25 = 0.0f;
  return ec_25;
}

/**
 * @brief  Read ORP from SEN0165.
 *         Formula adapted from DFRobot sample code for 12-bit ADC.
 *         ORP_mV = ((30 × Vref × 1000) - (75 × ADC_avg × Vref × 1000 / 4096)) /
 * 75 - OFFSET
 */
static float Sensor_ReadORP(void) {
  uint32_t adc_raw = ADC_Read_Channel(ADC_CHANNEL_6); /* PA6 */

  /* DFRobot ORP formula adapted for 12-bit ADC and 3.3V system with divider */
  float voltage_at_board =
      ((float)adc_raw / ADC_RESOLUTION) * VREF * VDIV_SCALE;
  /* ORP: voltage represents millivolts offset from reference electrode */
  /* The SEN0165 board output: ORP_mV ≈ (Vref/2 - Vout) × gain */
  /* Simplified: map 0–5V board output to ±2000mV ORP range */
  float orp_mv = ((30.0f * VREF * VDIV_SCALE * 1000.0f) -
                  (75.0f * (float)adc_raw * VREF * VDIV_SCALE * 1000.0f /
                   (ADC_RESOLUTION + 1.0f))) /
                     75.0f -
                 ORP_OFFSET;

  return orp_mv;
}

/* ═══════════════════════════════════════════════════════════════════════════
 */
/*                           JSON PACKET BUILDER                             */
/* ═══════════════════════════════════════════════════════════════════════════
 */

/**
 * @brief  Build a JSON payload from the sensor data struct.
 * @param  data     Pointer to sensor data
 * @param  buf      Output buffer
 * @param  buf_size Buffer size
 * @return Number of bytes written (excluding null terminator)
 */
static uint16_t WQM_BuildJSON(const WQM_SensorData_t *data, char *buf,
                              uint16_t buf_size) {
  int len = snprintf(buf, buf_size,
                     "{"
                     "\"id\":\"%s\","
                     "\"ts\":%lu,"
                     "\"pH\":%.2f,"
                     "\"turb_ntu\":%.1f,"
                     "\"tds_ppm\":%.1f,"
                     "\"do_mgl\":%.2f,"
                     "\"ec_uscm\":%.1f,"
                     "\"orp_mv\":%.1f,"
                     "\"temp_w1\":%.2f,"
                     "\"temp_w2\":%.2f,"
                     "\"depth_cm\":%.1f,"
                     "\"temp_air\":%.2f,"
                     "\"humidity\":%.1f,"
                     "\"pressure_hpa\":%.2f"
                     "}\r\n",
                     DEVICE_ID, (unsigned long)HAL_GetTick(), data->ph,
                     data->turbidity_ntu, data->tds_ppm, data->do_mgl,
                     data->ec_uscm, data->orp_mv, data->water_temp_1,
                     data->water_temp_2, data->depth_cm, data->air_temp,
                     data->humidity, data->pressure_hpa);

  return (uint16_t)((len > 0) ? len : 0);
}

/* ═══════════════════════════════════════════════════════════════════════════
 */
/*                           DEBUG UART OUTPUT                               */
/* ═══════════════════════════════════════════════════════════════════════════
 */

static void Debug_Print(const char *str) {
  HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 500);
}

/* ═══════════════════════════════════════════════════════════════════════════
 */
/*                      PERIPHERAL INIT (MANUAL)                             */
/* ═══════════════════════════════════════════════════════════════════════════
 */

/**
 * @brief  ADC1 initialization — 6 analog channels, single conversion mode.
 *         Channels are selected dynamically via ADC_Read_Channel().
 */
static void MX_ADC1_Init(void) {
  __HAL_RCC_ADC1_CLK_ENABLE();

  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  HAL_ADC_Init(&hadc1);
}

/**
 * @brief  I2C1 initialization — 100kHz standard mode for BME280.
 *         SCL = PB8, SDA = PB9.
 */
static void MX_I2C1_Init(void) {
  __HAL_RCC_I2C1_CLK_ENABLE();

  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x20404768; /* 100kHz @ 54MHz APB1 */
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  HAL_I2C_Init(&hi2c1);
}

/**
 * @brief  TIM6 initialization — free-running µs counter.
 *         Clock: APB1 Timer = 108 MHz → PSC=107 → 1 MHz (1 µs tick).
 */
static void MX_TIM6_Init(void) {
  __HAL_RCC_TIM6_CLK_ENABLE();

  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 107; /* 108 MHz / (107+1) = 1 MHz */
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 0xFFFF; /* Free-running 16-bit counter */
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_Base_Init(&htim6);
  HAL_TIM_Base_Start(&htim6);
}

/**
 * @brief  GPIO init for OneWire (PE6), ultrasonic Trig (PE4), Echo (PE5),
 *         and ADC analog pins.
 */
static void WQM_GPIO_Init(void) {
  GPIO_InitTypeDef g = {0};

  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* ── Ultrasonic trigger (PE4) — push-pull output ──────────────────────── */
  g.Pin = TRIG_PIN;
  g.Mode = GPIO_MODE_OUTPUT_PP;
  g.Pull = GPIO_NOPULL;
  g.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TRIG_PORT, &g);
  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);

  /* ── Ultrasonic echo (PE5) — input ────────────────────────────────────── */
  g.Pin = ECHO_PIN;
  g.Mode = GPIO_MODE_INPUT;
  g.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(ECHO_PORT, &g);

  /* ── OneWire (PE6) — starts as input with pull-up ─────────────────────── */
  g.Pin = OW_PIN;
  g.Mode = GPIO_MODE_INPUT;
  g.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(OW_PORT, &g);

  /* ── ADC analog pins — analog mode ────────────────────────────────────── */
  g.Mode = GPIO_MODE_ANALOG;
  g.Pull = GPIO_NOPULL;

  /* PA3 (pH), PA4 (DO), PA5 (Conductivity), PA6 (ORP) */
  g.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
  HAL_GPIO_Init(GPIOA, &g);

  /* PC0 (Turbidity), PC3 (TDS) */
  g.Pin = GPIO_PIN_0 | GPIO_PIN_3;
  HAL_GPIO_Init(GPIOC, &g);

  /* ── I2C1 pins: PB8 (SCL), PB9 (SDA) — alternate function open-drain ── */
  g.Pin = GPIO_PIN_8 | GPIO_PIN_9;
  g.Mode = GPIO_MODE_AF_OD;
  g.Pull = GPIO_PULLUP;
  g.Speed = GPIO_SPEED_FREQ_HIGH;
  g.Alternate = GPIO_AF4_I2C1;
  HAL_GPIO_Init(GPIOB, &g);
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  /* USER CODE BEGIN 2 */

  /* ── Initialize WQM peripherals ───────────────────────────────────────────
   */
  WQM_GPIO_Init();
  MX_TIM6_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();

  /* ── Boot message ─────────────────────────────────────────────────────────
   */
  Debug_Print("\r\n========================================\r\n");
  Debug_Print("  WQM Sensor Suite — NUCLEO-F722ZE\r\n");
  Debug_Print("========================================\r\n");

  /* ── Initialize BME280 ────────────────────────────────────────────────────
   */
  if (BME280_Init()) {
    Debug_Print("[OK]  BME280 initialized (ID=0x60)\r\n");
  } else {
    Debug_Print("[ERR] BME280 not found! Check I2C wiring.\r\n");
  }

  /* ── Discover DS18B20 sensors ─────────────────────────────────────────────
   */
  DS18B20_SearchROM();
  {
    char msg[64];
    snprintf(msg, sizeof(msg), "[OK]  DS18B20 sensors found: %u\r\n",
             ds18b20_count);
    Debug_Print(msg);
  }

  Debug_Print("[OK]  All peripherals initialized.\r\n");
  Debug_Print("========================================\r\n\r\n");

  HAL_Delay(1000); /* Let sensors stabilize */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    sample_counter++;

    /* ═══════════════════════════════════════════════════════════════════════
     */
    /*  STOP-AND-SAMPLE SEQUENCE (per WQM_Sensor_Suite_Guide.md)            */
    /*  1. BME280  2. DS18B20  3. Fast analog  4. ORP  5. pH  6. DO  7. US */
    /* ═══════════════════════════════════════════════════════════════════════
     */

    /* ── Step 1: Read BME280 (air temp, humidity, barometric pressure) ──── */
    BME280_ReadAll(&sensor_data.air_temp, &sensor_data.pressure_hpa,
                   &sensor_data.humidity);

    /* ── Step 2: Start DS18B20 conversion and read temperatures ────────── */
    DS18B20_StartConvertAll();
    HAL_Delay(750); /* 12-bit conversion takes up to 750ms */
    sensor_data.water_temp_1 = DS18B20_ReadTemp(0);
    sensor_data.water_temp_2 = DS18B20_ReadTemp(1);

    /* Use primary temp for compensation; fallback to backup if primary fails */
    float comp_temp = sensor_data.water_temp_1;
    if (comp_temp < -50.0f || comp_temp > 125.0f) {
      comp_temp = sensor_data.water_temp_2;
    }
    if (comp_temp < -50.0f || comp_temp > 125.0f) {
      comp_temp = 25.0f; /* Last resort default */
    }

    /* ── Step 3: Read fast analog sensors (conductivity, TDS, turbidity) ── */
    sensor_data.ec_uscm = Sensor_ReadConductivity(comp_temp);
    sensor_data.tds_ppm = Sensor_ReadTDS(comp_temp);
    sensor_data.turbidity_ntu = Sensor_ReadTurbidity();

    /* ── Step 4: Read ORP (~20s settling in real deployment, instant here)  */
    sensor_data.orp_mv = Sensor_ReadORP();

    /* ── Step 5: Read pH ────────────────────────────────────────────────────
     */
    sensor_data.ph = Sensor_ReadPH();

    /* ── Step 6: Read DO last (slowest — maximum settling time by now) ──── */
    sensor_data.do_mgl = Sensor_ReadDO(comp_temp, sensor_data.pressure_hpa);

    /* ── Step 7: Read ultrasonic depth ───────────────────────────────────────
     */
    sensor_data.depth_cm = JSNSR04T_ReadDistance();

    /* ═══════════════════════════════════════════════════════════════════════
     */
    /*  BUILD JSON PACKET & TRANSMIT VIA USART3 (debug; LoRa later)         */
    /* ═══════════════════════════════════════════════════════════════════════
     */

    uint16_t json_len = WQM_BuildJSON(&sensor_data, json_buf, JSON_BUF_SIZE);
    if (json_len > 0) {
      Debug_Print(json_buf);
    }

    /* Status LED blink — green LD1 on PB0 */
    HAL_GPIO_TogglePin(GPIOB, LD1_Pin);

    /* Wait for next sample interval */
    HAL_Delay(SAMPLE_INTERVAL_MS);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
   */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
   */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief USART3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART3_UART_Init(void) {

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */
}

/**
 * @brief USB_OTG_FS Initialization Function
 * @param None
 * @retval None
 */
static void MX_USB_OTG_FS_PCD_Init(void) {

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.battery_charging_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin | LD3_Pin | LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin,
                    GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin | LD3_Pin | LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* MPU Configuration */

void MPU_Config(void) {
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
   */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
