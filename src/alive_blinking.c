#include "alive_blinking.h"
#include "systime.h"
#include "rdy_run.h"


/* Produce a more or less constant green blinking. */
static unsigned long ulAliveBlinking;
static unsigned long ulAliveBlinkingDuration;
static RDYRUN_T tAliveBlinkingState;

void alive_blinking_init(unsigned long ulBlinkDuration)
{
	ulAliveBlinking = systime_get_ms();
	ulAliveBlinkingDuration = ulBlinkDuration;
	tAliveBlinkingState = RDYRUN_GREEN;
	rdy_run_setLEDs(RDYRUN_GREEN);
}



void alive_blinking(void)
{
	int iIsElapsed;


	iIsElapsed = systime_elapsed(ulAliveBlinking, ulAliveBlinkingDuration);
	if( iIsElapsed!=0 )
	{
		if( tAliveBlinkingState==RDYRUN_GREEN )
		{
			/* The SYS LED is currently green. Turn it off now. */
			tAliveBlinkingState = RDYRUN_OFF;
			rdy_run_setLEDs(RDYRUN_OFF);
		}
		else
		{
			/* The SYS LED is currently off. Switch to green now. */
			tAliveBlinkingState = RDYRUN_GREEN;
			rdy_run_setLEDs(RDYRUN_GREEN);
		}

		ulAliveBlinking = systime_get_ms();
	}
}
