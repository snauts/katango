#include "notes.h"

#define L1		80
#define L2		40
#define L4		20
#define L8		10
#define L16		5
#define LXX		2

/* staccato */
#define Lx4		FADE(1, L4)
#define Lx8		FADE(1, L8)
#define Lx16		FADE(1, L16)

/* slur */
#define Ls2		FADE(2, L2)
#define Ls4		FADE(2, L4)
#define Ls8		FADE(2, L8)
#define Ls16		FADE(2, L16)
#define LsXX		FADE(2, LXX)

#define Lz2		FADE(6, L2)

static int flame_height[] = {
    184, 184, 184, 184, 184, 184, 184
};

static unsigned if_silent[] = {
    P(L2), END
};

static unsigned if_euph_0[] = {
    P(L4), P(L8), A(4, Ls16), Gs(4, Ls16), END
};

static unsigned if_euph_1[] = {
    G(4, Ls8), P(L4), P(L8), END
};

static unsigned if_euph_2[] = {
    F(5, Ls8), P(L8), B(4, Ls8), P(L8), END
};

static unsigned if_euph_3[] = {
    C(5, Ls8), G(5, Ls8), P(L4), END
};

static unsigned if_euph_4[] = {
    C(5, Ls4), P(L4), END
};

static unsigned if_euph_5[] = {
    D(5, LsXX), C(5, Ls8 - LXX), P(L4), P(L8), END
};

static unsigned if_euph_6[] = {
    D(5, LsXX), C(5, Ls8 - LXX), P(L8),
    D(5, LsXX), C(5, Ls8 - LXX), P(L8),
    END
};

static unsigned if_euph_7[] = {
    C(4, Lx8), G(4, Lx8), G(4, Lx8), A(4, Lx8), END
};

static unsigned if_euph_8[] = {
    G(4, Lx8), F(4, Lx8), F(4, Lx8), A(4, Lx8), END
};

static unsigned if_euph_9[] = {
    As(4, Lx8), D(5, Lx8), F(5, Lx8), D(5, Lx8), END
};

static unsigned if_euph_A[] = {
    D(5, Lx8), C(5, Lx8), C(5, Ls4), END
};

static unsigned if_euph_B[] = {
    D(5, Lx8), E(4, Lx8), E(4, Lx8), D(5, Lx8), END
};

static unsigned if_euph_C[] = {
    C(5, Lx8), F(4, Lx8), F(4, Lx8), A(4, Lx8), END
};

static unsigned if_euph_D[] = {
    A(4, Lx8), G(4, Lx8), A(4, Lx8), G(4, Lx8), END
};

static unsigned if_euph_E[] = {
    G(4, Lx8), F(4, Lx8), F(4, Lx4), END
};

static unsigned if_euph_F[] = {
    A(4, Ls4), F(4, Ls4), END
};

static unsigned if_euph_G[] = {
    D(4, Ls4), C(4, Ls4), END
};

static unsigned if_euph_H[] = {
    C(5, Lx8), G(4, Lx8), A(4, Lx8), As(4, Lx8), END
};

static unsigned if_euph_I[] = {
    A(4, Lx8), G(4, Lx8), F(4, Lx4), END
};

static unsigned if_euph_J[] = {
    C(4, Lx8), D(4, Lx8), Ds(4, Lx8), E(4, Lx8), END
};

static unsigned if_euph_K[] = {
    F(4, Ls8), C(4, Ls8), F(4, Ls8), C(4, Ls8), END
};

static unsigned if_euph_L[] = {
    F(4, Ls8 + L16), F(4, Ls16), F(4, Ls8 + L16), F(4, Ls16), END
};

static unsigned if_euph_M[] = {
    F(5, Ls8 + L16), F(5, Ls16), F(5, Ls8 + L16), F(5, Ls16), END
};

static unsigned if_euph_N[] = {
    F(5, Ls4), F(4, Ls4), END
};

static unsigned if_euph_O[] = {
    As(3, Ls2), END
};

static unsigned if_euph_P[] = {
    C(4, Ls8), Ds(4, Ls8), D(4, Ls8), C(4, Ls8), END
};

static unsigned if_euph_Q[] = {
    F(4, Ls4), F(4, Ls4), END
};

static unsigned if_euph_R[] = {
    F(4, Ls8), G(4, Ls8), D(4, Ls8), Ds(4, Ls8), END
};

static unsigned if_euph_S[] = {
    C(4, Ls4), C(4, Ls4), END
};

static unsigned if_euph_T[] = {
    As(3, Ls8), As(4, Ls8), A(4, Ls8), G(4, Ls8), END
};

static unsigned if_euph_U[] = {
    F(4, Ls8), Ds(4, Ls8), D(4, Ls8), C(4, Ls8), END
};

static unsigned if_euph_V[] = {
    As(3, Ls8), F(4, Ls8), C(4, Ls8), D(4, Ls8), END
};

static unsigned if_euph_W[] = {
    As(3, Ls4), F(4, Ls4), END
};

static unsigned if_euph_X[] = {
    As(3, Ls4), A(4, Ls4), END
};

static unsigned if_euph_Y[] = {
    G(4, Ls4), A(4, Ls4), END
};

static unsigned if_euph_Z[] = {
    As(4, Ls4), As(4, Ls4), END
};

static unsigned if_next_0[] = {
    G(4, Ls4), G(4, Ls4), END
};

static unsigned if_next_1[] = {
    A(4, Ls4), A(4, Ls4), END
};

static unsigned if_next_2[] = {
    F(4, Ls4), E(4, Ls4), END
};

static unsigned if_next_3[] = {
    F(4, Ls8), F(4, Ls8), F(4, Ls8), F(4, Ls8), END
};

static unsigned if_next_4[] = {
    F(4, Ls4), P(L4), END
};

static unsigned if_next_5[] = {
    A(4, Ls4), P(L4), END
};

static unsigned if_next_6[] = {
    F(4, Lz2), END
};

static unsigned if_next_7[] = {
    F(5, Lz2), END
};

static unsigned if_next_8[] = {
    F(5, FADE(7, L2)), END
};

static void *infernal_euph[] = {
    if_euph_0, if_euph_1, if_euph_2, if_euph_3,
    if_euph_0, if_euph_1, if_euph_2, if_euph_4,
    if_silent, if_euph_5, if_silent, if_euph_5,
    if_silent, if_euph_6, if_euph_6, if_euph_6,
    if_euph_7, if_euph_8, if_euph_9, if_euph_A,
    if_euph_B, if_euph_C, if_euph_D, if_euph_D,
    if_euph_7, if_euph_8, if_euph_9, if_euph_A,
    if_euph_B, if_euph_C, if_euph_D, if_euph_E,
    if_euph_F, if_euph_G, if_euph_H, if_euph_I,
    if_euph_F, if_euph_G, if_euph_J, if_euph_E,
    if_euph_F, if_euph_G, if_euph_H, if_euph_I,
    if_euph_F, if_euph_G, if_euph_J, if_euph_K,
    if_euph_K, if_euph_K, if_euph_K, if_euph_L,
    if_euph_L, if_euph_M, if_euph_N, /*bb*/ if_euph_O,
    if_euph_P, if_euph_Q, if_euph_R, if_euph_S,
    if_euph_P, if_euph_T, if_euph_U, if_euph_O,
    if_euph_P, if_euph_Q, if_euph_R, if_euph_S,
    if_euph_P, if_euph_V, if_euph_W, if_euph_O,
    if_euph_P, if_euph_Q, if_euph_R, if_euph_S,
    // 77
    if_euph_P, if_euph_T, if_euph_U, if_euph_O,
    if_euph_P, if_euph_Q, if_euph_R, if_euph_S,
    if_euph_P, if_euph_V, if_euph_X, if_euph_Y,
    if_euph_Z, if_next_0, if_next_1, if_euph_Y,

    if_euph_Z, if_next_0, if_next_2, if_next_2,
    if_next_2, if_next_2, if_next_3, if_next_3,
    if_next_3, if_next_3, if_next_3, if_next_3,
    if_next_3, if_next_3, if_euph_L, if_euph_L,

    if_next_4, if_next_5, if_next_4, if_next_5,
    if_next_6, if_next_6, if_next_7, if_next_7,
    if_next_8,
    NULL,
};

static unsigned if_tuba_0[] = {
    D(4, Ls8), P(L8), B(3, Ls8), P(L8), END
};

static unsigned if_tuba_1[] = {
    E(4, Ls8), G(4, Ls8), P(L4), END
};

static unsigned if_tuba_2[] = {
    E(4, Ls4), P(L4), END
};

static unsigned if_tuba_3[] = {
    D(4, LsXX), C(4, Ls8 - LXX), P(L4), P(L8), END
};

static unsigned if_tuba_4[] = {
    D(4, LsXX), C(4, Ls8 - LXX), P(L8),
    D(4, LsXX), C(4, Ls8 - LXX), P(L8),
    END
};

static unsigned if_tuba_5[] = {
    C(4, Lx8), P(L4), P(L8), END
};

static unsigned if_tuba_6[] = {
    D(4, Lx8), P(L8), F(4, Lx8), P(L8), END
};

static unsigned if_tuba_7[] = {
    F(4, Lx8), P(L8), P(L4), END
};

static unsigned if_tuba_8[] = {
    E(4, Lx8), P(L8), P(L4), END
};

static unsigned if_tuba_9[] = {
    F(4, Lx8), F(4, Lx8), F(4, Lx4), END
};

static unsigned if_tuba_A[] = {
    A(4, Ls4), F(4, Ls4), END
};

static unsigned if_tuba_B[] = {
    D(4, Ls4), C(4, Ls4), END
};

static unsigned if_tuba_C[] = {
    C(5, Lx8), P(L4), P(L8), END
};

static unsigned if_tuba_D[] = {
    A(4, Ls8), P(L8), As(4, Ls8), P(L8), END
};

static unsigned if_tuba_E[] = {
    A(4, Ls8), P(L8), G(4, Ls8), P(L8), END
};

static unsigned if_tuba_F[] = {
    F(4, Ls8), P(L8), E(4, Ls8), P(L8), END
};

static unsigned if_tuba_G[] = {
    F(4, Ls8), P(L8), E(4, Ls8), C(4, Ls8), END
};

static unsigned if_tuba_H[] = {
    F(4, Ls8 + L16), F(4, Ls16), F(4, Ls8 + L16), F(4, Ls16), END
};

static unsigned if_tuba_I[] = {
    F(4, Ls8 + L16), F(4, Ls16), F(4, Ls4), END
};

static unsigned if_tuba_J[] = {
    As(4, Ls8), P(L8), As(4, Ls8), P(L8), END
};

static unsigned if_tuba_K[] = {
    F(4, Ls8), P(L8), F(4, Ls8), P(L8), END
};

static unsigned if_tuba_L[] = {
    As(4, Ls8), P(L8), F(4, Ls8), P(L8), END
};

static unsigned if_tuba_M[] = {
    As(4, Ls4), F(4, Ls4), END
};

static unsigned if_tuba_N[] = {
    As(4, Ls4), F(4, Ls8), F(4, Ls8), END
};

static unsigned if_tuba_O[] = {
    E(4, Ls8), E(4, Ls8), Ds(4, Ls8), Ds(4, Ls8), END
};

static unsigned if_tuba_P[] = {
    D(4, Ls8), D(4, Ls8), As(4, Ls8), As(4, Ls8), END
};

static unsigned if_tuba_Q[] = {
    C(4, Ls8), C(4, Ls8), C(4, Ls8), C(4, Ls8), END
};

static unsigned if_tuba_R[] = {
    F(4, Ls8), F(4, Ls8), F(4, Ls8), F(4, Ls8), END
};

static unsigned if_tuba_S[] = {
    D(4, Ls8), D(4, Ls8), As(3, Ls8), As(3, Ls8), END
};

static unsigned if_tuba_T[] = {
    A(4, Ls8), C(4, Ls8), G(4, Ls8), C(4, Ls8), END
};

static unsigned if_tuba_U[] = {
    F(5, Ls8), A(4, Ls8), E(5, Ls8), As(4, Ls8), END
};

static unsigned if_tuba_V[] = {
    C(5, Ls8), F(4, Ls8), As(4, Ls8), E(4, Ls8), END
};

static unsigned if_tuba_W[] = {
    F(4, Ls4), F(5, Ls4), END
};

static unsigned if_tuba_X[] = {
    E(5, Ls4), D(5, Ls4), END
};

static unsigned if_tuba_Y[] = {
    C(5, Ls4), As(4, Ls4), END
};

static unsigned if_tuba_Z[] = {
    A(4, Ls4), G(4, Ls4), END
};

static unsigned if_stop_0[] = {
    F(4, Ls4), P(L4), END
};

static unsigned if_stop_1[] = {
    A(4, Ls4), P(L4), END
};

static unsigned if_stop_2[] = {
    F(4, Lz2), END
};

static unsigned if_stop_3[] = {
    F(4, FADE(7, L2)), END
};

static void *infernal_tuba[] = {
    if_silent, if_silent, if_tuba_0, if_tuba_1,
    if_silent, if_silent, if_tuba_0, if_tuba_2,
    if_tuba_3, if_silent, if_tuba_3, if_silent,
    if_tuba_4, if_tuba_4, if_tuba_4, if_tuba_4,
    if_tuba_5, if_tuba_5, if_tuba_6, if_tuba_7,
    if_tuba_8, if_tuba_7, if_tuba_7, if_tuba_8,
    if_tuba_5, if_tuba_5, if_tuba_6, if_tuba_7,
    if_tuba_8, if_tuba_7, if_tuba_8, if_tuba_9,
    if_tuba_A, if_tuba_B, if_tuba_C, if_tuba_9,
    if_tuba_A, if_tuba_B, if_tuba_C, if_tuba_9,
    if_tuba_A, if_tuba_B, if_tuba_C, if_tuba_9,
    if_tuba_A, if_tuba_B, if_tuba_C, if_tuba_D,
    if_tuba_E, if_tuba_F, if_tuba_G, if_tuba_H,
    if_tuba_H, if_tuba_H, if_tuba_I, /*bb*/ if_tuba_J,
    if_tuba_K, if_tuba_J, if_tuba_J, if_tuba_K,
    if_tuba_K, if_tuba_J, if_tuba_K, if_tuba_J,
    if_tuba_K, if_tuba_J, if_tuba_J, if_tuba_K,
    if_tuba_K, if_tuba_L, if_tuba_M, if_tuba_J,
    if_tuba_K, if_tuba_J, if_tuba_J, if_tuba_K,
    // 77
    if_tuba_K, if_tuba_J, if_tuba_K, if_tuba_J,
    if_tuba_K, if_tuba_J, if_tuba_J, if_tuba_K,
    if_tuba_K, if_tuba_L, if_tuba_N, if_tuba_O,
    if_tuba_P, if_tuba_Q, if_tuba_R, if_tuba_O,
    if_tuba_S, if_tuba_Q, if_tuba_T, if_tuba_U,
    if_tuba_V, if_tuba_T, if_tuba_W, if_tuba_X,
    if_tuba_Y, if_tuba_Z, if_tuba_W, if_tuba_X,
    if_tuba_Y, if_tuba_Z, if_tuba_H, if_tuba_H,

    if_stop_0, if_stop_1, if_stop_0, if_stop_1,
    if_stop_2, if_stop_2, if_stop_2, if_stop_2,
    if_stop_3,
    NULL,
};

static char *infernal_fish[] = {
    "________", "________", "________", "________",
    "________", "________", "________", "________",
    "________", "________", "________", "________",
    "________", "________", "________", "________",

    "210x____", "2345____", "x321____", "x34_____", // 789A
    "________", "________", "________", "________", // BCDD
    "________", "________", "________", "________", // 789A
    "________", "________", "________", "________", // BCDE
    "________", "________", "________", "________", // FGHI
    "________", "________", "________", "________", // FGJE
    "________", "________", "________", "________", // FGHI
    "________", "________", "________", "________", // FGJK
    "________", "________", "________", "________", // KKKL
    "________", "________", "________", "________", // LMNO
    "________", "________", "________", "________", // PQRS
    "________", "________", "________", "________", // PTUO
    "________", "________", "________", "________", // PQRS
    "________", "________", "________", "________", // PVWO
    "________", "________", "________", "________", // PQRS

    "________", "________", "________", "________", // PTUO
    "________", "________", "________", "________", // PQRS
    "________", "________", "________", "________", // PVXY
    "________", "________", "________", "________", // Z01Y

    "________", "________", "________", "________", // Z022
    "________", "________", "________", "________", // 2233
    "________", "________", "________", "________", // 3333
    "________", "________", "________", "________", // 33LL

    "________", "________", "________", "________", // 4545
    "________", "________", "________", "________", // 6677
    NULL,
};

void save_infernal(void) {
    print_level("infernal_fish", infernal_fish, flame_height, infernal_euph);
    print_sheet("infernal_euph", infernal_euph);
    print_sheet("infernal_tuba", infernal_tuba);
}
