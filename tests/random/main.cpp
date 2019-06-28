#include <random>
#include <string>
#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <stdlib.h>

#include <fc/smart_ref_impl.hpp>
#include <fc/crypto/hmac.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/crypto/bigint.hpp>

extern "C" {
#include "TestU01.h"

}

/*
 *
 * test with double 10000
 * ========= Summary results of SmallCrush =========

 Version:          TestU01 1.2.3
 Generator:        std mt19937
 Number of statistics:  15
 Total CPU time:   00:15:48.33
 The following tests gave p-values outside [0.001, 0.9990]:
 (eps  means a value < 1.0e-300):
 (eps1 means a value < 1.0e-15):

       Test                          p-value
 ----------------------------------------------
  1  BirthdaySpacings                 eps
  2  Collision                        eps
  3  Gap                              eps
  6  MaxOft                           eps
 10  RandomWalk1 H                   6.8e-7
 10  RandomWalk1 M                   1.0e-6
 10  RandomWalk1 C                  7.9e-10
 ----------------------------------------------
 All other tests were passed


 */


char *itoa(long n)
{
    int len = n==0 ? 1 : floor(log10l(labs(n)))+1;
    if (n<0) len++; // room for negative sign '-'

    char    *buf = (char*)calloc(sizeof(char), len+1); // +1 for null
    snprintf(buf, len+1, "%ld", n);
    return   buf;
}

// GENERATOR SECTION
//
// This is the only part of the code that needs to change to switch to
// a new generator.

const char* gen_name = "std mt19937";  // TestU01 doesn't like colons!!?!
const char* chainId = "bb2aeb9eebaaa29d79ed81699ee49a912c19c59b9350f8f8d3d81b12fa178495";

const int MAX_SEEDS = 1;
uint64_t seed_data[MAX_SEEDS];

uint64_t uniform_int(uint64_t n, uint64_t max) {
//    return n % max;
    uint64_t offset = 0;
    uint64_t range = RAND_MAX;
    uint64_t scale;
    uint64_t k;

    if (n > range || n == 0) {
        return 0;
    }

    scale = range / max;

    do {
        k = (n - offset) / scale;
    } while ( k >= n );

    return k;
}

uint64_t generate_random(std::string c, std::string data, uint64_t min, uint64_t max) {
    fc::hmac_sha512 mac;
    fc::sha512 l = mac.digest( c.c_str(), c.size(), data.c_str(), data.size() );
    fc::bigint n = fc::bigint(l.data(), l.data_size());
    return (fc::bigint(min) + (n % fc::bigint(max-min))).to_int64();
}

uint64_t reject_sampling(std::string c, std::string data, uint64_t min, uint64_t max) {
    uint64_t usable = UINT32_MAX / (max-min);
    const char *current_data = data.c_str();
    size_t current_data_size = data.size();
    uint64_t retrieved = 0;

    do {
        fc::hmac_sha256 mac;
        fc::sha256 h = mac.digest( c.c_str(), c.size(), current_data, current_data_size );
        current_data = h.data();
        current_data_size = h.data_size();
        retrieved = generate_random(c, h, 0, UINT32_MAX);
    } while(retrieved < usable);

    return (retrieved % max) + min;
}

int blockId = 30000;
uint32_t gen32()
{
//    static std::mt19937 rng(seed_data[0]);
//
//    return rng();
    blockId ++;
    char *buffer = itoa(blockId);
    uint32_t res = uint32_t(reject_sampling(chainId,buffer , 0, 100));
    //printf("%d ", res);
    free(buffer);
    return res;
}

//double gen32d(uint32_t g, float n) {
//    return double(g/n);
//}

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


void play_dice() {
    int wins = 0;
    int win_streaks = 0;
    int num_streaks = 0;
    int last_win = 0;
    float balance = 1;
    static std::mt19937 rng(seed_data[0]);

    for (int i=0; i < 200000; i++) {
        char *buffer = itoa(i);
        uint32_t n = reject_sampling(chainId,buffer , 0, 100);
        if (n >= 50) {
            wins++;
            last_win++;
            win_streaks = MAX(win_streaks, last_win);
            float win_amount = (balance/8.0);
            balance = balance + win_amount;
        } else {
            last_win = 0;
            balance = balance - (balance/8.0);
        }
        if (n == 100) {
            printf("Found 100!\n");
            break;
        }
        printf("wins = %f balance =%f win=%d (%d)\n", wins/float(i), balance, n>=50, n);
    }
    printf("Wins = %d rate=%f\n", wins, wins/200000.0);
    printf("Streak = %d profit =%f\n", win_streaks, balance);
}

// END OF GENERATOR SECTION

inline uint32_t rev32(uint32_t v)
{
    // https://graphics.stanford.edu/~seander/bithacks.html
    // swap odd and even bits
    v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
    // swap consecutive pairs
    v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
    // swap nibbles ...
    v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
    // swap bytes
    v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
    // swap 2-byte-long pairs
    v = ( v >> 16             ) | ( v               << 16);
    return v;
}

uint32_t gen32_rev()
{
    return rev32(gen32());
}

const char* progname;

void usage()
{
    printf("%s: [-v] [-r] [seeds...]\n", progname);
    exit(1);
}

int main (int argc, char** argv)
{
    play_dice();
}

int main2 (int argc, char** argv)
{
    progname = argv[0];

    // Config options for TestU01
    swrite_Basic = FALSE;  // Turn of TestU01 verbosity by default
    // reenable by -v option.

    // Config options for generator output
    bool reverseBits = false;

    // Config options for tests
    bool testSmallCrush = false;
    bool testCrush = false;
    bool testBigCrush = false;
    bool testLinComp = false;

    // Handle command-line option switches
    while (1) {
        --argc; ++argv;
        if ((argc == 0) || (argv[0][0] != '-'))
            break;
        if ((argv[0][1]=='\0') || (argv[0][2]!='\0'))
            usage();
        switch(argv[0][1]) {
            case 'r':
                reverseBits = true;
                break;
            case 's':
                testSmallCrush = true;
                break;
            case 'm':
                testCrush = true;
                break;
            case 'b':
                testBigCrush = true;
                break;
            case 'l':
                testLinComp = true;
                break;
            case 'v':
                swrite_Basic = TRUE;
                break;
            default:
                usage();
        }
    }

    // Name of the generator

    std::string genName = gen_name;
    if (reverseBits)
        genName += " [Reversed]";

    // Determine a default test if need be

    if (!(testSmallCrush || testCrush || testBigCrush || testLinComp)) {
        testCrush = true;
    }

    // Initialize seed-data array, either using command-line arguments
    // or std::random_device.

    printf("Testing %s:\n", genName.c_str());
    printf("- seed_data[%u] = { ", MAX_SEEDS);
    std::random_device rdev;
    for (int i = 0; i < MAX_SEEDS; ++i) {
        if (argc >= i+1) {
            seed_data[i] = strtoull(argv[i],0,0);
        } else {
            seed_data[i] = (uint64_t(rdev()) << 32) | rdev();
        }
        printf("%s0x%016lx", i == 0 ? "" : ", ", seed_data[i]);
    }
    printf("}\n");
    fflush(stdout);

    // Create a generator for TestU01.

    unif01_Gen* gen =
            unif01_CreateExternGenBits((char*) genName.c_str(),gen32);

    // Run tests.

    if (testSmallCrush) {
        bbattery_SmallCrush(gen);
        fflush(stdout);
    }
    if (testCrush) {
        bbattery_Crush(gen);
        fflush(stdout);
    }
    if (testBigCrush) {
        bbattery_BigCrush(gen);
        fflush(stdout);
    }
    if (testLinComp) {
        scomp_Res* res = scomp_CreateRes();
        swrite_Basic = TRUE;
        for (int size : {250, 500, 1000, 5000, 25000, 50000, 75000})
            scomp_LinearComp(gen, res, 1, size, 0, 1);
        scomp_DeleteRes(res);
        fflush(stdout);
    }

    // Clean up.

    unif01_DeleteExternGenBits(gen);

    return 0;
}