#include "MPC5604B.h"
#include "Base.h"

int main(void) {
	volatile int i = 0;

	Disable_Watchdog();
	
	/* Loop forever */
	for (;;) {
		i++;
	}
}

