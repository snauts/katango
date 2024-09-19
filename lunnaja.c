#include "notes.h"

#define L1		96
#define L2		48
#define L4		24
#define L4t		L2 / 3
#define L8		12
#define L8t		L4 / 3
#define L16		6

/* staccato */
#define Ls2p		FADE(1, L2 + L4)
#define Ls4		FADE(1, L4)
#define Ls4t		FADE(1, L4t)
#define Ls4p		FADE(1, L4 + L8)
#define Ls8		FADE(1, L8)
#define Ls8p		FADE(1, L8 + L16)
#define Ls16		FADE(1, L16)

/* slurs */
#define Lr2		FADE(2, L2)
#define Lr2p		FADE(2, L2 + L4)
#define Lr4		FADE(2, L4)
#define Lr4t		FADE(2, L4t)
#define Lr8		FADE(2, L8)
#define Lr8t		FADE(2, L8t)
#define Lr8p		FADE(2, L8 + L16)
#define Lr16		FADE(2, L16)

/* quiet staccato */
#define Ls4q		FADE(3, L4)

/* quiet slur */
#define Lr2q		FADE(4, L2)
#define Lr2qp		FADE(4, L2 + L4)
#define Lr4q		FADE(4, L4)
#define Lr8q		FADE(4, L8)

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
    B(4, Lr8), A(4, Lr8), A(4, Lr2p),
    END
};

static unsigned ln_vln1_2[] = {
    D(5, Lr8), E(5, Lr8), F(5, Lr8), G(5, Lr8),
    A(5, Lr8), Gs(5, Lr8), A(5, Lr8), F(5, Lr8),
    END
};

static void *lunnaja_vln1[] = {
    ln_vln1_0, ln_vln1_1,
    ln_vln1_2, ln_silent, ln_silent, ln_silent,
    NULL,
};

static unsigned ln_vln2_0[] = {
    A(3, Lr2p), C(4, Lr4),
    END
};

static unsigned ln_vln2_1[] = {
    A(3, Lr2), B(3, Lr4), C(4, Lr4),
    END
};

static unsigned ln_vln2_2[] = {
    D(4, Lr2), F(4, Lr4), D(4, Lr4),
    END
};

static void *lunnaja_vln2[] = {
    ln_vln2_0, ln_vln2_1,
    ln_vln2_2, ln_silent, ln_silent, ln_silent,
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
