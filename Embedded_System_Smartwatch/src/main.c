#include <stdint.h>

int phase0_main(void);
int phase1_main(void);
int phase2_main(void);
int phase3_main(void);

int main(void)
{
#if SMARTWATCH_PHASE == 0
    return phase0_main();
#elif SMARTWATCH_PHASE == 1
    return phase1_main();
#elif SMARTWATCH_PHASE == 2
    return phase2_main();
#elif SMARTWATCH_PHASE == 3
    return phase3_main();
#else
    return -1;
#endif
}
