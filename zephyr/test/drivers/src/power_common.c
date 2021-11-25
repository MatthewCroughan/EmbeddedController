/* Copyright 2021 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <ztest.h>

#include "chipset.h"
#include "common.h"
#include "host_command.h"
#include "power.h"
#include "stubs.h"
#include "task.h"

#include "emul/emul_common_i2c.h"
#include "emul/emul_smart_battery.h"

#include "battery.h"
#include "battery_smart.h"

#define BATTERY_ORD	DT_DEP_ORD(DT_NODELABEL(battery))

/* Description of all power states with chipset state masks */
static struct {
	/* Power state */
	enum power_state p_state;
	/*
	 * CHIPSET_STATE_* to which this state transition (the same as
	 * transition_from for static states)
	 */
	int transition_to;
	/* CHIPSET_STATE_* from which this state transition */
	int transition_from;
} test_power_state_desc[] = {
	{
		.p_state = POWER_G3,
		.transition_to   = CHIPSET_STATE_HARD_OFF,
		.transition_from = CHIPSET_STATE_HARD_OFF,
	},
	{
		.p_state = POWER_G3S5,
		.transition_to   = CHIPSET_STATE_SOFT_OFF,
		.transition_from = CHIPSET_STATE_HARD_OFF,
	},
	{
		.p_state = POWER_S5G3,
		.transition_to   = CHIPSET_STATE_HARD_OFF,
		.transition_from = CHIPSET_STATE_SOFT_OFF,
	},
	{
		.p_state = POWER_S5,
		.transition_to   = CHIPSET_STATE_SOFT_OFF,
		.transition_from = CHIPSET_STATE_SOFT_OFF,
	},
	{
		.p_state = POWER_S5S3,
		.transition_to   = CHIPSET_STATE_SUSPEND,
		.transition_from = CHIPSET_STATE_SOFT_OFF,
	},
	{
		.p_state = POWER_S3S5,
		.transition_to   = CHIPSET_STATE_SOFT_OFF,
		.transition_from = CHIPSET_STATE_SUSPEND,
	},
	{
		.p_state = POWER_S3,
		.transition_to   = CHIPSET_STATE_SUSPEND,
		.transition_from = CHIPSET_STATE_SUSPEND,
	},
	{
		.p_state = POWER_S3S0,
		.transition_to   = CHIPSET_STATE_ON,
		.transition_from = CHIPSET_STATE_SUSPEND,
	},
	{
		.p_state = POWER_S0S3,
		.transition_to   = CHIPSET_STATE_SUSPEND,
		.transition_from = CHIPSET_STATE_ON,
	},
	{
		.p_state = POWER_S0,
		.transition_to   = CHIPSET_STATE_ON,
		.transition_from = CHIPSET_STATE_ON,
	},
};

/*
 * Chipset state masks used by chipset_in_state and
 * chipset_in_or_transitioning_to_state tests
 */
static int in_state_test_masks[] = {
	CHIPSET_STATE_HARD_OFF,
	CHIPSET_STATE_SOFT_OFF,
	CHIPSET_STATE_SUSPEND,
	CHIPSET_STATE_ON,
	CHIPSET_STATE_STANDBY,
	CHIPSET_STATE_ANY_OFF,
	CHIPSET_STATE_ANY_SUSPEND,
	CHIPSET_STATE_ANY_SUSPEND | CHIPSET_STATE_SOFT_OFF,
};

/** Test chipset_in_state() for each state */
static void test_power_chipset_in_state(void)
{
	bool expected_in_state;
	bool transition_from;
	bool transition_to;
	bool in_state;
	int mask;

	for (int i = 0; i < ARRAY_SIZE(test_power_state_desc); i++) {
		/* Set given power state */
		power_set_state(test_power_state_desc[i].p_state);
		/* Test with selected state masks */
		for (int j = 0; j < ARRAY_SIZE(in_state_test_masks); j++) {
			mask = in_state_test_masks[j];
			/*
			 * Currently tested mask match with state if it match
			 * with transition_to and from chipset states
			 */
			transition_to =
				mask & test_power_state_desc[i].transition_to;
			transition_from =
				mask & test_power_state_desc[i].transition_from;
			expected_in_state = transition_to && transition_from;
			in_state = chipset_in_state(mask);
			zassert_equal(expected_in_state, in_state,
				      "Wrong chipset_in_state() == %d, "
				      "should be %d; mask 0x%x; power state %d "
				      "in test case %d",
				      in_state, expected_in_state, mask,
				      test_power_state_desc[i].p_state, i);
		}
	}
}

/** Test chipset_in_or_transitioning_to_state() for each state */
static void test_power_chipset_in_or_transitioning_to_state(void)
{
	bool expected_in_state;
	bool in_state;
	int mask;

	for (int i = 0; i < ARRAY_SIZE(test_power_state_desc); i++) {
		/* Set given power state */
		power_set_state(test_power_state_desc[i].p_state);
		/* Test with selected state masks */
		for (int j = 0; j < ARRAY_SIZE(in_state_test_masks); j++) {
			mask = in_state_test_masks[j];
			/*
			 * Currently tested mask match with state if it match
			 * with transition_to chipset state
			 */
			expected_in_state =
				mask & test_power_state_desc[i].transition_to;
			in_state = chipset_in_or_transitioning_to_state(mask);
			zassert_equal(expected_in_state, in_state,
				      "Wrong "
				      "chipset_in_or_transitioning_to_state() "
				      "== %d, should be %d; mask 0x%x; "
				      "power state %d in test case %d",
				      in_state, expected_in_state, mask,
				      test_power_state_desc[i].p_state, i);
		}
	}
}

/** Test using chipset_exit_hard_off() in different power states */
static void test_power_exit_hard_off(void)
{
	/* Force initial state */
	force_power_state(true, POWER_G3);
	zassert_equal(POWER_G3, power_get_state(), NULL);

	/* Stop forcing state */
	force_power_state(false, 0);

	/* Test after exit hard off, we reach G3S5 */
	chipset_exit_hard_off();
	/*
	 * TODO(b/201420132) - chipset_exit_hard_off() is waking up
	 * TASK_ID_CHIPSET Sleep is required to run chipset task before
	 * continuing with test
	 */
	k_msleep(1);
	zassert_equal(POWER_G3S5, power_get_state(), NULL);

	/* Go back to G3 and check we stay there */
	force_power_state(true, POWER_G3);
	force_power_state(false, 0);
	zassert_equal(POWER_G3, power_get_state(), NULL);

	/* Exit G3 again */
	chipset_exit_hard_off();
	/* TODO(b/201420132) - see comment above */
	k_msleep(1);
	zassert_equal(POWER_G3S5, power_get_state(), NULL);

	/* Go to S5G3 */
	force_power_state(true, POWER_S5G3);
	zassert_equal(POWER_S5G3, power_get_state(), NULL);

	/* Test exit hard off in S5G3 -- should immedietly exit G3 */
	chipset_exit_hard_off();
	/* Go back to G3 and check we exit it to G3S5 */
	force_power_state(true, POWER_G3);
	zassert_equal(POWER_G3S5, power_get_state(), NULL);

	/* Test exit hard off is cleared on entering S5 */
	chipset_exit_hard_off();
	force_power_state(true, POWER_S5);
	zassert_equal(POWER_S5, power_get_state(), NULL);
	/* Go back to G3 and check we stay in G3 */
	force_power_state(true, POWER_G3);
	force_power_state(false, 0);
	zassert_equal(POWER_G3, power_get_state(), NULL);

	/* Test exit hard off doesn't work on other states */
	force_power_state(true, POWER_S5S3);
	force_power_state(false, 0);
	zassert_equal(POWER_S5S3, power_get_state(), NULL);
	chipset_exit_hard_off();
	/* TODO(b/201420132) - see comment above */
	k_msleep(1);

	/* Go back to G3 and check we stay in G3 */
	force_power_state(true, POWER_G3);
	force_power_state(false, 0);
	zassert_equal(POWER_G3, power_get_state(), NULL);
}

/* Test reboot ap on g3 host command is triggering reboot */
static void test_power_reboot_ap_at_g3(void)
{
	struct ec_params_reboot_ap_on_g3_v1 params;
	struct host_cmd_handler_args args = {
		.command = EC_CMD_REBOOT_AP_ON_G3,
		.version = 0,
		.send_response = stub_send_response_callback,
		.params = &params,
		.params_size = sizeof(params),
	};
	int offset_for_still_in_g3_test;
	int delay_ms;

	/* Force initial state S0 */
	force_power_state(true, POWER_S0);
	zassert_equal(POWER_S0, power_get_state(), NULL);

	/* Test version 0 (no delay argument) */
	zassert_equal(EC_RES_SUCCESS, host_command_process(&args), NULL);

	/* Go to G3 and check if reboot is triggered */
	force_power_state(true, POWER_G3);
	zassert_equal(POWER_G3S5, power_get_state(), NULL);

	/* Test version 1 (with delay argument) */
	args.version = 1;
	delay_ms = 3000;
	params.reboot_ap_at_g3_delay = delay_ms / 1000; /* in seconds */
	zassert_equal(EC_RES_SUCCESS, host_command_process(&args), NULL);

	/* Go to G3 and check if reboot is triggered after delay */
	force_power_state(true, POWER_G3);
	force_power_state(false, 0);
	zassert_equal(POWER_G3, power_get_state(), NULL);
	/*
	 * Arbitrary chosen offset before end of reboot delay to check if G3
	 * state wasn't left too soon
	 */
	offset_for_still_in_g3_test = 50;
	k_msleep(delay_ms - offset_for_still_in_g3_test);
	/* Test if still in G3 */
	zassert_equal(POWER_G3, power_get_state(), NULL);
	/*
	 * power_common_state() use for loop with 100ms sleeps. msleep() wait at
	 * least specified time, so wait 10% longer than specified delay to take
	 * this into account.
	 */
	k_msleep(offset_for_still_in_g3_test + delay_ms / 10);
	/* Test if reboot is triggered */
	zassert_equal(POWER_G3S5, power_get_state(), NULL);
}

/** Test setting cutoff and stay-up battery levels through host command */
static void test_power_hc_smart_discharge(void)
{
	struct ec_response_smart_discharge response;
	struct ec_params_smart_discharge params;
	struct host_cmd_handler_args args =
		BUILD_HOST_COMMAND(EC_CMD_SMART_DISCHARGE, 0, response);
	struct i2c_emul *emul;
	int hours_to_zero;
	int hibern_drate;
	int cutoff_drate;
	int stayup_cap;
	int cutoff_cap;

	emul = sbat_emul_get_ptr(BATTERY_ORD);

	/* Set up host command parameters */
	args.params = &params;
	args.params_size = sizeof(params);

	params.flags = EC_SMART_DISCHARGE_FLAGS_SET;

	/* Test fail when battery capacity is not available */
	i2c_common_emul_set_read_fail_reg(emul, SB_FULL_CHARGE_CAPACITY);
	zassert_equal(EC_RES_UNAVAILABLE, host_command_process(&args), NULL);
	i2c_common_emul_set_read_fail_reg(emul, I2C_COMMON_EMUL_NO_FAIL_REG);

	/* Setup discharge rates */
	params.drate.hibern = 10;
	params.drate.cutoff = 100;
	/* Test fail on higher discahrge in hibernation than cutoff */
	zassert_equal(EC_RES_INVALID_PARAM, host_command_process(&args), NULL);

	/* Setup discharge rates */
	params.drate.hibern = 10;
	params.drate.cutoff = 0;
	/* Test fail on only one discharge rate set to 0 */
	zassert_equal(EC_RES_INVALID_PARAM, host_command_process(&args), NULL);

	/* Setup correct parameters */
	hours_to_zero = 1000;
	hibern_drate = 100; /* uA */
	cutoff_drate = 10; /* uA */
	/* Need at least 100 mA capacity to stay 1000h using 0.1mAh */
	stayup_cap = hibern_drate * hours_to_zero / 1000;
	/* Need at least 10 mA capacity to stay 1000h using 0.01mAh */
	cutoff_cap = cutoff_drate * hours_to_zero / 1000;

	params.drate.hibern = hibern_drate;
	params.drate.cutoff = cutoff_drate;
	params.hours_to_zero = hours_to_zero;

	/* Test if correct values are set */
	zassert_equal(EC_RES_SUCCESS, host_command_process(&args), NULL);
	zassert_equal(hibern_drate, response.drate.hibern, NULL);
	zassert_equal(cutoff_drate, response.drate.cutoff, NULL);
	zassert_equal(hours_to_zero, response.hours_to_zero, NULL);
	zassert_equal(stayup_cap, response.dzone.stayup, NULL);
	zassert_equal(cutoff_cap, response.dzone.cutoff, NULL);

	/* Setup discharge rate to 0 */
	params.drate.hibern = 0;
	params.drate.cutoff = 0;
	/* Update hours to zero */
	hours_to_zero = 2000;
	params.hours_to_zero = hours_to_zero;
	/* Need at least 200 mA capacity to stay 2000h using 0.1mAh */
	stayup_cap = hibern_drate * hours_to_zero / 1000;
	/* Need at least 20 mA capacity to stay 2000h using 0.01mAh */
	cutoff_cap = cutoff_drate * hours_to_zero / 1000;

	/* Test that command doesn't change drate but apply new hours to zero */
	zassert_equal(EC_RES_SUCCESS, host_command_process(&args), NULL);
	zassert_equal(hibern_drate, response.drate.hibern, NULL);
	zassert_equal(cutoff_drate, response.drate.cutoff, NULL);
	zassert_equal(hours_to_zero, response.hours_to_zero, NULL);
	zassert_equal(stayup_cap, response.dzone.stayup, NULL);
	zassert_equal(cutoff_cap, response.dzone.cutoff, NULL);

	/* Setup any parameters != 0 */
	params.drate.hibern = 1000;
	params.drate.cutoff = 1000;
	/* Clear set flag */
	params.flags = 0;

	/* Test that command doesn't change drate and dzone */
	zassert_equal(EC_RES_SUCCESS, host_command_process(&args), NULL);
	zassert_equal(hibern_drate, response.drate.hibern, NULL);
	zassert_equal(cutoff_drate, response.drate.cutoff, NULL);
	zassert_equal(hours_to_zero, response.hours_to_zero, NULL);
	zassert_equal(stayup_cap, response.dzone.stayup, NULL);
	zassert_equal(cutoff_cap, response.dzone.cutoff, NULL);
}

/**
 * Test if default board_system_is_idle() recognize cutoff and stay-up
 * levels correctly.
 */
static void test_power_board_system_is_idle(void)
{
	struct ec_response_smart_discharge response;
	struct ec_params_smart_discharge params;
	struct host_cmd_handler_args args =
		BUILD_HOST_COMMAND(EC_CMD_SMART_DISCHARGE, 0, response);
	struct sbat_emul_bat_data *bat;
	struct i2c_emul *emul;
	uint64_t last_shutdown_time = 0;
	uint64_t target;
	uint64_t now;

	emul = sbat_emul_get_ptr(BATTERY_ORD);
	bat = sbat_emul_get_bat_data(emul);

	/* Set up host command parameters */
	args.params = &params;
	args.params_size = sizeof(params);
	params.drate.hibern = 100; /* uA */
	params.drate.cutoff = 10; /* uA */
	params.hours_to_zero = 1000; /* h */
	params.flags = EC_SMART_DISCHARGE_FLAGS_SET;
	/* Set stay-up and cutoff zones */
	zassert_equal(EC_RES_SUCCESS, host_command_process(&args), NULL);

	/* Test shutdown ignore is send when target time is in future */
	target = 1125;
	now = 1000;
	zassert_equal(CRITICAL_SHUTDOWN_IGNORE,
		      board_system_is_idle(last_shutdown_time, &target, now),
		      NULL);

	/* Set "now" time after target time */
	now = target + 30;

	/*
	 * Test hibernation is requested when battery remaining capacity
	 * is not available
	 */
	i2c_common_emul_set_read_fail_reg(emul, SB_REMAINING_CAPACITY);
	zassert_equal(CRITICAL_SHUTDOWN_HIBERNATE,
		      board_system_is_idle(last_shutdown_time, &target, now),
		      NULL);
	i2c_common_emul_set_read_fail_reg(emul, I2C_COMMON_EMUL_NO_FAIL_REG);

	/* Setup remaining capacity to trigger cutoff */
	bat->cap = response.dzone.cutoff - 5;
	zassert_equal(CRITICAL_SHUTDOWN_CUTOFF,
		      board_system_is_idle(last_shutdown_time, &target, now),
		      NULL);

	/* Setup remaining capacity to trigger stay-up and ignore shutdown */
	bat->cap = response.dzone.stayup - 5;
	zassert_equal(CRITICAL_SHUTDOWN_IGNORE,
		      board_system_is_idle(last_shutdown_time, &target, now),
		      NULL);

	/* Setup remaining capacity to be in safe zone to hibernate */
	bat->cap = response.dzone.stayup + 5;
	zassert_equal(CRITICAL_SHUTDOWN_HIBERNATE,
		      board_system_is_idle(last_shutdown_time, &target, now),
		      NULL);
}

void test_suite_power_common(void)
{
	ztest_test_suite(power_common,
			 ztest_unit_test(test_power_chipset_in_state),
			 ztest_unit_test(
			       test_power_chipset_in_or_transitioning_to_state),
			 ztest_unit_test(test_power_exit_hard_off),
			 ztest_unit_test(test_power_reboot_ap_at_g3),
			 ztest_unit_test(test_power_hc_smart_discharge),
			 ztest_unit_test(test_power_board_system_is_idle));
	ztest_run_test_suite(power_common);
}
