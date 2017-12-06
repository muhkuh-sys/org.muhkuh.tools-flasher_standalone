/* This is the progress bar interface. It is used by the flasher to provide
 * feedback to the host. In this application the output is drastically reduced.
 */

#include "progress_bar.h"
#include "alive_blinking.h"
#include "serial_vectors.h"
#include "systime.h"
#include "uprintf.h"


static unsigned long ulProgressBarTimer;

void progress_bar_init(unsigned long ulMaxvalue __attribute__((unused)))
{
	ulProgressBarTimer = systime_get_ms();
	uprintf("%% ");
	if (tSerialVectors.fn.fnFlush != NULL)
	{
		tSerialVectors.fn.fnFlush();
	}
}



void progress_bar_set_position(unsigned long ulPosition __attribute__((unused)))
{
	progress_bar_check_timer();
}



void progress_bar_check_timer(void)
{
	int iIsElapsed;


	alive_blinking();

	iIsElapsed = systime_elapsed(ulProgressBarTimer, 2000);
	if( iIsElapsed!=0 )
	{
		if (tSerialVectors.fn.fnPut != NULL)
		{
			tSerialVectors.fn.fnPut('.');
		}

		if (tSerialVectors.fn.fnFlush != NULL)
		{
			tSerialVectors.fn.fnFlush();
		}

		ulProgressBarTimer = systime_get_ms();
	}
}



void progress_bar_finalize(void)
{
	uprintf("\n");
}
