#include "notes.h"

#if defined(PAL)
#define L16		7
#else
#define L16		9
#endif

#define L8		(2 * L16)
#define L4		(2 * L8)
#define L2		(2 * L4)
#define L1		(2 * L2)

#define L4t		L2 / 3
#define L8t		L4 / 3

#define Lx8		FADE(1, L8)

#define Ls4		FADE(4, L4)
#define Ls8		FADE(4, L8)

/* slurs */
#define Lr1		FADE(5, L1)
#define Lr2		FADE(5, L2)
#define Lr2p		FADE(5, L2 + L4)
#define Lr4		FADE(5, L4)
#define Lr4p		FADE(5, L4 + L8)
#define Lr8		FADE(5, L8)
#define Lr8p		FADE(5, L8 + L16)
#define Lr16		FADE(5, L16)

static int ocean_height[] = {
    204, 188, 204, 188, 204, 188, 204
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

static unsigned ln_vln1_4[] = {
    D(5, Lr8), E(5, Lr8), F(5, Lr8), G(5, Lr8),
    A(5, Lr8), Gs(5, Lr8), A(5, Lr8p), F(5, Lr16),
    END
};

static unsigned ln_vln1_5[] = {
    E(5, Lr8), C(5, Lr8), C(5, Lr2p), END
};

static unsigned ln_vln1_6[] = {
    B(4, Lr8), B(4, Lr8), B(4, Lr8), B(4, Lr8),
    B(4, Lr8), A(4, Lr8), Gs(4, Lr8), A(4, Lr8),
    END
};

static unsigned ln_vln1_7[] = {
    B(4, Lr1), END
};

static unsigned ln_vln1_8[] = {
    P(L4), C(5, Lr4), C(5, Lr4), B(4, Lr4), END
};

static unsigned ln_vln1_9[] = {
    B(4, Lr4), A(4, Lr8), A(4, Lr4), A(4, Lr8), A(4, Lr8), G(4, Lr8), END
};

static unsigned ln_vln1_A[] = {
    G(4, Lr4), F(4, Lr8), F(4, Lr4), D(4, Lr8), E(4, Lr8), F(4, Lr8), END
};

static unsigned ln_vln1_B[] = {
    G(4, Lr4p), A(4, Lr8), B(4, Lr8), A(4, Lr8), G(4, Lr8), F(4, Lr8), END
};

static unsigned ln_vln1_C[] = {
    E(4, Lr2), E(4, Lr8), E(4, Lr8), Ds(4, Lr8), E(4, Lr8), END
};

static unsigned ln_vln1_D[] = {
    G(4, Lr4), F(4, Lr8), F(4, Lr4), A(4, Lr8), Gs(4, Lr8), A(4, Lr8), END
};

static unsigned ln_vln1_E[] = {
    C(5, Lr4), A(4, Lr8), A(4, Lr4), A(4, Lr8), B(4, Lr8), C(5, Lr8), END
};

static unsigned ln_vln1_F[] = {
    E(5, Lr4p), D(5, Lr8), C(5, Lr8), B(4, Lr8), A(4, Lr8), Gs(4, Lr8), END
};

static unsigned ln_vln1_G[] = {
    A(4, Lr4), C(5, Lr4), C(5, Lr4), B(4, Lr4), END
};

static unsigned ln_vln1_H[] = {
    A(4, Lr2), P(L2), END
};

static unsigned ln_vln1_X[] = {
    E(4, Lx8), A(4, Lx8), Gs(4, Lx8), A(4, Lx8),
    C(5, Lx8), B(4, Lx8), A(4, Lx8), C(5, Lx8),
    END
};

static unsigned ln_vln1_Y[] = {
    E(5, Lx8), D(5, Lx8), C(5, Lx8), B(4, Lx8),
    A(4, Lr2),
    END
};

static void *lunnaja_vln1[] = {
    ln_vln1_X, ln_vln1_Y, ln_vln1_0, ln_vln1_1,
    ln_vln1_2, ln_vln1_3, ln_vln1_4, ln_vln1_5,
    ln_vln1_6, ln_vln1_7, ln_vln1_8, ln_vln1_9,
    ln_vln1_A, ln_vln1_B, ln_vln1_C, ln_vln1_D,
    ln_vln1_E, ln_vln1_F, ln_vln1_G, ln_vln1_9,
    ln_vln1_A, ln_vln1_B, ln_vln1_C, ln_vln1_D,
    ln_vln1_E, ln_vln1_F, ln_vln1_H,
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

static unsigned ln_vln2_4[] = {
    C(4, Lr4), A(3, Lr2), B(3, Lr8), C(4, Lr8), END
};

static unsigned ln_vln2_5[] = {
    Ds(4, Ls4), Fs(4, Ls4), Ds(4, Ls4), B(3, Ls4), END
};

static unsigned ln_vln2_6[] = {
    Gs(3, Lr4), B(3, Lr4), D(4, Lr4), E(4, Lr4), END
};

static unsigned ln_vln2_7[] = {
    Gs(4, Lr2p), Gs(3, Lr4), END
};

static unsigned ln_vln2_8[] = {
    A(3, Ls8), A(3, Ls8), A(3, Ls8), A(3, Ls8),
    C(4, Ls8), C(4, Ls8), Cs(4, Ls8), Cs(4, Ls8),
    END
};

static unsigned ln_vln2_9[] = {
    D(4, Ls8), D(4, Ls8), A(3, Ls8), A(3, Ls8),
    A(3, Ls8), A(3, Ls8), A(3, Ls8), A(3, Ls8),
    END
};

static unsigned ln_vln2_A[] = {
    G(3, Ls8), G(3, Ls8), B(3, Ls8), B(3, Ls8),
    B(3, Ls8), B(3, Ls8), D(4, Ls8), D(4, Ls8),
    END
};

static unsigned ln_vln2_B[] = {
    C(4, Ls8), C(4, Ls8), G(3, Ls8), G(3, Ls8),
    A(3, Ls8), A(3, Ls8), Cs(4, Ls8), Cs(4, Ls8),
    END
};

static unsigned ln_vln2_C[] = {
    D(4, Ls8), D(4, Ls8), A(3, Ls8), A(3, Ls8),
    D(4, Ls8), D(4, Ls8), E(4, Ls8), F(4, Ls8),
    END
};

static unsigned ln_vln2_D[] = {
    E(4, Ls8), E(4, Ls8), C(4, Ls8), C(4, Ls8),
    C(4, Ls8), C(4, Ls8), D(4, Ls8), E(4, Ls8),
    END
};

static unsigned ln_vln2_E[] = {
    F(4, Ls8), F(4, Ls8), B(3, Ls8), B(3, Ls8),
    E(4, Ls8), D(4, Ls8), C(4, Ls8), B(3, Ls8),
    END
};

static unsigned ln_vln2_F[] = {
    A(3, Ls4), E(4, Ls4), E(4, Ls4), Gs(3, Ls4),
    END
};

static unsigned ln_vln2_G[] = {
    A(3, Ls8), A(3, Ls8), B(3, Ls8), B(3, Ls8),
    C(4, Ls8), C(4, Ls8), Cs(4, Ls8), Cs(4, Ls8),
    END
};

static unsigned ln_vln2_H[] = {
    D(4, Ls8), D(4, Ls8), C(4, Ls8), C(4, Ls8),
    B(3, Ls8), B(3, Ls8), A(3, Ls8), A(3, Ls8),
    END
};

static unsigned ln_vln2_I[] = {
    G(3, Ls8), G(3, Ls8), B(3, Ls8), B(3, Ls8),
    D(4, Ls8), D(4, Ls8), B(3, Ls8), B(3, Ls8),
    END
};

static unsigned ln_vln2_J[] = {
    D(4, Lr8), A(3, Lr8), A(3, Lr2), D(3, Lr4), END
};

static unsigned ln_vln2_K[] = {
    C(4, Lr8), E(4, Lr8), E(4, Lr2), C(3, Lr4), END
};

static unsigned ln_vln2_L[] = {
    D(4, Lr8), F(4, Lr4p), E(4, Ls8), P(L8), B(3, Ls8), P(L8), END
};

static unsigned ln_vln2_M[] = {
    C(4, Ls8), P(L8), A(3, Lr4), P(L2), END
};

static unsigned ln_vln2_X[] = {
    A(3, Ls8), P(L4), P(L8), A(3, Ls8), P(L4), P(L8), END
};

static unsigned ln_vln2_Y[] = {
    D(4, Ls8), P(L8), Gs(3, Ls8), P(L8), P(L4), A(3, Lr4), END
};

static void *lunnaja_vln2[] = {
    ln_vln2_X, ln_vln2_Y, ln_vln2_0, ln_vln2_1,
    ln_vln2_2, ln_vln2_3, ln_vln2_2, ln_vln2_4,
    ln_vln2_5, ln_vln2_6, ln_vln2_7, ln_vln2_8,
    ln_vln2_9, ln_vln2_A, ln_vln2_B, ln_vln2_C,
    ln_vln2_D, ln_vln2_E, ln_vln2_F, ln_vln2_G,
    ln_vln2_H, ln_vln2_I, ln_vln2_B, ln_vln2_J,
    ln_vln2_K, ln_vln2_L, ln_vln2_M,
    NULL,
};

static char *lunnaja_fish[] = {
    "________", "________", "2101234x", "210_____", // XY01
    "12345654", "3_______", "2101234x", "234_____", // 2345
    "56543210", "1_______", "246_____", "421345__", // 6789
    "643123__", "412345__", "62345___", "643123__", // ABCD
    "421345__", "632101__", "2424____", "201345__", // EFG9
    "643565__", "410123__", "40123___", "465321__", // ABCD
    "023565__", "412345__", "4_______",             // EFH
    NULL,
};

static unsigned vln2_mod(unsigned note) {
    if (IS_NOTE(note)) {
	note -= (1 << 24); /* lower octave */
	note -= (1 << 8); /* change envelopes */
    }
    return note;
}

void save_lunnaja(void) {
    mod_notes(lunnaja_vln2, &vln2_mod);

    print_level("lunnaja_fish", lunnaja_fish, ocean_height, lunnaja_vln1);
    print_sheet("lunnaja_vln1", lunnaja_vln1);
    print_sheet("lunnaja_vln2", lunnaja_vln2);
}
