/*
    FLI Filetype declaration
    (c) 2004 Tomasz Lis
*/
#if !defined( __FLIFile_H )
#define __FLIFile_H

#include "PrgTools.h"

enum ChunkType
  {
    FLI_COLOR256=   4,
    FLI_SS2=        7,
    FLI_COLOR=     11,
    FLI_LC =       12,
    FLI_BLACK=     13,
    FLI_BRUN =     15,
    FLI_COPY=      16,
    FLI_PSTAMP=    18
  };

typedef struct
  {
     ulong size;
     unsigned int magic;
     unsigned int frames;
     unsigned int width;
     unsigned int  height;
  } FLIMainHeader;

typedef struct
  {
     unsigned int depth;
     unsigned int flags;
     unsigned int speed;
     ulong next;
     ulong frit;
     //Wa¾ne mineˆy - tu s¥ pierdoˆy (offset 26 dec)
     ulong creator;
     ulong lastchange;
     ulong changerserial;
     unsigned int Xaspec;
     unsigned int Yaspec;
     unsigned int reserved1[19];
     ulong frame1;
     ulong frame2;
     unsigned int reserved2[20];
  } FLIAddHeader;

typedef struct
  {
     ulong size;
     unsigned int magic;
     unsigned int chunks;
     unsigned int expand[4];
  } FLIFrameHeader;

typedef struct
  {
     ulong size;
     unsigned int type;
  } FLIChunkHeader;

typedef struct
  {
     unsigned int val1;
     unsigned int val2;
  } FLIColor256Header;

//Funkcje
char *GetChunkTypeStr(int iType);
void DisplayHeaderInfo(FLIMainHeader *AnimHeader,FLIAddHeader *AnimAddHeader,ulong HStart,ulong HEnd, ulong FSize);
void DisplayFrameInfo(FLIFrameHeader *CurrFrameHdr,ulong FrameNumber,ulong TotalFrames,ulong HStart,ulong HEnd);
void DisplayChunkInfo(FLIChunkHeader *CurrChunkHdr,ulong ChunkNumber,ulong HStart);

void ClearFLIChunkHdr(FLIChunkHeader *HdrPtr);
void ClearFLIFrameHdr(FLIFrameHeader *HdrPtr);
void ClearFLIAddHdr(FLIAddHeader *HdrPtr);
void FLIAddHdrDefaults(FLIAddHeader *HdrPtr);

int HasAddHeader(FLIMainHeader *AnimHeader);
void LoadMainHeader(FLIMainHeader *AnimHeader,FLIAddHeader *AnimAddHeader,int AnimFile,int Options);
void LoadFrameHeader(int AnimFile,FLIFrameHeader *AnimFrameHdr,ulong StartOffset,ulong CurrFrame,ulong TotalFrames,int Options);
void LoadChunkHeader(int AnimFile,FLIChunkHeader *AnimChunkHdr,ulong StartOffset,ulong ChunkNumber,int Options);
void LoadChunkData(int AnimFile,void *AnimChunkData,ulong Size,int Options);

int ValidChunk(FLIChunkHeader *AnimChunkHdr);
int ValidFrame(FLIFrameHeader *AnimFrameHdr);
void FixMainHeader(FLIMainHeader *AnimHeaderSrc,FLIAddHeader *AnimAddHeaderSrc,FLIMainHeader *AnimHeaderDest,FLIAddHeader *AnimAddHeaderDest,ulong FSize,int Options);
void FixFrameHeader(FLIFrameHeader *AnimFrameHdr,ulong FramePos,ulong FrameEnd,int Options);

#endif	// __FLIFile_H
