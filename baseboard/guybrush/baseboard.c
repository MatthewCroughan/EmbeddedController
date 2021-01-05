/* Copyright 2020 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* Guybrush family-specific configuration */

#include "adc.h"
#include "adc_chip.h"
#include "battery_fuel_gauge.h"
#include "chipset.h"
#include "charge_manager.h"
#include "charge_ramp.h"
#include "charge_state.h"
#include "charge_state_v2.h"
#include "charger.h"
#include "driver/ppc/aoz1380.h"
#include "driver/ppc/nx20p348x.h"
#include "driver/tcpm/nct38xx.h"
#include "gpio.h"
#include "i2c.h"
#include "ioexpander.h"
#include "isl9241.h"
#include "nct38xx.h"
#include "pi3usb9201.h"
#include "power.h"
#include "temp_sensor.h"
#include "thermal.h"
#include "usb_mux.h"
#include "usb_pd_tcpm.h"
#include "usbc_ppc.h"

/* Wake Sources */
const enum gpio_signal hibernate_wake_pins[] = {
	GPIO_LID_OPEN,
	GPIO_AC_PRESENT,
	GPIO_POWER_BUTTON_L,
};
const int hibernate_wake_pins_used =  ARRAY_SIZE(hibernate_wake_pins);

/* Power Signal Input List */
const struct power_signal_info power_signal_list[] = {
	[X86_SLP_S0_N] = {
		.gpio = GPIO_PCH_SLP_S0_L,
		.flags = POWER_SIGNAL_ACTIVE_HIGH,
		.name = "SLP_S0_DEASSERTED",
	},
	[X86_SLP_S3_N] = {
		.gpio = GPIO_PCH_SLP_S3_L,
		.flags = POWER_SIGNAL_ACTIVE_HIGH,
		.name = "SLP_S3_DEASSERTED",
	},
	[X86_SLP_S5_N] = {
		.gpio = GPIO_PCH_SLP_S5_L,
		.flags = POWER_SIGNAL_ACTIVE_HIGH,
		.name = "SLP_S5_DEASSERTED",
	},
	[X86_S0_PGOOD] = {
		.gpio = GPIO_S0_PGOOD,
		.flags = POWER_SIGNAL_ACTIVE_HIGH,
		.name = "S0_PGOOD",
	},
	[X86_S5_PGOOD] = {
		.gpio = GPIO_S5_PGOOD,
		.flags = POWER_SIGNAL_ACTIVE_HIGH,
		.name = "S5_PGOOD",
	},
};
BUILD_ASSERT(ARRAY_SIZE(power_signal_list) == POWER_SIGNAL_COUNT);

const struct i2c_port_t i2c_ports[] = {
	{
		.name = "tcpc0",
		.port = I2C_PORT_TCPC0,
		.kbps = 400,
		.scl = GPIO_EC_I2C_USB_A0_C0_SCL,
		.sda = GPIO_EC_I2C_USB_A0_C0_SDA,
	},
	{
		.name = "tcpc1",
		.port = I2C_PORT_TCPC1,
		.kbps = 400,
		.scl = GPIO_EC_I2C_USB_A1_C1_SCL,
		.sda = GPIO_EC_I2C_USB_A1_C1_SDA,
	},
	{
		.name = "battery",
		.port = I2C_PORT_BATTERY,
		.kbps = 100,
		.scl = GPIO_EC_I2C_BATT_SCL,
		.sda = GPIO_EC_I2C_BATT_SDA,
	},
	{
		.name = "usb_mux",
		.port = I2C_PORT_USB_MUX,
		.kbps = 400,
		.scl = GPIO_EC_I2C_USBC_MUX_SCL,
		.sda = GPIO_EC_I2C_USBC_MUX_SDA,
	},
	{
		.name = "charger",
		.port = I2C_PORT_CHARGER,
		.kbps = 400,
		.scl = GPIO_EC_I2C_POWER_SCL,
		.sda = GPIO_EC_I2C_POWER_SDA,
	},
	{
		.name = "eeprom",
		.port = I2C_PORT_EEPROM,
		.kbps = 400,
		.scl = GPIO_EC_I2C_CBI_SCL,
		.sda = GPIO_EC_I2C_CBI_SDA,
	},
	{
		.name = "sensor",
		.port = I2C_PORT_SENSOR,
		.kbps = 400,
		.scl = GPIO_EC_I2C_SENSOR_SCL,
		.sda = GPIO_EC_I2C_SENSOR_SDA,
	},
	{
		.name = "soc_thermal",
		.port = I2C_PORT_THERMAL_AP,
		.kbps = 400,
		.scl = GPIO_EC_I2C_SOC_SIC,
		.sda = GPIO_EC_I2C_SOC_SID,
	},
};
const unsigned int i2c_ports_used = ARRAY_SIZE(i2c_ports);

/* ADC Channels */
const struct adc_t adc_channels[] = {
	[ADC_TEMP_SENSOR_SOC] = {
		.name = "SOC",
		.input_ch = NPCX_ADC_CH0,
		.factor_mul = ADC_MAX_VOLT,
		.factor_div = ADC_READ_MAX + 1,
		.shift = 0,
	},
	[ADC_TEMP_SENSOR_CHARGER] = {
		.name = "CHARGER",
		.input_ch = NPCX_ADC_CH1,
		.factor_mul = ADC_MAX_VOLT,
		.factor_div = ADC_READ_MAX + 1,
		.shift = 0,
	},
	[ADC_TEMP_SENSOR_MEMORY] = {
		.name = "MEMORY",
		.input_ch = NPCX_ADC_CH2,
		.factor_mul = ADC_MAX_VOLT,
		.factor_div = ADC_READ_MAX + 1,
		.shift = 0,
	},

};
BUILD_ASSERT(ARRAY_SIZE(adc_channels) == ADC_CH_COUNT);

/* Temp Sensors */
const struct temp_sensor_t temp_sensors[] = {
	[TEMP_SENSOR_SOC] = {
		.name = "SOC",
		.type = TEMP_SENSOR_TYPE_BOARD,
		.read = baseboard_get_temp,
		.idx = TEMP_SENSOR_SOC,
	},
	[TEMP_SENSOR_CHARGER] = {
		.name = "Charger",
		.type = TEMP_SENSOR_TYPE_BOARD,
		.read = baseboard_get_temp,
		.idx = TEMP_SENSOR_CHARGER,
	},
	[TEMP_SENSOR_MEMORY] = {
		.name = "Memory",
		.type = TEMP_SENSOR_TYPE_BOARD,
		.read = baseboard_get_temp,
		.idx = TEMP_SENSOR_MEMORY,
	},
	[TEMP_SENSOR_CPU] = {
		.name = "CPU",
		.type = TEMP_SENSOR_TYPE_CPU,
		.read = baseboard_get_temp,
		.idx = TEMP_SENSOR_CPU,
	},
};
BUILD_ASSERT(ARRAY_SIZE(temp_sensors) == TEMP_SENSOR_COUNT);

struct ec_thermal_config thermal_params[TEMP_SENSOR_COUNT] = {
	[TEMP_SENSOR_SOC] = {
		.temp_host = {
			[EC_TEMP_THRESH_HIGH] = C_TO_K(90),
			[EC_TEMP_THRESH_HALT] = C_TO_K(92),
		},
		.temp_host_release = {
			[EC_TEMP_THRESH_HIGH] = C_TO_K(80),
		},
		.temp_fan_off = C_TO_K(32),
		.temp_fan_max = C_TO_K(75),
	},
	[TEMP_SENSOR_CHARGER] = {
		.temp_host = {
			[EC_TEMP_THRESH_HIGH] = C_TO_K(90),
			[EC_TEMP_THRESH_HALT] = C_TO_K(92),
		},
		.temp_host_release = {
			[EC_TEMP_THRESH_HIGH] = C_TO_K(80),
		},
		.temp_fan_off = 0,
		.temp_fan_max = 0,
	},
	[TEMP_SENSOR_MEMORY] = {
		.temp_host = {
			[EC_TEMP_THRESH_HIGH] = C_TO_K(90),
			[EC_TEMP_THRESH_HALT] = C_TO_K(92),
		},
		.temp_host_release = {
			[EC_TEMP_THRESH_HIGH] = C_TO_K(80),
		},
		.temp_fan_off = 0,
		.temp_fan_max = 0,
	},
	[TEMP_SENSOR_CPU] = {
		.temp_host = {
			[EC_TEMP_THRESH_HIGH] = C_TO_K(90),
			[EC_TEMP_THRESH_HALT] = C_TO_K(92),
		},
		.temp_host_release = {
			[EC_TEMP_THRESH_HIGH] = C_TO_K(80),
		},
		.temp_fan_off = 0,
		.temp_fan_max = 0,
	},
};
BUILD_ASSERT(ARRAY_SIZE(thermal_params) == TEMP_SENSOR_COUNT);

/*
 * Battery info for all Guybrush battery types. Note that the fields
 * start_charging_min/max and charging_min/max are not used for the charger.
 * The effective temperature limits are given by discharging_min/max_c.
 *
 * Fuel Gauge (FG) parameters which are used for determining if the battery
 * is connected, the appropriate ship mode (battery cutoff) command, and the
 * charge/discharge FETs status.
 *
 * Ship mode (battery cutoff) requires 2 writes to the appropriate smart battery
 * register. For some batteries, the charge/discharge FET bits are set when
 * charging/discharging is active, in other types, these bits set mean that
 * charging/discharging is disabled. Therefore, in addition to the mask for
 * these bits, a disconnect value must be specified. Note that for TI fuel
 * gauge, the charge/discharge FET status is found in Operation Status (0x54),
 * but a read of Manufacturer Access (0x00) will return the lower 16 bits of
 * Operation status which contains the FET status bits.
 *
 * The assumption for battery types supported is that the charge/discharge FET
 * status can be read with a sb_read() command and therefore, only the register
 * address, mask, and disconnect value need to be provided.
 */
const struct board_batt_params board_battery_info[] = {
	/* AP18F4M / LIS4163ACPC */
	[BATTERY_AP18F4M] = {
		.fuel_gauge = {
			.manuf_name = "Murata KT00404001",
			.ship_mode = {
				.reg_addr = 0x3A,
				.reg_data = { 0xC574, 0xC574 },
			},
			.fet = {
				.reg_addr = 0x0,
				.reg_mask = 0x2000,
				.disconnect_val = 0x2000,
			}
		},
		.batt_info = {
			.voltage_max          = 8700,
			.voltage_normal       = 7600,
			.voltage_min          = 5500,
			.precharge_current    = 256,
			.start_charging_min_c = 0,
			.start_charging_max_c = 50,
			.charging_min_c       = 0,
			.charging_max_c       = 60,
			.discharging_min_c    = -20,
			.discharging_max_c    = 75,
		},
	},
};
BUILD_ASSERT(ARRAY_SIZE(board_battery_info) == BATTERY_TYPE_COUNT);
const enum battery_type DEFAULT_BATTERY_TYPE = BATTERY_AP18F4M;

const struct charger_config_t chg_chips[] = {
	{
		.i2c_port = I2C_PORT_CHARGER,
		.i2c_addr_flags = ISL9241_ADDR_FLAGS,
		.drv = &isl9241_drv,
	},
};

const struct tcpc_config_t tcpc_config[] = {
	[USBC_PORT_C0] = {
		.bus_type = EC_BUS_TYPE_I2C,
		.i2c_info = {
			.port = I2C_PORT_TCPC0,
			.addr_flags = NCT38XX_I2C_ADDR1_1_FLAGS,
		},
		.drv = &nct38xx_tcpm_drv,
		.flags = TCPC_FLAGS_TCPCI_REV2_0,
	},
	[USBC_PORT_C1] = {
		.bus_type = EC_BUS_TYPE_I2C,
		.i2c_info = {
			.port = I2C_PORT_TCPC1,
			.addr_flags = NCT38XX_I2C_ADDR1_1_FLAGS,
		},
		.drv = &nct38xx_tcpm_drv,
		.flags = TCPC_FLAGS_TCPCI_REV2_0,
	},
};
BUILD_ASSERT(ARRAY_SIZE(tcpc_config) == USBC_PORT_COUNT);
BUILD_ASSERT(CONFIG_USB_PD_PORT_MAX_COUNT == USBC_PORT_COUNT);

const int usb_port_enable[USBA_PORT_COUNT] = {
	IOEX_EN_PP5000_USB_A0_VBUS,
	IOEX_EN_PP5000_USB_A1_VBUS_DB,
};

struct ppc_config_t ppc_chips[] = {
	[USBC_PORT_C0] = {
		/* Device does not talk I2C */
		.drv = &aoz1380_drv
	},

	[USBC_PORT_C1] = {
		.i2c_port = I2C_PORT_TCPC1,
		.i2c_addr_flags = NX20P3483_ADDR1_FLAGS,
		.drv = &nx20p348x_drv
	},
};
BUILD_ASSERT(ARRAY_SIZE(ppc_chips) == USBC_PORT_COUNT);
unsigned int ppc_cnt = ARRAY_SIZE(ppc_chips);

const struct pi3usb9201_config_t pi3usb9201_bc12_chips[] = {
	[USBC_PORT_C0] = {
		.i2c_port = I2C_PORT_TCPC0,
		.i2c_addr_flags = PI3USB9201_I2C_ADDR_3_FLAGS,
	},

	[USBC_PORT_C1] = {
		.i2c_port = I2C_PORT_TCPC1,
		.i2c_addr_flags = PI3USB9201_I2C_ADDR_3_FLAGS,
	},
};
BUILD_ASSERT(ARRAY_SIZE(pi3usb9201_bc12_chips) == USBC_PORT_COUNT);

struct usb_mux usb_muxes[] = {
	[USBC_PORT_C0] = {
		/* TODO: FIll in FP6 USB Mux configuration */
	},
	[USBC_PORT_C1] = {
		/* TODO: Fill in dynamically */
	}
};
BUILD_ASSERT(ARRAY_SIZE(usb_muxes) == USBC_PORT_COUNT);

struct ioexpander_config_t ioex_config[] = {
	[USBC_PORT_C0] = {
		.i2c_host_port = I2C_PORT_TCPC0,
		.i2c_slave_addr = NCT38XX_I2C_ADDR1_1_FLAGS,
		.drv = &nct38xx_ioexpander_drv,
	},
	[USBC_PORT_C1] = {
		.i2c_host_port = I2C_PORT_TCPC1,
		.i2c_slave_addr = NCT38XX_I2C_ADDR1_1_FLAGS,
		.drv = &nct38xx_ioexpander_drv,
	},
};
BUILD_ASSERT(ARRAY_SIZE(ioex_config) == USBC_PORT_COUNT);
BUILD_ASSERT(CONFIG_IO_EXPANDER_PORT_COUNT == USBC_PORT_COUNT);


int board_set_active_charge_port(int port)
{

	return EC_SUCCESS;
}

int board_is_i2c_port_powered(int port)
{
	switch (port) {
	case I2C_PORT_USB_MUX:
	case I2C_PORT_SENSOR:
		/* USB mux and sensor i2c bus is unpowered in Z1 */
		return chipset_in_state(CHIPSET_STATE_HARD_OFF) ? 0 : 1;
	case I2C_PORT_THERMAL_AP:
		/* SOC thermal i2c bus is unpowered in S0i3/S3/S5/Z1 */
		return chipset_in_state(CHIPSET_STATE_ANY_OFF |
					CHIPSET_STATE_ANY_SUSPEND) ? 0 : 1;
	default:
		return 1;
	}
}

/*
 * In the AOZ1380 PPC, there are no programmable features.  We use
 * the attached NCT3807 to control a GPIO to indicate 1A5 or 3A0
 * current limits.
 */
int board_aoz1380_set_vbus_source_current_limit(int port,
						enum tcpc_rp_value rp)
{
	int rv;

	/* Use the TCPC to set the current limit */
	rv = ioex_set_level(IOEX_USB_C0_PPC_ILIM_3A_EN,
			    (rp == TYPEC_RP_3A0) ? 1 : 0);

	return rv;
}

void board_set_charge_limit(int port, int supplier, int charge_ma,
			    int max_ma, int charge_mv)
{
	charge_set_input_current_limit(MAX(charge_ma,
					   CONFIG_CHARGER_INPUT_CURRENT),
				       charge_mv);
}

void sbu_fault_interrupt(enum ioex_signal signal)
{
	/* TODO */
}

void tcpc_alert_event(enum gpio_signal signal)
{
	/* TODO */
}

void ppc_interrupt(enum gpio_signal signal)
{
	/* TODO */
}

void bc12_interrupt(enum gpio_signal signal)
{
	/* TODO */
}

int baseboard_get_temp(int idx, int *temp_ptr)
{
	/* TODO */
	return 0;
}

int board_is_vbus_too_low(int port, enum chg_ramp_vbus_state ramp_state)
{
	/* TODO */
	return false;
}
