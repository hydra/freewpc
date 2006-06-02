
#include <freewpc.h>
#include <rtsol.h>


/* Magnet RTT runs every 8 ms */
#define MAG_RTT_FREQ 8

#define MAG_POWER_TIME (400 / MAG_RTT_FREQ)
#define MAG_HOLD_TIME (1100 / MAG_RTT_FREQ)


enum magnet_state {
	MAG_DISABLED,
	MAG_ENABLED,
	MAG_ON_POWER,
	MAG_ON_HOLD,
} left_magnet_state, upper_right_magnet_state, lower_right_magnet_state;

U8 left_magnet_timer, upper_right_magnet_timer, lower_right_magnet_timer;


/** The magnet switch handler is a frequently called function
 * that polls the magnet switches to see if a ball is on
 * top of the magnet, and quickly turns on the magnet when
 * it senses a ball there if it has been enabled to do so. */
static inline void magnet_rtt_switch_handler (
	const U8 sw_magnet,
	enum magnet_state *state,
	U8 *timer )
{
	if ((*state == MAG_ENABLED) &&
		 (rt_switch_poll (sw_magnet)))
	{
		*state = MAG_ON_POWER;
	}
}


/** The magnet duty handler is a less-frequently called
 * function that turns on/off the magnet as necessary.
 * When a ball is being held, it uses duty cycling to avoid
 * burnout. */
static inline void magnet_rtt_duty_handler (
	const U8 sw_magnet,
	const U8 sol_magnet,
	enum magnet_state *state,
	U8 *timer )
{
	switch (*state)
	{
		case MAG_DISABLED:
			/* nothing to do - magnet should already be off */
			break;

		case MAG_ENABLED:
			/* magnet will switch to MAG_ON_POWER if switch closes */

			/* switch to POWER - 100% on */
			rt_sol_enable (sol_magnet);
			*timer = MAG_POWER_TIME;
			*state = MAG_ON_POWER;
			break;

		case MAG_ON_POWER:
			/* keep magnet on with high power */
			/* switch to MAG_ON_HOLD fairly quickly though */
			/* switch should remain closed in this state */
			if (--*timer == 0)
			{
				/* switch to HOLD */
				*timer = MAG_HOLD_TIME;
				*state = MAG_ON_HOLD;
			}
			break;

		case MAG_ON_HOLD:
			/* keep magnet on with low power */
			/* switch should remain closed in this state */
			if (--*timer == 0)
			{
				rt_sol_disable (sol_magnet);
				/* switch to DISABLED */
				*state = MAG_DISABLED;
			}
			else
			{
				if ((*timer % 4) == 0)
				{
					rt_sol_enable (sol_magnet);
				}
				else
				{
					rt_sol_disable (sol_magnet);
				}
			}
			break;
	}
}


void magnet_switch_rtt (void)
{
	magnet_rtt_switch_handler (SW_LEFT_MAGNET,
		&left_magnet_state, &left_magnet_timer);
	
	magnet_rtt_switch_handler (SW_UPPER_RIGHT_MAGNET,
		&upper_right_magnet_state, &upper_right_magnet_timer);
	
	magnet_rtt_switch_handler (SW_LOWER_RIGHT_MAGNET,
		&lower_right_magnet_state, &lower_right_magnet_timer);
}


void magnet_duty_rtt (void)
{
	magnet_rtt_duty_handler (SW_LEFT_MAGNET, SOL_LEFT_MAGNET, 
		&left_magnet_state, &left_magnet_timer);
	
	magnet_rtt_duty_handler (SW_UPPER_RIGHT_MAGNET, SOL_UPPER_RIGHT_MAGNET, 
		&upper_right_magnet_state, &upper_right_magnet_timer);
	
	magnet_rtt_duty_handler (SW_LOWER_RIGHT_MAGNET, SOL_RIGHT_MAGNET, 
		&lower_right_magnet_state, &lower_right_magnet_timer);
}


void magnet_init (void)
{
	left_magnet_state = upper_right_magnet_state = 
		lower_right_magnet_state = MAG_DISABLED;
	left_magnet_timer = upper_right_magnet_timer = 
		lower_right_magnet_timer = 0;
}

