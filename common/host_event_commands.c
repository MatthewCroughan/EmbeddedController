/* Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* Host event commands for Chrome EC */

#include "console.h"
#include "host_command.h"
#include "lpc.h"
#include "util.h"

/*****************************************************************************/
/* Console commands */

static int command_host_event(int argc, char **argv)
{
	/* Handle sub-commands */
	if (argc == 3) {
		char *e;
		int i = strtoi(argv[2], &e, 0);
		if (*e) {
			ccputs("Invalid event mask\n");
			return EC_ERROR_INVAL;
		}

		if (!strcasecmp(argv[1], "set")) {
			ccprintf("Setting host event mask 0x%08x\n", i);
			lpc_set_host_events(i);
		} else if (!strcasecmp(argv[1], "clear")) {
			ccprintf("Clearing host event mask 0x%08x\n", i);
			lpc_clear_host_events(i);
		} else if (!strcasecmp(argv[1], "smi")) {
			ccprintf("Setting SMI mask to 0x%08x\n", i);
			lpc_set_host_event_mask(LPC_HOST_EVENT_SMI, i);
		} else if (!strcasecmp(argv[1], "sci")) {
			ccprintf("Setting SCI mask to 0x%08x\n", i);
			lpc_set_host_event_mask(LPC_HOST_EVENT_SCI, i);
		} else if (!strcasecmp(argv[1], "wake")) {
			ccprintf("Setting wake mask to 0x%08x\n", i);
			lpc_set_host_event_mask(LPC_HOST_EVENT_WAKE, i);
		} else {
			ccputs("Unknown sub-command\n");
			return EC_ERROR_INVAL;
		}
	}

	/* Print current SMI/SCI status */
	ccprintf("Raw host events: 0x%08x\n", lpc_get_host_events());
	ccprintf("SMI mask:        0x%08x\n",
		 lpc_get_host_event_mask(LPC_HOST_EVENT_SMI));
	ccprintf("SCI mask:        0x%08x\n",
		 lpc_get_host_event_mask(LPC_HOST_EVENT_SCI));
	ccprintf("Wake mask:       0x%08x\n",
		 lpc_get_host_event_mask(LPC_HOST_EVENT_WAKE));
	return EC_SUCCESS;
}
DECLARE_CONSOLE_COMMAND(hostevent, command_host_event);

/*****************************************************************************/
/* Host commands */

static int host_event_get_smi_mask(uint8_t *data, int *resp_size)
{
	struct lpc_response_host_event_mask *r =
		(struct lpc_response_host_event_mask *)data;

	r->mask = lpc_get_host_event_mask(LPC_HOST_EVENT_SMI);
	*resp_size = sizeof(struct lpc_response_host_event_mask);
	return EC_LPC_RESULT_SUCCESS;
}
DECLARE_HOST_COMMAND(EC_LPC_COMMAND_HOST_EVENT_GET_SMI_MASK,
		     host_event_get_smi_mask);


static int host_event_get_sci_mask(uint8_t *data, int *resp_size)
{
	struct lpc_response_host_event_mask *r =
		(struct lpc_response_host_event_mask *)data;

	r->mask = lpc_get_host_event_mask(LPC_HOST_EVENT_SCI);
	*resp_size = sizeof(struct lpc_response_host_event_mask);
	return EC_LPC_RESULT_SUCCESS;
}
DECLARE_HOST_COMMAND(EC_LPC_COMMAND_HOST_EVENT_GET_SCI_MASK,
		     host_event_get_sci_mask);


static int host_event_get_wake_mask(uint8_t *data, int *resp_size)
{
	struct lpc_response_host_event_mask *r =
		(struct lpc_response_host_event_mask *)data;

	r->mask = lpc_get_host_event_mask(LPC_HOST_EVENT_WAKE);
	*resp_size = sizeof(struct lpc_response_host_event_mask);
	return EC_LPC_RESULT_SUCCESS;
}
DECLARE_HOST_COMMAND(EC_LPC_COMMAND_HOST_EVENT_GET_WAKE_MASK,
		     host_event_get_wake_mask);


static int host_event_set_smi_mask(uint8_t *data, int *resp_size)
{
	const struct lpc_params_host_event_mask *p =
		(const struct lpc_params_host_event_mask *)data;

	lpc_set_host_event_mask(LPC_HOST_EVENT_SMI, p->mask);
	return EC_LPC_RESULT_SUCCESS;
}
DECLARE_HOST_COMMAND(EC_LPC_COMMAND_HOST_EVENT_SET_SMI_MASK,
		     host_event_set_smi_mask);


static int host_event_set_sci_mask(uint8_t *data, int *resp_size)
{
	const struct lpc_params_host_event_mask *p =
		(const struct lpc_params_host_event_mask *)data;

	lpc_set_host_event_mask(LPC_HOST_EVENT_SCI, p->mask);
	return EC_LPC_RESULT_SUCCESS;
}
DECLARE_HOST_COMMAND(EC_LPC_COMMAND_HOST_EVENT_SET_SCI_MASK,
		     host_event_set_sci_mask);


static int host_event_set_wake_mask(uint8_t *data, int *resp_size)
{
	const struct lpc_params_host_event_mask *p =
		(const struct lpc_params_host_event_mask *)data;

	lpc_set_host_event_mask(LPC_HOST_EVENT_WAKE, p->mask);
	return EC_LPC_RESULT_SUCCESS;
}
DECLARE_HOST_COMMAND(EC_LPC_COMMAND_HOST_EVENT_SET_WAKE_MASK,
		     host_event_set_wake_mask);


static int host_event_clear(uint8_t *data, int *resp_size)
{
	const struct lpc_params_host_event_mask *p =
		(const struct lpc_params_host_event_mask *)data;

	lpc_clear_host_events(p->mask);
	return EC_LPC_RESULT_SUCCESS;
}
DECLARE_HOST_COMMAND(EC_LPC_COMMAND_HOST_EVENT_CLEAR, host_event_clear);
