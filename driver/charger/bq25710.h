/* Copyright 2018 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * TI bq25710 battery charger driver.
 */

#ifndef __CROS_EC_BQ25710_H
#define __CROS_EC_BQ25710_H

/* SMBUS Interface */
#define BQ25710_SMBUS_ADDR1_FLAGS 0x09

#define BQ25710_BC12_MIN_VOLTAGE_MV	1408

/* Registers */
#define BQ25710_REG_CHARGE_OPTION_0		0x12
#define BQ25710_REG_CHARGE_CURRENT		0x14
#define BQ25710_REG_MAX_CHARGE_VOLTAGE		0x15
#define BQ25710_REG_CHARGER_STATUS		0x20
#define BQ25710_REG_PROCHOT_STATUS		0x21
#define BQ25710_REG_IIN_DPM			0x22
#define BQ25710_REG_ADC_VBUS_PSYS		0x23
#define BQ25710_REG_ADC_IBAT			0x24
#define BQ25710_REG_ADC_CMPIN_IIN		0x25
#define BQ25710_REG_ADC_VSYS_VBAT		0x26
#define BQ25710_REG_CHARGE_OPTION_1		0x30
#define BQ25710_REG_CHARGE_OPTION_2		0x31
#define BQ25710_REG_CHARGE_OPTION_3		0x32
#define BQ25710_REG_PROCHOT_OPTION_0		0x33
#define BQ25710_REG_PROCHOT_OPTION_1		0x34
#define BQ25710_REG_ADC_OPTION			0x35
#ifdef CONFIG_CHARGER_BQ25720
#define BQ25720_REG_CHARGE_OPTION_4		0x36
#define BQ25720_REG_VMIN_ACTIVE_PROTECTION	0x37
#endif
#define BQ25710_REG_OTG_VOLTAGE			0x3B
#define BQ25710_REG_OTG_CURRENT			0x3C
#define BQ25710_REG_INPUT_VOLTAGE		0x3D
#define BQ25710_REG_MIN_SYSTEM_VOLTAGE		0x3E
#define BQ25710_REG_IIN_HOST			0x3F
#define BQ25710_REG_MANUFACTURER_ID		0xFE
#define BQ25710_REG_DEVICE_ADDRESS		0xFF

/* ChargeOption0 Register */
#define BQ25710_CHARGE_OPTION_0_LOW_POWER_MODE	BIT(15)
#define BQ25710_CHARGE_OPTION_0_IDPM_AUTO_DIS	BIT(12)
#define BQ25710_CHARGE_OPTION_0_EN_LEARN	BIT(5)
#define BQ25710_CHARGE_OPTION_0_IADP_GAIN	BIT(4)
#define BQ25710_CHARGE_OPTION_0_EN_IDPM		BIT(1)
#define BQ25710_CHARGE_OPTION_0_CHRG_INHIBIT	BIT(0)

/* ChargeOption2 Register */
#define BQ25710_CHARGE_OPTION_2_EN_EXTILIM	BIT(7)
#define BQ25710_CHARGE_OPTION_2_TMAX_SHIFT	8
#define BQ25710_CHARGE_OPTION_2_TMAX_MASK (0x3 << \
					   BQ25710_CHARGE_OPTION_2_TMAX_SHIFT)

/* ChargeOption3 Register */
#define BQ25710_CHARGE_OPTION_3_RESET_REG	BIT(14)
#define BQ25710_CHARGE_OPTION_3_EN_ICO_MODE	BIT(11)

/* ChargeStatus Register */
#define BQ25710_CHARGE_STATUS_ICO_DONE		BIT(14)

/* IIN_DPM Register */
#define BQ25710_IIN_DPM_CURRENT_SHIFT		8
#define BQ25710_IIN_DPM_CURRENT_STEP_MA		50

/* ADCOption Register */
#define BQ25710_ADC_OPTION_ADC_START		BIT(14)
#define BQ25710_ADC_OPTION_EN_ADC_VBUS		BIT(6)
#define BQ25710_ADC_OPTION_EN_ADC_IIN		BIT(4)
#define BQ25710_ADC_OPTION_EN_ADC_ALL		0xFF

/* ADC conversion time ins ms */
#if defined(CONFIG_CHARGER_BQ25720)
#define BQ25710_ADC_OPTION_ADC_CONV_MS		25
#elif defined(CONFIG_CHARGER_BQ25710)
#define BQ25710_ADC_OPTION_ADC_CONV_MS		10
#else
#error Only the BQ25720 and BQ25710 are supported by bq25710 driver.
#endif

/* ADCVBUS/PSYS Register */
#if defined(CONFIG_CHARGER_BQ25720)
#define BQ25720_ADC_VBUS_STEP_MV		96
#elif defined(CONFIG_CHARGER_BQ25710)
#define BQ25710_ADC_VBUS_STEP_MV		64
#define BQ25710_ADC_VBUS_BASE_MV		3200
#else
#error Only the BQ25720 and BQ25710 are supported by bq25710 driver.
#endif
#define BQ25710_ADC_VBUS_STEP_BIT_OFFSET	8

/* ADCIIN Register */
#define BQ25710_ADC_IIN_STEP_MA			50
#define BQ25710_ADC_IIN_STEP_BIT_OFFSET		8

/* ProchotOption0 Register */
#define BQ25710_PROCHOT0_ILIM_VTH_SHIFT 11
#define BQ25710_PROCHOT0_ILIM_VTH_MASK	(0x1f << \
					 BQ25710_PROCHOT0_ILIM_VTH_SHIFT)

/* ProchotOption1 Register */
#define BQ25710_PROCHOT_PROFILE_VDPM		BIT(7)
#define BQ25710_PROCHOT_PROFILE_IDCHG		BIT(3)
#define BQ25710_PROCHOT_PROFILE_VSYS		BIT(2)
#define BQ25710_PROCHOT_IDCHG_VTH_MASK		0xFC00

/* IIN_HOST Register */
#define BQ25710_IIN_HOST_CURRENT_SHIFT		8
#define BQ25710_IIN_HOST_CURRENT_STEP_MA	50

#if defined(CONFIG_CHARGER_BQ25720)
/* Vmin Active Protection Register */
#define BQ25720_VMIN_AP_VSYS_TH2_SHIFT		2
#define BQ25720_VMIN_AP_VSYS_TH2_MASK	GENMASK(7, \
						BQ25720_VMIN_AP_VSYS_TH2_SHIFT)
#endif

/* Min System Voltage Register */
#if defined(CONFIG_CHARGER_BQ25720)
#define BQ25710_MIN_SYSTEM_VOLTAGE_STEP_MV	100
#elif defined(CONFIG_CHARGER_BQ25710)
#define BQ25710_MIN_SYSTEM_VOLTAGE_STEP_MV	256
#else
#error Only the BQ25720 and BQ25710 are supported by bq25710 driver.
#endif
#define BQ25710_MIN_SYSTEM_VOLTAGE_SHIFT	8

extern const struct charger_drv bq25710_drv;

/**
 * Set VSYS_MIN
 *
 * @param chgnum: Index into charger chips
 * @param mv: min system voltage in mV
 * @return EC_SUCCESS or error
 */
int bq25710_set_min_system_voltage(int chgnum, int mv);

#endif /* __CROS_EC_BQ25710_H */
