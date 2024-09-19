#include "notes.h"

#define L1		96
#define L2		48
#define L4		24
#define L4t		L2 / 3
#define L8		12
#define L8t		L4 / 3

/* staccato */
#define Ls4		FADE(1, L4)
#define Ls4t		FADE(1, L4t)
#define Ls4p		FADE(1, L4 + L8)
#define Ls8		FADE(1, L8)

/* slurs */
#define Lr2		FADE(2, L2)
#define Lr4		FADE(2, L4)
#define Lr8		FADE(2, L8)
#define Lr8t		FADE(2, L8t)

/* quiet staccato */
#define Ls4q		FADE(3, L4)

/* quiet slur */
#define Lr4q		FADE(4, L4)
#define Lr8q		FADE(4, L8)

static int alley_height[] = {
    208, 208, 208, 208, 208, 208, 208
};

static unsigned hb_silent[] = {
    P(L1), END
};

static unsigned hb_bass_0[] = {
    D(2, Ls4q), P(L8), A(2, Lr8q), F(3, Ls4q), A(2, Ls4q), END
};

static unsigned hb_bass_1[] = {
    D(2, Ls4q), P(L8), As(2, Lr8q), G(3, Ls4q), A(2, Ls4q), END
};

static unsigned hb_bass_2[] = {
    D(2, Ls4q), P(L8), A(2, Lr8q), Fs(3, Ls4q), A(2, Ls4q), END
};

static unsigned hb_bass_3[] = {
    D(2, Ls4q), P(L8), A(2, Lr8q), G(3, Ls4q), A(2, Ls4q), END
};

static unsigned hb_bass_4[] = {
    D(2, Ls4q), P(L4), A(3, Ls4q), G(3, Lr4q), END
};

static unsigned hb_bass_5[] = {
    Fs(3, Ls4q), P(L8), A(2, Lr8q), D(2, Ls4q), P(L4), END
};

static void *habanera_bass[] = {
    hb_bass_0, hb_bass_0, hb_bass_0, hb_bass_0, hb_bass_0,
    hb_bass_0, hb_bass_0, hb_bass_1, hb_bass_1, hb_bass_1,
    hb_bass_1, hb_bass_0, hb_bass_0, hb_bass_1, hb_bass_1,
    hb_bass_1, hb_bass_1, hb_bass_1, hb_bass_1, hb_bass_2,
    hb_bass_2, hb_bass_2, hb_bass_2, hb_bass_3, hb_bass_3,
    hb_bass_3, hb_bass_3, hb_bass_2, hb_bass_2, hb_bass_2, hb_bass_2,
    hb_bass_3, hb_bass_3, hb_bass_3, hb_bass_3, hb_bass_2, hb_bass_2,
    hb_bass_2, hb_bass_2, hb_bass_3, hb_bass_3, hb_bass_3,
    hb_bass_4, hb_bass_5,
    NULL,
};

static unsigned hb_high_1[] = {
    P(L2), D(5, Ls4), Cs(5, Ls4), END
};

static unsigned hb_high_2[] = {
    Cs(5, Ls4t), Cs(5, Ls4t), Cs(5, Ls4t), B(4, Ls4), As(4, Ls4), END
};

static unsigned hb_high_3[] = {
    A(4, Ls4p), A(4, Ls8), Gs(4, Ls4), G(4, Ls4), END
};

static unsigned hb_high_4[] = {
    F(4, Lr8t), G(4, Lr8t), F(4, Lr8t), E(4, Lr8), F(4, Lr8),
    G(4, Ls4), F(4, Ls4), END
};

static unsigned hb_high_5[] = {
    E(4, Ls4), P(L4), D(5, Ls4), Cs(5, Ls4), END
};

static unsigned hb_high_6[] = {
    A(4, Ls4p), A(4, Ls8), G(4, Ls4), F(4, Ls4), END
};

static unsigned hb_high_7[] = {
    E(4, Lr8t), F(4, Lr8t), E(4, Lr8t), D(4, Lr8), E(4, Lr8),
    F(4, Ls4), E(4, Ls4), END
};

static unsigned hb_high_8[] = {
    D(4, Lr2), D(5, Ls4), Cs(5, Ls4), END
};

static unsigned hb_high_9[] = {
    D(4, Ls4), P(L4), D(5, Ls4), Cs(5, Ls4), END
};

static unsigned hb_high_A[] = {
    C(5, Ls4t), C(5, Ls4t), C(5, Ls4t), B(4, Ls4), As(4, Ls4), END
};

static unsigned hb_high_B[] = {
    A(4, Ls4p), A(4, Lr8), Gs(4, Ls4), G(4, Ls4), END
};

static unsigned hb_high_C[] = {
    Fs(4, Lr8t), G(4, Lr8t), Fs(4, Lr8t), E(4, Lr8), Fs(4, Lr8),
    G(4, Ls4), Fs(4, Ls4), END
};

static unsigned hb_high_D[] = {
    E(4, Ls4), P(L4), D(5, Ls4), Cs(5, Ls4), END
};

static unsigned hb_high_E[] = {
    A(4, Ls4p), A(4, Lr8), G(4, Ls4), Fs(4, Ls4), END
};

static unsigned hb_high_F[] = {
    E(4, Lr8t), Fs(4, Lr8t), E(4, Lr8t), D(4, Lr8), E(4, Lr8),
    Fs(4, Ls4), E(4, Ls4), END
};

static unsigned hb_high_G[] = {
    D(4, Ls4), P(L8), A(4, Lr8), D(4, Ls4), E(4, Ls4), END
};

static unsigned hb_high_H[] = {
    Fs(4, Ls4p), A(4, Lr8), Fs(4, Ls4), E(4, Ls4), END
};

static unsigned hb_high_I[] = {
    D(4, Ls4p), E(4, Lr8), Fs(4, Ls4), G(4, Ls4), END
};

static unsigned hb_high_J[] = {
    A(4, Ls8), A(4, Ls8), A(4, Ls8), A(4, Ls8), B(4, Ls4), A(4, Ls4), END
};

static unsigned hb_high_K[] = {
    G(4, Ls4), P(L8), B(4, Lr8), E(4, Ls4), Fs(4, Ls4), END
};

static unsigned hb_high_L[] = {
    G(4, Ls4p), B(4, Lr8), G(4, Ls4), Fs(4, Ls4), END
};

static unsigned hb_high_M[] = {
    E(4, Ls4p), Fs(4, Lr8), G(4, Ls4), A(4, Ls4), END
};

static unsigned hb_high_N[] = {
    B(4, Ls8), B(4, Ls8), B(4, Ls8), B(4, Ls8), Cs(5, Ls4), B(4, Ls4), END
};

static unsigned hb_high_O[] = {
    A(4, Ls4), P(L8), A(4, Lr8), D(5, Ls4), E(5, Ls4), END
};

static unsigned hb_high_P[] = {
    Fs(5, Ls4), P(L8), A(4, Lr8), Fs(4, Ls4), E(4, Ls4), END
};

static unsigned hb_high_Q[] = {
    A(4, Ls8), A(4, Ls8), A(4, Ls8), A(4, Ls8), D(5, Ls4), Cs(5, Ls4), END
};

static unsigned hb_high_R[] = {
    A(4, Ls4), P(L8), B(4, Lr8), E(5, Ls4), Fs(5, Ls4), END
};

static unsigned hb_high_S[] = {
    G(5, Ls4), P(L8), B(4, Lr8), G(4, Ls4), Fs(4, Ls4), END
};

static unsigned hb_high_T[] = {
    Cs(5, Lr8), B(4, Lr8), Gs(4, Lr8), A(4, Lr8), Fs(5, Lr4),
    E(5, Lr8t), Fs(5, Lr8t), E(5, Lr8t), END
};

static unsigned hb_high_U[] = {
    D(5, Ls4), P(L8), A(5, Lr8), D(6, Ls4), P(L4), END
};

static void *habanera_high[] = {
    hb_silent, hb_silent, hb_silent, hb_high_1, hb_high_2,
    hb_high_3, hb_high_4, hb_high_5, hb_high_2, hb_high_6,
    hb_high_7, hb_high_8, hb_high_2, hb_high_3, hb_high_4,
    hb_high_5, hb_high_2, hb_high_6, hb_high_7, hb_high_9,
    hb_high_A, hb_high_B, hb_high_C, hb_high_D, hb_high_A,
    hb_high_E, hb_high_F, hb_high_G, hb_high_H, hb_high_I, hb_high_J,
    hb_high_K, hb_high_L, hb_high_M, hb_high_N, hb_high_O, hb_high_P,
    hb_high_I, hb_high_Q, hb_high_R, hb_high_S, hb_high_M,
    hb_high_T, hb_high_U,
    NULL,
};

static char *habanera_fish[] = {
    "_______", "_______", "_______", "21_____", "01212__",            // 00012
    "3432___", "3xx5x43", "232____", "10101__", "2323___",            // 34526
    "2xx4x32", "345____", "43232__", "3234___", "5xx3x43",            // 78234
    "212____", "10101__", "2323___", "4xx6x56", "545____",            // 52679
    "43232__", "1010___", "1xx3x23", "454____", "56565__",            // ABCDA
    "4343___", "4xx6x54", "3454___", "5432___", "1212___", "345656_", // EFGHIJ
    "5434___", "3232___", "1232___", "123454_", "3212___", "3454___", // KLMNOP
    "3232___", "123454_", "5434___", "3212___", "1212___",            // IQRSM
    "32x34xx3", "234____",                                            // TU
    NULL,
};

static unsigned vc_bass[] = {
    P(L2),
    C(2, Lr4q), E(2, Lr4q), G(3, Lr8q), A(2, Lr8q), C(3, Lr4q),
    P(L2), END
};

static unsigned vc_high[] = {
    P(L2),
    C(4, Ls4), E(4, Ls4), G(4, Ls8), A(4, Ls8), C(5, Ls4),
    P(L2), END
};

static void *victory_bass[] = { vc_bass };
static void *victory_high[] = { vc_high };

void save_habanera(void) {
    print_level("habanera_fish", habanera_fish, alley_height, habanera_high);
    print_sheet("habanera_bass", habanera_bass);
    print_sheet("habanera_high", habanera_high);

    print_sheet("victory_bass", victory_bass);
    print_sheet("victory_high", victory_high);
}
