/*
 * Author: Christian Storm
 * Copyright (C) 2016, Siemens AG
 *
 * SPDX-License-Identifier:     GPL-2.0-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <util.h>
#include <bootloader.h>
#include <state.h>
#include <network_ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include "pctl.h"

/*
 * This check is to avoid to corrupt the environment
 * An empty key is accepted, but U-Boot reports a corrupted
 * environment/
 */
#define CHECK_STATE_VAR(v) do { \
	if (strnlen(v, BOOTLOADER_VAR_LENGTH) == 0) { \
		WARN("Update Status Storage Key " \
			"is empty, setting it to 'ustate'"); \
		v = (char *)"ustate"; \
	} \
} while(0)

static server_op_res_t do_save_state(char *key, char* value)
{
	char c;
	CHECK_STATE_VAR(key);
	if (!value)
		return -EINVAL;
	c = *value;
	if (c < STATE_OK || c > STATE_LAST)
		return -EINVAL;
	return bootloader_env_set(key, value) == 0 ? SERVER_OK : SERVER_EERR;
}

server_op_res_t save_state(char *key, update_state_t value)
{
	char value_str[2] = {value, '\0'};
	ipc_message msg;
	if (pid == getpid()) {
		memset(&msg, 0, sizeof(msg));
		msg.magic = IPC_MAGIC;
		msg.type = SET_UPDATE_STATE;
		msg.data.msg[0] = (char)value;
		return (ipc_send_cmd(&msg));
	} else { /* Main process */
		return do_save_state(key, value_str);
	}
}


server_op_res_t save_state_string(char *key, update_state_t value)
{
	CHECK_STATE_VAR(key);
	if (!value)
		return -EINVAL;
	if (value < STATE_OK || value > STATE_LAST)
		return -EINVAL;
	return bootloader_env_set(key, get_state_string(value)) == 0 ?
		SERVER_OK : SERVER_EERR;
}

server_op_res_t read_state(char *key, update_state_t *value)
{
	char *envval;
	CHECK_STATE_VAR(key);

	envval = bootloader_env_get(key);
	if (envval == NULL) {
		INFO("Key '%s' not found in Bootloader's environment.", key);
		*value = STATE_NOT_AVAILABLE;
		return SERVER_OK;
	}
	/* TODO It's a bit whacky just to cast this but as we're the only */
	/*      ones touching the variable, it's maybe OK for a PoC now. */
	*value = (update_state_t)*envval;

	/* bootloader get env allocates space for the value */
	free(envval);

	return SERVER_OK;
}
server_op_res_t unset_state(char *key)
{
	int ret;

	CHECK_STATE_VAR(key);
	ret = bootloader_env_unset(key);
	return ret == 0 ? SERVER_OK : SERVER_EERR;
}

update_state_t get_state(void) {
	update_state_t state;

	if (read_state((char *)STATE_KEY, &state) != SERVER_OK) {
		ERROR("Cannot read stored update state.");
		return STATE_ERROR;
	}
	TRACE("Read state=%c from persistent storage.", state);

	if (is_valid_state(state)) {
		return state;
	}
	ERROR("Unknown update state=%c", state);
	return STATE_ERROR;
}
