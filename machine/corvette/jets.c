
#include <freewpc.h>

__local__ U8 jet_count;

__local__ U8 jet_goal;

__local__ U8 ultra_jet_count;


void jet_deff (void)
{
	dmd_alloc_low_clean ();
	font_render_string_center (&font_mono5, 96, 5, "CORNERING");
	sprintf ("%d", jet_count);
	font_render_string_center (&font_fixed10, 96, 16, sprintf_buffer);

	if (jet_count == jet_goal)
		sprintf ("PIT STOP LIT");
	else
		sprintf ("LITE PIT STOP AT %d", jet_goal);

	font_render_string_center (&font_var5, 64, 26, sprintf_buffer);
	dmd_show_low ();
	task_sleep_sec (2);
	deff_exit ();
}

void jet_flasher (void)
{
	flasher_pulse (FLASH_JETS);
	task_sleep (TIME_500MS);
	task_exit ();
}

void jet_goal_reset (void)
{
	jet_count = 0;
	jet_goal = 10;
}


void jet_goal_award (void)
{
	//sound_start (ST_SAMPLE, MUS_TICKET_BOUGHT, SL_1S, PRI_GAME_QUICK5);

	if (!flag_test(FLAG_PIT_STOP_LIT)) {
		flag_on(FLAG_PIT_STOP_LIT);
	} else {
		score(SC_5M);
	}

	// update the goal hits required to reach the next goal
	jet_count = 0;
	if (jet_goal < 25)
		jet_goal += 5;
}


CALLSET_ENTRY (jets, sw_left_jet, sw_upper_jet, sw_lower_jet)
{
	++jet_count;
	score (SC_5K);
	deff_start (DEFF_JET);
	task_sleep (TIME_16MS);
	task_create_gid1 (GID_JET_FLASHER, jet_flasher);

	if (jet_count == jet_goal)
	{
		jet_goal_award ();
	}
	else
	{
		//sound_start (ST_SAMPLE, SND_JET_BUMPER, SL_500MS, PRI_GAME_QUICK3);
	}
}

CALLSET_ENTRY (jets, start_player)
{
	jet_goal_reset ();
}

CALLSET_ENTRY (jets, start_ball)
{
	ultra_jet_count = 0;
}
