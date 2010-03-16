/*
 * Copyright 2009 by Brian Dominy <brian@oddchange.com>
 *
 * This file is part of FreeWPC.
 *
 * FreeWPC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * FreeWPC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FreeWPC; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <freewpc.h>

#define CONFIG_EMULATE_SERIAL

#define proc_debug(format, rest...) \
	do { \
		extern U16 sys_time; \
		printf ("P-ROC: [%04X] ", sys_time); \
		printf (format, ## rest); \
	} while (0) \

static const uint32_t proc_init_sequence[] = {
	0x801F1122, 0x345678AB
};

struct proc_command
{
	unsigned int len;
	uint32_t data[64];
};

struct proc_response
{
	unsigned int len;
	uint32_t data[64];
};

int proc_write_command (struct proc_command *cmd);
int proc_read_response (struct proc_response *resp);

int serial_port;

unsigned char proc_lamps[16] = { 0, };


/**
 * Open and configure the serial port used for the offload I/O.
 */
int proc_serial_open (const char *name)
{
	struct proc_command initcmd;

#ifdef CONFIG_EMULATE_SERIAL
#else
	serial_port = open (name, O_RDWR | O_NOCTTY);
	if (serial_port < 0)
		return -1;
#endif

	initcmd.len = 2;
	initcmd.data[0] = proc_init_sequence[0];
	initcmd.data[1] = proc_init_sequence[1];
	return proc_write_command (&initcmd);
}

/**
 * Write a command to the serial port.
 */
int proc_write_command (struct proc_command *cmd)
{
#ifdef CONFIG_EMULATE_SERIAL
	int n;
	for (n=0; n < cmd->len; n++)
		proc_debug ("SEND: %08X\n", cmd->data[n]);
	return 0;
#else
	return write (serial_port, cmd->data, cmd->len * sizeof (uint32_t));
#endif
}


/**
 * Read a response from the serial port.
 */
int proc_read_response (struct proc_response *resp)
{
	int n = 0;
	int rc = -1;
	uint32_t *data = resp->data;

#ifdef CONFIG_EMULATE_SERIAL
	n = 0;
#else

	if (resp->len == 0)
		return -1;
	rc = read (serial_port, data, sizeof (uint32_t));

	if (resp->len < PACKET_HEADER_LEN (*data))
		return -1;

	resp->len = PACKET_HEADER_LEN (*data++);
	for (n=0; n < resp->len-1; n++)
		rc = read (serial_port, data++, sizeof (uint32_t));
#endif

	if (rc > 0)
		resp->len = rc;
	return rc;
}

/**
 * Perform a synchronous read request, by writing a command and waiting
 * for the response.
 */
int proc_request (struct proc_command *cmd, struct proc_response *resp)
{
	int rc;
	rc = proc_write_command (cmd);
	rc = proc_read_response (resp);
	return rc;
}


void proc_serial_close (void)
{
#ifdef CONFIG_EMULATE_SERIAL
#else
	close (serial_port);
#endif
}

CALLSET_ENTRY (proc_serial, init)
{
	proc_serial_open ("/dev/ttyUSB1");
}


void proc_write_solenoid (unsigned int solno, const U8 state)
{
	proc_debug ("solno %d is %d\n", solno, state);
}


void proc_write_lamps (unsigned int lamp_strobe_column, unsigned char bits)
{
	if (bits != proc_lamps[lamp_strobe_column])
	{
		proc_lamps[lamp_strobe_column] = bits;
		proc_debug ("lampcol %d is %02X\n", lamp_strobe_column, bits);
	}
}

