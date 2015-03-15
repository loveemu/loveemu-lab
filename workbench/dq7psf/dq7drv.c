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
#define PSFDRV_LOAD         (0x800FF000)
#define PSFDRV_SIZE         (0x00000800)
#define PSFDRV_PARAM        (0x800FF800)
#define PSFDRV_PARAM_SIZE   (0x00000100)

/*
** You can also define locations of game-specific data here.
*/
#define MY_SEQ          (0x80138000)
#define MY_SEQ_SIZE     (0x00020000)
#define MY_INSTR        (0x80180000)
#define MY_INSTR_SIZE   (0x00070000)

/*
** Parameters - you can make up any parameters you want within the
** PSFDRV_PARAM block.
*/
#define PARAM_SAMPLE    (*((unsigned char*)(PSFDRV_PARAM+0x0000)))

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
  (int)"Dragon Quest VII psf driver v1.0",
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
  //(int)"SLPM_865.00", 0xBD059C55,
  (int)"SLPM_865.00", 0x01B0F5D2, // use customized PSX-EXE
  /*
  ** Jump patch address
  ** You should change this to point to the address of the "jal main"
  ** instruction in the game's original EXE.
  */
  0x8008DAEC,
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
  #define SetSp                                  F1(0x800A1A24)

  #define SpuInit                                F0(0x80090E54)
  #define minit                                  F0(0x80082480)
  #define mopen_file                             F1(0x80082598)
  #define mopenplay_seq                          F3(0x80034988)
  #define mopenplay_seq_fadein                   F3(0x80035684)
  #define mreopen_seq                            F2(0x80035CD4)
  #define mplay_seq                              F2(0x80031310)

  #define SpuSetReverb(a)                        F1(0x800925B4) ((int)(a))
  #define SpuSetReverbModeParam(a)               F1(0x80092794) ((int)(a))
  #define SpuSetReverbVoice(a,b)                 F2(0x80093194) ((int)(a),(int)(b))

/***************************************************************************/
/*
** PSF driver main() replacement
*/
int psfdrv(void) {
  unsigned long seq_size = *((unsigned long *)MY_SEQ);

  /*
  ** Initialize and startup stuff
  */
  SetSp(0x800C58E4);
  SpuInit();
  minit();

  /*
  ** Load sound file for instrument set
  */
  mopen_file(MY_INSTR);

  /*
  ** Load sound file for sequence
  */
  if (seq_size != 0) {
    /* this call should always execute mopenplay_seq, since I modified mopen_file a lot */
    mopen_file(MY_SEQ);

    mplay_seq(MY_SEQ, 0x1200);
  }
  else {
    die();
  }

  /*
  ** Loop a while.
  */
  loopforever();

  return 0;
}

/***************************************************************************/
