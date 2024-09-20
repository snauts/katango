#include "notes.h"

#define L1		80

static int flame_height[] = {
    208, 208, 208, 208, 208, 208, 208
};

static unsigned if_silent[] = {
    P(L1), END
};

static void *infernal_euph[] = {
    if_silent, if_silent, if_silent, if_silent,
    NULL,
};

static void *infernal_tuba[] = {
    if_silent, if_silent, if_silent, if_silent,
    NULL,
};

static char *infernal_fish[] = {
    "________", "________", "________", "________",
    NULL,
};

void save_infernal(void) {
    print_level("infernal_fish", infernal_fish, flame_height, infernal_euph);
    print_sheet("infernal_euph", infernal_euph);
    print_sheet("infernal_tuba", infernal_tuba);
}
