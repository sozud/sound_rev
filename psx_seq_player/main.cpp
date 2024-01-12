
#ifdef PSX
#include <SYS/TYPES.H>
#else
#include <sys/types.h>
#endif
#include "lib_snd.hpp"
#include "lib_spu.hpp"
#include "Sound.hpp"

#include "mines.vb.h"
#include "mines.vh.h"
#include "miseq.bsq.h"
#include "SeqChunkParser.hpp"
#include "test.hpp"

#ifdef PSX
#include <STDLIB.H>
#else
#include <stdlib.h>
#endif

#ifndef PSX
#include "../mednafen/spu.h"
#include "../mednafen/dma.h"
#include <string>

#define SDL_MAIN_HANDLED
#include <SDL.h>
//#include <SDL_main.h>

PS_SPU* SPU = nullptr;

void SetSpuReg(volatile u32* pReg, u32 value)
{
    if (pReg == (volatile u32*)0x1f8010f0)
    {
        MDFN_IEN_PSX::DMA_Write(0, (uint32)pReg, value);
    }
    else
    {
        SPU->Write(0, (uint32_t)pReg, value);
        SPU->Write(0, ((uint32_t)pReg) | 2, value >> 16); // TODO: Check |2 hack works
    }
}

void SetSpuReg(volatile u16* pReg, u16 value)
{
    SPU->Write(0, (uint32_t)pReg, value);
}

void SetSpuReg(volatile short* pReg, short value)
{
    SPU->Write(0, (uint32_t)pReg, value);
}

u32 GetSpuRegU32(volatile u32* pReg)
{
    if (pReg == (volatile u32*)0x1f8010f0)
    {
        return MDFN_IEN_PSX::DMA_Read(0, (uint32)pReg);
    }

    // TODO: needs to be done as 2x reads ??
    const u16 lo = SPU->Read(0, (uint32_t)pReg);
    const u16 hi = SPU->Read(0, ((uint32_t)pReg) | 2); // TODO: Check |2 hack works

    return lo | ((u32)hi >> 16);
}

u16 GetSpuRegU16(volatile u16* pReg)
{
    return SPU->Read(0, (uint32_t)pReg);
}


static Sound gSound;
// static SeqChunkParser gChunkParser;

static void VSync(int mode)
{

}

#include <memory.h>

bool ready = false;

double accum = 0;

extern "C"
void my_audio_callback(void *userdata, Uint8 *stream, int len)
{
    if(!ready)
    {
        return;
    }
    for(int i = 0; i < len / 4; i++)
    {
        // generate one sample
        SPU->UpdateFromCDC(768);
        if(accum >= 735.735)
        {
         SsSeqCalledTbyT();
         accum -= 735.735;
        }
        accum += 1;
    }
#if 0
    int i = 0;
    while (i * 4 < len) {
        // emit until we have to generate again
        int32_t samples[2];

        int32_t to_mix_l = stream[i * 4 + 1] << 8 | stream[i * 4 + 0];
        int32_t to_mix_r = stream[i * 4 + 3] << 8 | stream[i * 4 + 2];

        to_mix_l += rand();//IntermediateBuffer[i / 4][0];
        to_mix_r += IntermediateBuffer[i / 4][1];

        // stream[i * 4 + 1] = samples[0] >> 8;
        // stream[i * 4 + 0] = samples[0];

        // // right
        // stream[i * 4 + 3] = samples[1] >> 8;
        // stream[i * 4 + 2] = samples[1];
        i += 1;
    }
#endif
    // printf("%d %d\n", IntermediateBufferPos, len);
    memcpy(stream, IntermediateBuffer, len);

    if (IntermediateBufferPos >= 1024)
    {
        IntermediateBufferPos = 0;
    }

//    IntermediateBuffer, IntermediateBufferPos
}
#endif
       #include <unistd.h>


    // Sound gSound;
    // 

extern "C"
void InitSoundRev()
{
    MDFN_IEN_PSX::DMA_Init();
    MDFN_IEN_PSX::DMA_Power();
    SPU = new PS_SPU();
    SPU->Power();
    ready = true;

    gSound.Init();
    SsSetTickMode(SS_NOTICK);
}

extern "C" char aPbav[];
extern "C" char D_8013B6A0[];

extern "C"
void PlaySoundRev()
{
    SeqChunkParser gChunkParser;

    if (!gSound.LoadVab(aPbav, D_8013B6A0))
    {
        printf("Vab load failure\n");
    }

    extern u8 lib_seq[];

    if (!gSound.PlaySEQ(lib_seq))
    {
        printf("PlaySEQ failure\n");
    }

    SsSetTickMode(SS_NOTICK);
}

#if 0

int main(int argc, char* argv[])
{
#ifndef PSX
    if(argc > 1)
    {
        std::string arg = argv[1];
        if(arg == "test")
        {
            run_unit_tests();
        }
 
    }
    SDL_SetMainReady();
    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec wav_spec = {};
    wav_spec.channels = 2;
    wav_spec.samples = 2048;
    wav_spec.format = AUDIO_S16;
    wav_spec.callback = my_audio_callback;
    wav_spec.userdata = NULL;
    if (SDL_OpenAudio(&wav_spec, NULL) < 0) 
    {
        printf(SDL_GetError());
        return 1;
    }
    SDL_PauseAudio(0);

    MDFN_IEN_PSX::DMA_Init();

    MDFN_IEN_PSX::DMA_Power();


    SPU = new PS_SPU();

    SPU->Power();
    ready = true;
    /*
    for (int i = 0; i < 900; i++)
    {
        SPU->UpdateFromCDC(400);
        MDFN_IEN_PSX::DMA_Update(0);

    }*/
#else

#ifdef UNIT_TESTS
    run_unit_tests();
#endif

Sound gSound;
SeqChunkParser gChunkParser;

#endif

    gSound.Init();

    if (!gChunkParser.Parse(MISEQ_BSQ))
    {
        printf("Chunk parse failure\n");
    }
    
    if (!gSound.LoadVab(MINES_VH, MINES_VB))
    {
        printf("Vab load failure\n");
    }

    if (!gSound.PlaySEQ(gChunkParser.SeqPtr(MISEQ_BSQ, 13) + sizeof(ChunkHeader)))
    {
        printf("PlaySEQ failure\n");
    }

    unsigned int clocks = 0;
    printf("Enter loop\n");
    bool quit = false;
    int ts = 0;
    SsSetTickMode(SS_NOTICK);
    for (;quit != true;)
    {
        // 563969
// #ifndef PSX
//         clocks = SPU->UpdateFromCDC(768);
//         // clocks = MDFN_IEN_PSX::DMA_Update(ts);
//         // ts += 768;
// #endif
//         VSync(0);
//         usleep((1001/60000)*1000000);
// #ifndef PSX
//         SsSeqCalledTbyT();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
            quit = true;
                break;
             case SDL_KEYDOWN:
             quit = true;
                break;
            }
        }
        // #endif

    }

   //  audio_batch_cb((int16_t*)&IntermediateBuffer,IntermediateBufferPos);
    // 

    return 0;
}



#endif
extern "C"
void LoadVab(char*vh, char*vb)
{
    gSound.LoadVab(vh, vb) ;
}
