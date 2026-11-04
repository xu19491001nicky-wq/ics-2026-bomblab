#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t lo = (uint32_t)challenge;
    uint32_t mid = (uint32_t)(challenge >> 16);
    uint32_t hi = (uint32_t)(challenge >> 32);

    uint32_t s2 = (hi * 5) ^ (lo + 0x84da);
    uint32_t s0 = (lo + mid * 2) ^ 0xe6c4;
    uint32_t s1 = (s2 + 0xcb3b) ^ s0;

    values[0] = s1;

    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;

    values[0] = (uint32_t)((challenge >> 42) & 0x1f);
    for (size_t i = 1; i <= 5; ++i) {
        uint32_t shift = 15 + (uint32_t)(i - 1) * 5;
        uint32_t val = (uint32_t)((challenge >> shift) & 0x3f) + values[i - 1];
        values[i] = val ^ (uint32_t)i;
    }

    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    values[0] = (uint32_t)(challenge & 0x7);
    uint32_t case_idx = values[0];
    uint32_t param = (uint32_t)(challenge >> 3) ^ (uint32_t)(challenge >> 32);

    switch (case_idx) {
        case 0:
            values[1] = (param ^ 0x23db) + (param >> 1);
            break;
        case 1:
            values[1] = (param * 4) ^ 0xc840;
            break;
        case 2:
            values[1] = (param + param) ^ 0x66e4;
            break;
        case 3:
            values[1] = param + param * 4 + 0x8c78;
            break;
        case 4:
            values[1] = (param ^ 0xe0c1) + (param >> 1);
            break;
        case 5:
            values[1] = param + param * 2 + 0xa4b7;
            break;
        case 6:
            values[1] = param + param * 4 + 0x259d6;
            break;
        case 7:
            values[1] = param + param * 4 + 0x4e23c;
            break;
        default:
            values[1] = 0;
            break;
    }

    return write_answer(out, cap, values, count);
}

static uint32_t walk_sim(uint32_t target_val, uint32_t lo, uint32_t hi, uint32_t c) {
    uint32_t mid = lo + ((hi - lo) >> 1);
    uint32_t ebx = mid + c;
    if (target_val == mid) {
        return (mid + 0xad) ^ c;
    } else if (target_val < mid) {
        uint32_t sub = walk_sim(target_val, lo, mid - 1, c);
        return ((sub + sub * 4) ^ ebx) ^ 0xc;
    } else {
        uint32_t sub = walk_sim(target_val, mid + 1, hi, c);
        return ((sub + sub * 4) ^ ebx) ^ 0xf;
    }
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t target = (uint32_t)challenge & 0x7f;
    uint32_t rcx = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);

    values[0] = walk_sim(target, 0, 0x7f, rcx);

    return write_answer(out, cap, values, count);
}

struct P5Node {
    int next_idx;
    uint32_t value;
    uint32_t salt;
};

static const struct P5Node p5_nodes[12] = {
    { 4,  0x00000047, 0x0000f68d },
    { 11, 0x00000525, 0x0000c593 },
    { -1, 0x0000025c, 0x000033b7 },
    { 5,  0x00000327, 0x0000bcb6 },
    { 6,  0x0000085d, 0x0000a436 },
    { 8,  0x00000a67, 0x0000158f },
    { 9,  0x00000120, 0x0000174d },
    { 3,  0x00000414, 0x00001b89 },
    { 10, 0x00000785, 0x0000923d },
    { 1,  0x00000639, 0x00006e47 },
    { 2,  0x00000948, 0x00004e89 },
    { 7,  0x00000b21, 0x000070e7 }
};

static const int p5_heads[8] = { 0, 4, 6, 9, 1, 11, 7, 3 };

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    int head_idx = p5_heads[challenge & 0x7];
    int steps = (int)((challenge >> 3) & 0x3);

    uint32_t acc = (uint32_t)(challenge >> 6);
    acc ^= (uint32_t)(challenge >> 32);
    acc ^= 0xe96c;

    int curr = head_idx;
    uint32_t last_val = 0;

    for (int i = 0; i <= steps; ++i) {
        acc += p5_nodes[curr].salt;
        acc = acc + (acc << 2);
        acc ^= p5_nodes[curr].value;
        last_val = p5_nodes[curr].value;
        if (i < steps) {
            curr = p5_nodes[curr].next_idx;
        }
    }

    values[0] = acc;
    values[1] = last_val;

    return write_answer(out, cap, values, count);
}

static uint32_t p6_op_0(uint32_t edi) {
    uint32_t eax = edi;
    eax = (eax >> 5) ^ edi;
    eax += 0xe7f9;
    return eax;
}

static uint32_t p6_op_1(uint32_t edi) {
    uint32_t eax = edi;
    eax = (eax >> 1) | (eax << 31);
    return eax;
}

static uint32_t p6_op_2(uint32_t edi) {
    uint32_t eax = edi;
    uint32_t shifted = edi << 5;
    eax ^= 0x98e8;
    eax += shifted;
    return eax;
}

static uint32_t p6_op_3(uint32_t edi) {
    uint32_t eax = edi + 0x1e55;
    uint32_t shr = edi >> 1;
    eax ^= shr;
    return eax;
}

static uint32_t p6_op_4(uint32_t edi) {
    uint32_t edx = edi * 4;
    uint32_t eax = edx + 0x6d0d;
    edx += edi;
    eax ^= edx;
    return eax;
}

static uint32_t p6_op_5(uint32_t edi) {
    uint32_t eax = edi;
    eax = (eax << 5) | (eax >> 27);
    eax ^= 0xd358;
    return eax;
}

static uint32_t run_p6_op(uint32_t idx, uint32_t edi) {
    switch (idx & 0x7) {
        case 0: return p6_op_1(edi);
        case 1: return p6_op_3(edi);
        case 2: return p6_op_5(edi);
        case 3: return p6_op_3(edi);
        case 4: return p6_op_0(edi);
        case 5: return p6_op_5(edi);
        case 6: return p6_op_4(edi);
        case 7: return p6_op_2(edi);
        default: return p6_op_0(edi);
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint64_t rdi = challenge;
    uint32_t rbx = (uint32_t)rdi;
    uint32_t edi_init = (uint32_t)(rdi >> 6);
    uint32_t rax_high = (uint32_t)(rdi >> 32);
    uint32_t edi = edi_init ^ rax_high;

    uint32_t idx1 = rbx & 0x7;
    uint32_t idx2 = (rbx >> 3) & 0x7;

    uint32_t res1 = run_p6_op(idx1, edi);
    res1 ^= 0x865c;
    uint32_t res2 = run_p6_op(idx2, res1);
    res2 += 0x4694;

    values[0] = res2;

    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t number1 = (uint32_t)context & 3;

    uint32_t a = (uint32_t)challenge ^ (uint32_t)(challenge >> 32);
    uint32_t b = (uint32_t)context ^ (uint32_t)(context >> 32);

    uint32_t mixed = ((a ^ b) * 3 + 0xc14b) ^ 0xb44;
    uint32_t extra = number1 * 0xc14b ^ (uint32_t)(context >> 16);
    uint32_t number2 = mixed ^ extra;

    uint32_t number3 = number2 ^ (b + number1 + 0xb44 + (a >> 5));

    int written = snprintf(
        out,
        cap,
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOPEN%" PRIu32 " %" PRIu32 " %" PRIu32,
        number1,
        number2,
        number3
    );

    return (written < 0 || (size_t)written >= cap) ? 1 : 0;
}
