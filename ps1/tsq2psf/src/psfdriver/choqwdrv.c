/***************************************************************************/
/*
** PSF-o-Cycle development system
**
** This is an example which demonstrates the basics of how to make a PSF
** driver stub and illustrates the format of the PSF_DRIVER_INFO block.
** It can be customized to create stubs for actual games (whether they use
** the SEQ/VAB library or not).
*/

/*
** Define the location of the PSF driver stub.
** You should define this to somewhere safe where there's no useful data and
** which will not get overwritten by the BSS clear loop.
*/
#define PSFDRV_LOAD         (0x80010000)
#define PSFDRV_SIZE         (0x00000800)
#define PSFDRV_PARAM        (0x80010800)
#define PSFDRV_PARAM_SIZE   (0x00000100)
#define MINIPSF_PARAM       (0x800EF800)

/*
** You can also define locations of game-specific data here.
*/
#define MY_SEQ          (0x800F0000)
#define MY_SEQ_SIZE     (0x00018000)
#define MY_INSTR        (0x8001D800)
#define MY_INSTR_SIZE   (0x00080000)

/*
** Parameters - you can make up any parameters you want within the
** PSFDRV_PARAM block.
** In this example, I'm including the sequence volume, reverb type and depth.
*/
#define PARAM_RTYPE         (*((unsigned char*)(PSFDRV_PARAM+0x0000)))
#define PARAM_RDEPTH        (*((unsigned char*)(PSFDRV_PARAM+0x0001)))
#define PARAM_MVOL          (*((unsigned short*)(PSFDRV_PARAM+0x0002)))
#define MPARAM_SONGINDEX    (*(unsigned short*)(MINIPSF_PARAM+0x0000))
#define MPARAM_RESERVED     (*(unsigned short*)(MINIPSF_PARAM+0x0002))
#define MPARAM_RUSE_SUB     (*((unsigned char*)(MINIPSF_PARAM+0x0004)))
#define MPARAM_RTYPE_SUB    (*((unsigned char*)(MINIPSF_PARAM+0x0005)))
#define MPARAM_RDEPTH_SUB   (*((unsigned char*)(MINIPSF_PARAM+0x0006)))

/***************************************************************************/
/*
** Entry point
*/
int psfdrv(void);
int psfdrv_entry(void) {
  /*
  ** Read the entire driver area, to ensure it doesn't get thrown out by
  ** PSFLab's optimizer
  */
  int *a = ((int*)(PSFDRV_LOAD));
  int *b = ((int*)(PSFDRV_LOAD+PSFDRV_SIZE));
  int c = 0;
  while(a < b) { c += (*a++); }
  /* This return value is completely ignored. */
  return c + psfdrv();
}

/***************************************************************************/

#define ASCSIG(a,b,c,d) ( \
  ((((unsigned long)(a)) & 0xFF) <<  0) | \
  ((((unsigned long)(b)) & 0xFF) <<  8) | \
  ((((unsigned long)(c)) & 0xFF) << 16) | \
  ((((unsigned long)(d)) & 0xFF) << 24)   \
  )

/***************************************************************************/
/*
** PSF_DRIVER_INFO block.
*/
unsigned long driverinfo[] = {
  /*
  ** Signature
  */
  ASCSIG('P','S','F','_'),
  ASCSIG('D','R','I','V'),
  ASCSIG('E','R','_','I'),
  ASCSIG('N','F','O',':'),
  /*
  ** Driver load address (was #defined earlier)
  */
  PSFDRV_LOAD,
  /*
  ** Driver entry point
  */
  (int)psfdrv_entry,
  /*
  ** Driver text string.  This should include the name of the game.
  */
  (int)"Choro Q Wonderful! psf driver v1.0",
  /*
  ** Original EXE filename and CRC - ignore if zero
  **
  ** You may not want to use the exact original EXE here.  Sometimes you may
  ** want to patch the BSS clearing loop first, to ensure that it doesn't
  ** overwrite your driver stub, SEQ data, or other data that you added after
  ** the fact.  In this case I usually use a different name for the patched
  ** EXE, i.e. "ff8patch.exe" for Final Fantasy 8, and redo the CRC
  ** accordingly.
  */
  (int)"SLPS_022.05", 0x1461CAC2,
  /*
  ** Jump patch address
  ** You should change this to point to the address of the "jal main"
  ** instruction in the game's original EXE.
  */
  0x801B5C4C,
  /*
  ** List of song-specific areas we DO NOT upgrade.
  ** This is a 0-terminated list of addresses and byte lengths.
  ** Mark the areas containing SEQ, VAB, or other song-specific data here.
  ** Marking the psfdrv parameter area here might also be a good idea.
  */
  MY_SEQ, MY_SEQ_SIZE,
  MY_INSTR, MY_INSTR_SIZE,
  PSFDRV_PARAM, PSFDRV_PARAM_SIZE,
  0,
  /*
  ** List of parameters (name,address,bytesize)
  ** This is a 0-terminated list.
  */
  (int)"rtype" , (int)(&PARAM_RTYPE ), 1,
  (int)"rdepth", (int)(&PARAM_RDEPTH), 1,
  (int)"mvol", (int)(&PARAM_MVOL), 2,
  0
};

/***************************************************************************/
/*
** Handy definitions
*/
#define NULL (0)

#define F0(x) (*((func0)(x)))
#define F1(x) (*((func1)(x)))
#define F2(x) (*((func2)(x)))
#define F3(x) (*((func3)(x)))
#define F4(x) (*((func4)(x)))
typedef int (*func0)(void);
typedef int (*func1)(int);
typedef int (*func2)(int,int);
typedef int (*func3)(int,int,int);
typedef int (*func4)(int,int,int,int);

/*
** die() function - emits a break instruction.
** This isn't emulated in Highly Experimental, so it will cause the emulation
** to halt (this is a desired effect).
*/
unsigned long die_data[] = {0x4D};
#define die F0(die_data)

/*
** loopforever() - emits a simple branch and nop.
** Guaranteed to be detected as idle in H.E. no matter what the compiler
** does.
*/
unsigned long loopforever_data[] = {0x1000FFFF,0};
#define loopforever F0(loopforever_data)

#define ASSERT(x) { if(!(x)) { die(); } }

/***************************************************************************/
/*
** Library call addresses.
**
** You'll want to fill in the proper addresses for these based on what you
** found in IDA Pro or similar.
**
** I left some numbers from a previous rip in here just to make the example
** look pretty.  Trust me, you will want to change these.
*/
  #define bzero(a,b)                             F2(0x801B5DC8) ((int)(a),(int)(b))

  #define VSync(a)                               F1(0x801BF538) ((int)(a))
  #define ResetCallback                          F0(0x801BF768)
  #define VSyncCallback(a)                       F1(0x801BF7F8) ((int)(a))

  #define SndVSyncCallbackPtr                    0x80164978
  #define SndInit                                F0(0x8016F238)
  #define SndDequeuePlaybackRequest              F0(0x8016F360)
  #define SndDispatchMessage(a,b)                F2(0x8016F4A8) ((int)(a),(int)(b))
  #define SndPlaySFX(a,b,c)                      F3(0x8016FB84) ((int)(a),(int)(b),(int)(c))
  #define SndPlayBGM(a,b)                        F2(0x8016FC6C) ((int)(a),(int)(b))
  #define SndLoadTSQ(a,b)                        F2(0x801704C4) ((int)(a),(int)(b))
  #define SndLoadTVB(a,b,c)                      F3(0x801705D4) ((int)(a),(int)(b),(int)(c))
  #define SndSetMasterVolume(a)                  F1(0x801706D0) ((int)(a))
  #define SndSetReverbMode(a)                    F1(0x80170740) ((int)(a))
  #define SndSetReverb(a)                        F1(0x80170800) ((int)(a))
  #define SndSetReverbDepth(a)                   F1(0x801707A4) ((int)(a))
  #define SndQueuePlaybackRequest(a)             F1(0x801708AC) ((int)(a))

  #define SPU_ON                                 1
  #define SPU_OFF                                0

/***************************************************************************/
/*
** PSF driver main() replacement
*/
int psfdrv(void) {
  int rtype;
  int rdepth;
  int mvol;

  int ruse_sub;
  int rtype_sub;
  int rdepth_sub;

  void *tsq;
  void *tvb;

  int song;
  int bank;
  int spu_adpcm_addr;

  /*
  ** Retrieve parameters and set useful defaults if they're zero
  */
  rtype  = PARAM_RTYPE;
  rdepth = PARAM_RDEPTH;
  mvol   = PARAM_MVOL;

  /*
  ** Overwrite reverb parameters if requested
  */
  ruse_sub = MPARAM_RUSE_SUB;
  rtype_sub = MPARAM_RTYPE_SUB;
  rdepth_sub = MPARAM_RDEPTH_SUB;
  if (ruse_sub != 0)
  {
    rtype = rtype_sub;
    rdepth = rdepth_sub;
  }

  /*
  ** Retrieve parameters for playback function
  */
  song = MPARAM_SONGINDEX;

  /*
  ** Other constant parameters
  */
  tsq = (void*)(MY_SEQ);
  tvb = (void*)(MY_INSTR);
  bank = 0;
  spu_adpcm_addr = 0x1010;

  /*
  ** Initialize and startup stuff (comes from original main and init routine)
  */
  bzero(0x1F800000, 0x400);
  ResetCallback();
  SndInit();

  /*
  ** Reverb setup
  */
  if (rtype != 0 || rdepth != 0)
  {
    //rtype = 3;
    //rdepth = 64;

    SndSetReverbMode(rtype);
    SndSetReverb(SPU_ON);
    SndSetReverbDepth(rdepth);
  }
  else
  {
    SndSetReverb(SPU_OFF);
  }

  /*
  ** More initialize stuff
  */
  SndDequeuePlaybackRequest();
  VSyncCallback(SndVSyncCallbackPtr);
  VSync(0);

  /*
  ** Load sound files
  */
  SndLoadTSQ(tsq, bank);
  SndLoadTVB(tvb, bank, spu_adpcm_addr);

  //mvol = 0x2000;
  SndSetMasterVolume(mvol);

  /*
  ** Play the song
  */
  SndQueuePlaybackRequest(0x40400000 | (bank << 8) | song);

  /*
  ** Loop a while.
  */
  loopforever();

  return 0;
}

/***************************************************************************/
