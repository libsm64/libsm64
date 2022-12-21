#pragma once
#include "convUtils.h"

#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "convUtils.h"
#include "convTypes.h"

/**
 * This code is based on the only documentation that exists (that I know of) for the SM64 CTL/TBL format
 * as well as dylanpdx's audio extraction implementation.
 * https://github.com/n64decomp/sm64/blob/1372ae1bb7cbedc03df366393188f4f05dcfc422/tools/disassemble_sound.py
 * https://github.com/n64decomp/sm64/blob/1372ae1bb7cbedc03df366393188f4f05dcfc422/tools/assemble_sound.py
 * https://github.com/Retro64Mod/libsm64-retro64
 */
 
#define ALIGN16(val) (((val) + 0xF) & ~0xF)

unsigned char* gCtlSeqs;

struct seqFile* parse_seqfile(unsigned char* seq){ /* Read SeqFile data */
    short revision = read_u16_be(seq);
    short bankCount = read_u16_be(seq + 2);

    unsigned int size = sizeof(struct seqFile) + (bankCount-1) * sizeof(struct seqObject);

    struct seqFile* seqFile = (struct seqFile*)calloc(size, 1);
    seqFile->revision = revision;
    seqFile->seqCount = bankCount;
    for (int i = 0; i < bankCount; i++){ // read bank offsets and sizes
        seqFile->seqArray[i].offset = (uintptr_t)read_u32_be(seq + 4 + i * 8);
        seqFile->seqArray[i].len = read_u32_be((seq + 4 + i * 8 + 4));
    }

    if (revision == TYPE_CTL){
        // CTL file, contains instrument and drum data, this is really the only one that needs to be parsed, the rest only needs a header change
        gCtlSeqs = (unsigned char*)calloc(0x20B40, 1); // We only really need 0x20AD0 bytes but still
        uintptr_t pos = (uintptr_t)gCtlSeqs;
        for (int i = 0; i < bankCount; i++){
            uintptr_t start = pos;
            struct CTL* ptr = parse_ctl_data(seq+(seqFile->seqArray[i].offset), &pos);
            seqFile->seqArray[i].offset = ptr;
            seqFile->seqArray[i].len = (unsigned int)(pos - start);
        }
    }else if (revision == TYPE_TBL){
        // TBL file, contains raw audio data
        for (int i = 0; i < bankCount; i++){
            seqFile->seqArray[i].offset = seq+(seqFile->seqArray[i].offset);
        }
    }else if (revision == TYPE_SEQ){
        // SEQ file, contains music files (*.m64)
        for (int i = 0; i < bankCount; i++){
            seqFile->seqArray[i].offset = seq+(seqFile->seqArray[i].offset);
        }
    }

    return seqFile;
}

void ctl_free(){
    free(gCtlSeqs);
}

void snd_ptrs_to_offsets(struct CSound* snd, uintptr_t ctlData){
    struct CSample* smp = snd->sample_addr;
    if((uintptr_t)(smp->loop) > ctlData)
        smp->loop = (struct CLoop*)((uintptr_t)(smp->loop) - ctlData);
    if((uintptr_t)(smp->book) > ctlData)
        smp->book = (struct CBook*)((uintptr_t)(smp->book) - ctlData);
    snd->sample_addr = (struct CSample*)((uintptr_t)(snd->sample_addr) - ctlData);
}

void ptrs_to_offsets(struct seqFile* ctl){
    if (ctl->revision != TYPE_CTL){
        return;
    }

    for (int i = 0; i < ctl->seqCount; i++){
        struct CTL* ptr = (struct CTL*)ctl->seqArray[i].offset;
        uintptr_t ctlData = (uintptr_t)ptr + 0x10;
        // find all samples in the CTL file
        for (int j = 0; j < ptr->numInstruments; j++){
            struct CInstrument* inst = ptr->instrument_pointers[j];
            if (inst==0x0)
                continue; // null instrument.
            inst->env_addr = (struct CEnvelope*)((uintptr_t)inst->env_addr - ctlData);
            if (inst->sound_hi.sample_addr!=0x0){
                snd_ptrs_to_offsets(&(inst->sound_hi), ctlData);
            }
            if (inst->sound_med.sample_addr!=0x0){
                snd_ptrs_to_offsets(&(inst->sound_med), ctlData);
            }
            if (inst->sound_lo.sample_addr!=0x0){
                snd_ptrs_to_offsets(&(inst->sound_lo), ctlData);
            }
            ptr->instrument_pointers[j] = (struct CInstrument*)((uintptr_t)(inst) - ctlData);
        }
        if(ptr->numDrums != 0){
            for (int j = 0; j < ptr->numDrums; j++){
                struct CDrum* drum = ptr->drum_pointers[j];
                if (drum==0x0)
                    continue; // null drum.
                drum->env_addr = (struct CEnvelope*)((uintptr_t)drum->env_addr - ctlData);
                if (drum->snd.sample_addr!=0x0){
                    snd_ptrs_to_offsets(&(drum->snd), ctlData);
                }
                ptr->drum_pointers[j] = (struct CDrum*)((uintptr_t)(drum) - ctlData);
            }
            ptr->drum_pointers = (struct CDrum**)((uintptr_t)(ptr->drum_pointers) - ctlData);
        }
    }
}

struct CLoop* parse_loop(unsigned char* loop, uintptr_t* pos){
    uint32_t count = read_u32_be(loop + 8); // variable is signed, but the data is being read as unsigned.
    unsigned int size = sizeof(struct CLoop) - 4;
    if(count != 0){
        size = sizeof(struct CLoop) - 4 + sizeof(short) * 16;
    }
    struct CLoop* loop_ptr = (struct CLoop*)(*pos);
    *pos += size;
    *pos = ALIGN16(*pos);

    loop_ptr->start = read_u32_be(loop);
    loop_ptr->end = read_u32_be(loop + 4);
    loop_ptr->count = count;
    loop_ptr->pad = read_u32_be(loop + 12);

    if (loop_ptr->count!=0){
        for (int i = 0;i<16;i++){
            loop_ptr->state[i]=read_u16_be(loop + 16 + i*2);
        }
    }

    return loop_ptr;
}

struct CBook* parse_book(unsigned char* book, uintptr_t* pos){
    struct CBook* book_ptr = (struct CBook*)(*pos);
    *pos += sizeof(struct CBook);
    *pos = ALIGN16(*pos);
    book_ptr->order = read_u32_be(book);
    book_ptr->npredictors = read_u32_be(book + 4); // both are signed
    unsigned char* table_data = book+8;
    for (int i = 0; i < 8 * book_ptr->order * book_ptr->npredictors; i ++){
        book_ptr->table[i] = read_u16_be(table_data + i * 2);
    }
    return book_ptr;
}

struct CSample* parse_sample(unsigned char* sample,unsigned char* ctl, uintptr_t* pos){
    struct CSample* samp = (struct CSample*)(*pos);
    *pos += sizeof(struct CSample);
    *pos = ALIGN16(*pos);
    samp->zero=read_u32_be(sample);
    samp->addr=read_u32_be(sample+4);
    samp->loop=read_u32_be(sample+8);// loop address
    samp->book=read_u32_be(sample+12);// book address
    samp->sample_size=read_u32_be(sample+16);

    samp->book=parse_book(ctl+((uintptr_t)samp->book), pos);
    samp->loop=parse_loop(ctl+((uintptr_t)samp->loop), pos);
    return samp;
}

struct CSound* parse_sound(unsigned char* sound,unsigned char* ctl, uintptr_t* pos, uintptr_t sndPos, struct SampleList* samples){
    struct CSound* snd = (struct CSound*)(sndPos);
    snd->sample_addr=read_u32_be(sound);
    snd->tuning = (float)read_f32_be(sound+4);
    // if sample_addr is 0 then the sound is null
    if (snd->sample_addr!=0){
        int smpIndex = -1;
        for(int i = 0; i < samples->count; i++){
            if(samples->orig_addrs[i] == (uintptr_t)(snd->sample_addr)){
                smpIndex = i;
                break;
            }
        }
        if(smpIndex < 0){
            samples->orig_addrs[samples->count] = (uintptr_t)(snd->sample_addr);
            snd->sample_addr = parse_sample(ctl+((uintptr_t)snd->sample_addr),ctl, pos);
            samples->addrs[samples->count] = snd->sample_addr;
            samples->count++;
        } else {
            snd->sample_addr = samples->addrs[smpIndex];
        }
    }
    return snd;
}

struct CDrum* parse_drum(unsigned char* drum,unsigned char* ctl, uintptr_t* pos, struct SampleList* samples){ /* Read Drum data */
    struct CDrum* drumData = malloc(sizeof(struct CDrum));
    drumData->release_rate = drum[0];
    drumData->pan = drum[1];
    drumData->loaded = drum[2];
    drumData->pad = drum[3];
    drumData->snd=*parse_sound(drum+4,ctl, pos, &drumData->snd, samples);
    drumData->env_addr=read_u32_be(drum+12);
    return drumData;
}

struct CEnvelope* parse_envelope(unsigned char* env, uintptr_t* pos, int* size){
    int count = 0;
    while(1){
        unsigned short delay = read_u16_le(env + count * 4);
        unsigned short arg = read_u16_le(env + count * 4 + 2);
        unsigned short delayC = (-delay);
        count++;
        if ((1 <= delayC && delayC <= 3) || delay == 0)
            break;
    }
    *size = sizeof(struct CEnvelope) + sizeof(struct delay_arg) * (count-1);
    struct CEnvelope* envData = malloc(*size);
    for (int i = 0; i < count; i++){
        envData->delay_args[i].delay = read_u16_le(env + i * 4);
        envData->delay_args[i].arg = read_u16_le(env + i * 4 + 2);
    }
    return envData;
}

struct CInstrument* parse_instrument(unsigned char* instrument,unsigned char* ctl, uintptr_t* pos, struct SampleList* samples){
    struct CInstrument* inst = malloc(sizeof(struct CInstrument));
    inst->loaded = instrument[0];
    inst->normal_range_lo = instrument[1];
    inst->normal_range_hi = instrument[2];
    inst->release_rate = instrument[3];
    inst->env_addr=read_u32_be(instrument+4);
    inst->sound_lo=*parse_sound(instrument+8,ctl, pos, &(inst->sound_lo), samples);
    inst->sound_med=*parse_sound(instrument+16,ctl, pos, &(inst->sound_med), samples);
    inst->sound_hi=*parse_sound(instrument+24,ctl, pos, &(inst->sound_hi), samples);

    return inst;
}

struct TBL* parse_tbl_data(unsigned char* tbl){
    struct TBL* tblData = malloc(sizeof(struct TBL));
    tblData->data = tbl;
    return tblData;
}

    struct SEQ* parse_seq_data(unsigned char* seq){
    struct SEQ* seqData = malloc(sizeof(struct SEQ));
    seqData->data = seq;
    return seqData;
}

struct CTL* parse_ctl_data(unsigned char* ctlData, uintptr_t* pos){
    int instruments=read_u32_be(ctlData);
    unsigned int size = sizeof(struct CTL) + sizeof(struct CInstrument*) * (instruments-1);
    struct CTL* ctl = (struct CTL*)(*pos);
    *pos += size;
    *pos = ALIGN16(*pos);
    #pragma region Parse CTL header
    ctl->numInstruments = read_u32_be(ctlData);
    ctl->numDrums = read_u32_be(ctlData + 4);
    ctl->shared = read_u32_be(ctlData + 8);
    ctl->iso_date = read_u32_be(ctlData + 12);
    #pragma endregion
    struct SampleList samples = {0};
    struct EnvelopeMeta envData[128] = {0};
    int envCount = 0;
    samples.count = 0;
    // header parsed, now read data
    if(ctl->numDrums != 0) {
        ctl->drum_pointers= (struct CDrum**)(*pos);
        size = sizeof(struct CDrum*) * ctl->numDrums;
        *pos += size;
        *pos = ALIGN16(*pos);
        int drumTablePtr = read_u32_be(ctlData + 16);
        for (int i = 0; i < ctl->numDrums; i++){
            uint32_t data = read_u32_be(ctlData + drumTablePtr+16 + i * 4);
            
            struct CDrum* d = parse_drum(ctlData+data+16,ctlData+16, pos, &samples);
            bool used = 0;
            for(int j = 0; j < envCount; j++){
                if(envData[j].orig == (uintptr_t)d->env_addr){
                    used = 1;
                    break;
                }
            }
            if(used == 0){
                int size = 0;
                unsigned char* addr = ctlData+((uintptr_t)d->env_addr)+16;
                envData[envCount].orig = (uintptr_t)(d->env_addr);
                envData[envCount].addr = parse_envelope(addr, pos, &size);
                envData[envCount].size = size;
                envCount++;
            }
            ctl->drum_pointers[i] = d;
        }
        *pos = ALIGN16(*pos);
    } else {
        ctl->drum_pointers= NULL;
    }
    // parse instrument data
    int instTablePtr = 4;
    for (int i = 0; i < ctl->numInstruments; i++){
        uint32_t data = read_u32_be(ctlData + 16 + instTablePtr + i * 4);
        if (data == 0)
            continue;
        struct CInstrument* inst = parse_instrument(ctlData+16+data,ctlData+16, pos, &samples);
        bool used = 0;
        for(int j = 0; j < envCount; j++){
            if(envData[j].orig == (uintptr_t)inst->env_addr){
                used = 1;
                break;
            }
        }
        if(used == 0){
            int size = 0;
            unsigned char* addr = ctlData+((uintptr_t)inst->env_addr)+16;
            envData[envCount].orig = (uintptr_t)(inst->env_addr);
            envData[envCount].addr = parse_envelope(addr, pos, &size);
            envData[envCount].size = size;
            envCount++;
        }
        ctl->instrument_pointers[i] = inst;
    }
    *pos = ALIGN16(*pos);

    // Copy envelopes to ctl
    for (int i = 0; i < envCount; i++){
        struct CEnvelope* env = envData[i].addr;
        memcpy((uint8_t*)(*pos), env, envData[i].size);
        for (int j = 0; j < ctl->numInstruments; j++){
            struct CInstrument* inst = ctl->instrument_pointers[j];
            if (inst == 0x0)
                continue;
            if((uintptr_t)(inst->env_addr) == envData[i].orig){
                inst->env_addr = (struct CEnvelope*)(*pos);
            }
        }
        for (int j = 0; j < ctl->numDrums; j++){
            struct CDrum* drum = ctl->drum_pointers[j];
            if (drum == 0x0)
                continue;
            if((uintptr_t)(drum->env_addr) == envData[i].orig){
                drum->env_addr = (struct CEnvelope*)(*pos);
            }
        }
        free(env);
        *pos += envData[i].size;
    }
    *pos = ALIGN16(*pos);

    // Copy instruments to ctl
    for (int i = 0; i < ctl->numInstruments; i++){
        struct CInstrument* inst = ctl->instrument_pointers[i];
        if (inst == 0x0)
            continue;
        memcpy((uint8_t*)(*pos), inst, sizeof(struct CInstrument));
        free(inst);
        ctl->instrument_pointers[i] = (struct CInstrument*)(*pos);
        *pos += sizeof(struct CInstrument);
    }
    *pos = ALIGN16(*pos);

    // Copy drums to ctl
    for (int i = 0; i < ctl->numDrums; i++){
        struct CDrum* drum = ctl->drum_pointers[i];
        if (drum == 0x0)
            continue;
        memcpy((uint8_t*)(*pos), drum, sizeof(struct CDrum));
        free(drum);
        ctl->drum_pointers[i] = (struct CDrum*)(*pos);
        *pos += sizeof(struct CDrum);
    }
    *pos = ALIGN16(*pos);
    // 

    return ctl;
}