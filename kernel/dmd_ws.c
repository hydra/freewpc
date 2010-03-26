
#include <freewpc.h>
#include <queue.h>

#define PLASMA_QUEUE_LEN 32

struct {
	queue_t header;
	U8 elems[PLASMA_QUEUE_LEN];
} plasma_write_queue;

ws_dmd_msg_t plasma_msg;


static __attribute__((noinline)) void plasma_queue_insert (U8 val)
{
	queue_insert (&plasma_write_queue.header, PLASMA_QUEUE_LEN, val);
}

static U8 plasma_write_queue_remove (void)
{
	return queue_remove (&plasma_write_queue.header, PLASMA_QUEUE_LEN);
}

static inline bool plasma_write_queue_empty_p (void)
{
	return queue_empty_p (&plasma_write_queue.header);
}


/*
 * Send the next output byte to the plasma display.
 */
void dmd_ws_rtt (void)
{
	if (unlikely (!plasma_write_queue_empty_p ()))
	{
		U8 val = plasma_write_queue_remove ();
		writeb (WS_PLASMA_DATA, val);
		noop ();
		(void) readb (WS_PLASMA_STROBE);
	}
}


/*
 * Send a formatted message to the plasma display.
 *
 * This function queues all of the bytes of the message for transmission by
 * the realtime handler.
 */
void plasma_send (void)
{
	disable_interrupts ();
	plasma_queue_insert (&plasma_msg.len);
	plasma_queue_insert (&plasma_msg.options);
	plasma_queue_insert (&plasma_msg.cmd);
	enable_interrupts ();
}


CALLSET_ENTRY (dmd_ws, amode_start)
{
	plasma_msg.len = 2;
	plasma_msg.options = 0x80;
	plasma_msg.cmd = 0; /* ?? */
}


CALLSET_ENTRY (dmd_ws, init)
{
	writeb (WS_PLASMA_RESET, 0);
}

