#include "notes.h"

#define L1		144
#define L2		72
#define L4		36
#define L4t		L2 / 3
#define L8		18
#define L8t		L4 / 3
#define L16		9

/* slurs */
#define Lr1		FADE(5, L2)
#define Lr2		FADE(5, L2)
#define Lr2p		FADE(5, L2 + L4)
#define Lr4		FADE(5, L4)
#define Lr4p		FADE(5, L4 + L8)
#define Lr8		FADE(5, L8)
#define Lr8p		FADE(5, L8 + L16)
#define Lr16		FADE(5, L16)

static int ocean_height[] = {
    188, 204, 188, 204, 188, 204, 188
};

static unsigned ln_silent[] = {
    P(L1), END
};

static unsigned ln_vln1_0[] = {
    A(4, Lr8), B(4, Lr8), C(5, Lr8), D(5, Lr8),
    E(5, Lr8), Ds(5, Lr8), E(5, Lr8p), C(5, Lr16),
    END
};

static unsigned ln_vln1_1[] = {
    B(4, Lr8), A(4, Lr8), A(4, Lr2p), END
};

static unsigned ln_vln1_2[] = {
    D(5, Lr8), E(5, Lr8), F(5, Lr8), G(5, Lr8),
    A(5, Lr8), Gs(5, Lr8), A(5, Lr8), F(5, Lr8),
    END
};

static unsigned ln_vln1_3[] = {
    E(5, Lr1), END
};

static void *lunnaja_vln1[] = {
    ln_vln1_0, ln_vln1_1,
    ln_vln1_2, ln_vln1_3, ln_silent, ln_silent,
    NULL,
};

static unsigned ln_vln2_0[] = {
    A(3, Lr2p), C(4, Lr4), END
};

static unsigned ln_vln2_1[] = {
    A(3, Lr2), B(3, Lr4), C(4, Lr4), END
};

static unsigned ln_vln2_2[] = {
    D(4, Lr2), F(4, Lr4), D(4, Lr4), END
};

static unsigned ln_vln2_3[] = {
    C(4, Lr4p), B(3, Lr8), A(3, Lr4), C(4, Lr4), END
};

static void *lunnaja_vln2[] = {
    ln_vln2_0, ln_vln2_1,
    ln_vln2_2, ln_vln2_3, ln_silent, ln_silent,
    NULL,
};

static char *lunnaja_fish[] = {
    "_______", "_______",
    "_______", "_______", "_______", "_______",
    NULL,
};

void save_lunnaja(void) {
    print_level("lunnaja_fish", lunnaja_fish, ocean_height, lunnaja_vln1);
    print_sheet("lunnaja_vln1", lunnaja_vln1);
    print_sheet("lunnaja_vln2", lunnaja_vln2);
}
