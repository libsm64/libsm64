#pragma once

#include <stdint.h>
#include <stdlib.h>

#define TYPE_CTL 1
#define TYPE_TBL 2
#define TYPE_SEQ 3

struct CLoop
{
    unsigned int start;
    unsigned int end;
    int count;
    unsigned int pad;
    short state[1];
};


struct CBook
{
    int order; // must be 2
    int npredictors; // must be 2
    short table[32]; // 8 * order * npredictors
};

struct CSample{
    unsigned int zero;
    uintptr_t addr;
    struct CLoop* loop; // must not be null
    struct CBook* book; // must not be null
    unsigned int sample_size;
};

struct CSound{
    struct CSample* sample_addr;
    float tuning;
};

struct delay_arg{
    unsigned short delay;
    unsigned short arg;
};

struct CEnvelope{
    struct delay_arg delay_args[1]; // array of [(delay,arg)]
};

struct CDrum{
    unsigned char release_rate;
    unsigned char pan;
    unsigned char loaded;
    unsigned char pad;
    struct CSound snd;
    struct CEnvelope* env_addr;
};

struct CInstrument{
    unsigned char loaded;
    unsigned char normal_range_lo;
    unsigned char normal_range_hi;
    unsigned char release_rate;
    struct CEnvelope* env_addr;
    struct CSound sound_lo;
    struct CSound sound_med;
    struct CSound sound_hi;
};

struct TBL{
    unsigned char* data;
};

struct SEQ{
    unsigned char* data;
};

struct CTL 
{
    unsigned int numInstruments;
    unsigned int numDrums;
    unsigned int shared;
    unsigned int iso_date;
    struct CDrum** drum_pointers;
    struct CInstrument* instrument_pointers[1];
};

struct seqObject{
    uintptr_t offset __attribute__((aligned (8)));
    unsigned int len __attribute__((aligned (8)));
};

struct seqFile{
    unsigned short revision;
    unsigned short seqCount;
	unsigned int pad;
    struct seqObject seqArray[1];
} __attribute__((aligned (16)));