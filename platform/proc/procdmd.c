
#include <freewpc.h>

#define MAX_DMD_BUFFERS 2

U8 *linux_dmd_low_page;

U8 *linux_dmd_high_page;

U8 *remote_dmd_buffer[MAX_DMD_BUFFERS];


/**
 * Allocate host memory for building the DMD frames.
 */
void remote_dmd_init (void)
{
	int n;

	for (n = 0; n < MAX_DMD_BUFFERS; n++)
		remote_dmd_buffer[n] = malloc (DMD_PAGE_SIZE);

	/* For legacy code */
	linux_dmd_low_page = remote_dmd_buffer[0];
	linux_dmd_high_page = remote_dmd_buffer[1];
}

