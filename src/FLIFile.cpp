/*
    FLI Filetype functions
    (c) 2004 Tomasz Lis
*/
#if !defined( __FLIFile_Cpp )
#define __FLIFile_Cpp

#include <stdio.h>
#include <conio.h>
#include <io.h>
#include "FLIFile.h"


/*
===========================================================
Displaying FLI file information
===========================================================
*/

char *GetChunkTypeStr(unsigned int iType)
{
  switch (iType)
    {
    case FLI_COLOR256:return "FLI_COLOR256 (256-level color palette info)";
    case      FLI_SS2:return "FLI_SS2 (Word-oriented delta compression)";
    case    FLI_COLOR:return "FLI_COLOR (64-level color palette info)";
    case       FLI_LC:return "FLI_LC (Byte-oriented delta compression)";
    case    FLI_BLACK:return "FLI_BLACK (Set whole screen to color 0)";
    case     FLI_BRUN:return "FLI_BRUN (Bytewise run-length compression)";
    case     FLI_COPY:return "FLI_COPY (Indicates uncompressed full frame)";
    case   FLI_PSTAMP:return "FLI_PSTAMP (Postage stamp sized image)";
    default:          return "UNKNOWN (Not a standard FLI chunk type)";
    };
}

void DisplayHeaderInfo(FLIMainHeader *AnimHeader,FLIAddHeader *AnimAddHeader,ulong HStart,ulong HEnd, ulong FSize)
{
  printf("-------Main header starts on byte %lu ---------\n",HStart);
  printf("Filesize from header: %7lu    Real filesize:%lu\n",AnimHeader->size,FSize);
  printf("Magic number: %15u    Shall be:%u\n",AnimHeader->magic,int(0x0af11));
  printf("Frames count: %15u    Value should be positive & big\n",AnimHeader->frames);
  printf("Width: %22u    Value should be positive & <1000\n",AnimHeader->width);
  printf("Height: %21u    Value should be positive & <800\n",AnimHeader->height);
  printf("Color depth: %16u    Should be set to:8\n",AnimAddHeader->depth);
  printf("Flags: %22u    Usually: 3\n",AnimAddHeader->flags);
  printf("Frame speed: %16u    In sec/1024;no estimations\n",AnimAddHeader->speed);
  printf("Unknown value: %14lu    Usually 0;no estimations\n",AnimAddHeader->next);
  printf("Date/Time of creation:%7lu    No estimations\n",AnimAddHeader->frit);
  printf("Creator: %20lu    No estimations\n",AnimAddHeader->creator);
  printf("Date/Time of last change:%4lu    No estimations\n",AnimAddHeader->lastchange);
  printf("Serial number: %14lu    No estimations\n",AnimAddHeader->changerserial);
  printf("X-Aspect ratio: %13u    Can be 0;no estimations\n",AnimAddHeader->Xaspec);
  printf("Y-Aspect ratio: %13u    Can be 0;no estimations\n",AnimAddHeader->Yaspec);
  printf("Reserved(1st):");
  for (ulong n=0;n<(sizeof(AnimAddHeader->reserved1)/sizeof(unsigned int));n++)
     printf("%u ",AnimAddHeader->reserved1[n]);
     printf("(no estims)\n");
  printf("Offset of frame 1: %10lu    (no estimations)\n",AnimAddHeader->frame1);
  printf("Offset of frame 2: %10lu    (no estimations)\n",AnimAddHeader->frame2);
  printf("Reserved(2nd):");
  for (n=0;n<(sizeof(AnimAddHeader->reserved2)/sizeof(unsigned int));n++)
     printf("%u ",AnimAddHeader->reserved2[n]);
     printf("(no estims)\n");
  printf("-------Main header ends on byte %lu ---------\n",HEnd);
}

void DisplayFrameInfo(FLIFrameHeader *CurrFrameHdr,ulong FrameNumber,ulong TotalFrames,ulong HStart,ulong HEnd)
{
  printf("  -------Frame no %lu of %lu header starts on byte %lu ---------\n",FrameNumber,TotalFrames,HStart);
  printf("  Bytes in frame: %11lu    Shall be <65536\n",CurrFrameHdr->size);
  printf("  Magic: %20u    Always %u\n",CurrFrameHdr->magic,0xf1fa);
  printf("  Chunks in frame:%11u    No estimations\n",CurrFrameHdr->chunks);
  printf("  Expand:");
  for (ulong n=0;n<(sizeof(CurrFrameHdr->expand)/sizeof(unsigned int));n++)
     printf("%u ",CurrFrameHdr->expand[n]);
     printf("  (no estims)\n");
  printf("  -------Frame %lu header ends on byte %lu ---------\n",FrameNumber,HEnd);
}

void DisplayChunkInfo(FLIChunkHeader *CurrChunkHdr,ulong ChunkNumber,ulong HStart)
{
  printf("    ==>Chunk no %lu, starts on byte %lu\n",ChunkNumber,HStart);
  printf("    Bytes in chunk:%10lu    ( <65536 )\n",CurrChunkHdr->size);
  printf("    Type of chunk:%11u  %s\n",CurrChunkHdr->type,GetChunkTypeStr(CurrChunkHdr->type));
}

/*
===========================================================
Clearing structures
===========================================================
*/

void ClearNBufferBytes(char *Buf,ulong N)
{
  if (Buf==NULL) return;
  for (ulong i=0;i<N;i++)
    Buf[i]=0;
}

void ClearFLIChunkHdr(FLIChunkHeader *HdrPtr)
{
  char *Buf=(char *)HdrPtr;
  ClearNBufferBytes(Buf,sizeof(FLIChunkHeader));
}

void ClearFLIFrameHdr(FLIFrameHeader *HdrPtr)
{
  char *Buf=(char *)HdrPtr;
  ClearNBufferBytes(Buf,sizeof(FLIFrameHeader));
}

void ClearFLIAddHdr(FLIAddHeader *HdrPtr)
{
  char *Buf=(char *)HdrPtr;
  ClearNBufferBytes(Buf,sizeof(FLIAddHeader));
}

void FLIAddHdrDefaults(FLIAddHeader *HdrPtr)
{
  HdrPtr->depth=8;
  HdrPtr->flags=3;
  HdrPtr->speed=5;
  HdrPtr->next=0;
  HdrPtr->frit=0;
  HdrPtr->creator=0;
  HdrPtr->lastchange=0;
  HdrPtr->changerserial=0;
  HdrPtr->Xaspec=1;
  HdrPtr->Yaspec=1;
  for (ulong i=0;i<19;i++)
    HdrPtr->reserved1[i]=0;
  HdrPtr->frame1=0;
  HdrPtr->frame2=0;
  for (i=0;i<20;i++)
    HdrPtr->reserved2[i]=0;
}

/*
===========================================================
Reading structures
===========================================================
*/

int HasAddHeader(FLIMainHeader *AnimHeader)
{
  if ((AnimHeader->magic==44818)&&(AnimHeader->size==12))
    return 0;
  return 1;
}

void LoadMainHeader(FLIMainHeader *AnimHeader,FLIAddHeader *AnimAddHeader,int AnimFile,int Options)
{
  if (Options & poManualSeeking)
    lseek(AnimFile,0 ,SEEK_SET);
  unsigned int ReadOK=read(AnimFile,AnimHeader,sizeof(FLIMainHeader));
  if (ReadOK!=sizeof(FLIMainHeader)) ShowError(errFileRead+errMainHdr+errCritical,"I couldn't read enought bytes of file.");
  if (HasAddHeader(AnimHeader))
    {
    ReadOK=read(AnimFile,AnimAddHeader,sizeof(FLIAddHeader));
    if (ReadOK!=sizeof(FLIAddHeader)) ShowError(errFileRead+errMainHdr,"I couldn't read enought bytes for additional header.");
    }
   else
    {
    FLIAddHdrDefaults(AnimAddHeader);
    };
}

void LoadFrameHeader(int AnimFile,FLIFrameHeader *AnimFrameHdr,ulong StartOffset,ulong CurrFrame,ulong TotalFrames,int Options)
{
  ClearFLIFrameHdr(AnimFrameHdr);
  lseek(AnimFile, StartOffset, SEEK_SET);
  unsigned int ReadOK=read(AnimFile,AnimFrameHdr,sizeof(FLIFrameHeader));
  if (Options & poDisplayAllInfo) DisplayFrameInfo(AnimFrameHdr,CurrFrame,TotalFrames,StartOffset,tell(AnimFile)-1);
  if ((ReadOK!=sizeof(FLIFrameHeader)) && !(Options & poIgnoreExceptions))
    ShowError(errFileRead+errFrameHdr+errCritical,"I couldn't read enought bytes for frame header");
}

void LoadChunkHeader(int AnimFile,FLIChunkHeader *AnimChunkHdr,ulong StartOffset,ulong ChunkNumber,int Options)
{
  ClearFLIChunkHdr(AnimChunkHdr);
  lseek(AnimFile, StartOffset, SEEK_SET);
  unsigned int ReadOK=read(AnimFile,AnimChunkHdr,sizeof(FLIChunkHeader));
  if (Options & poDisplayAllInfo) DisplayChunkInfo(AnimChunkHdr,ChunkNumber,StartOffset);
  if ((ReadOK!=sizeof(FLIChunkHeader)) && (!(Options & poIgnoreExceptions)))
    { ShowError(errFileRead+errChunkHdr,"Couldn't read this header."); };
}

void LoadChunkData(int AnimFile,void *AnimChunkData,ulong Size,int Options)
{
  char *Buf=(char *)AnimChunkData;
  for (ulong i=0;i<Size;i++)
    Buf[i]=0;
  unsigned int ReadOK=read(AnimFile,AnimChunkData,Size);
  if ((ReadOK!=Size) && !(Options & poIgnoreExceptions))
    { ShowError(errFileRead+errChunkData,"Couldn't read this data. EOF or read error."); };
}

/*
===========================================================
Fixing structures
===========================================================
*/

int ValidChunk(FLIChunkHeader *AnimChunkHdr)
{
  if (AnimChunkHdr==NULL) return 0;
  if (((AnimChunkHdr->size)<(ulong)(32768))&&((AnimChunkHdr->size)>(sizeof(FLIChunkHeader)-1))&&(AnimChunkHdr->type<128)&&(AnimChunkHdr->type>0)) return 1;
  return 0;
}

int ValidFrame(FLIFrameHeader *AnimFrameHdr)
{
  if (AnimFrameHdr==NULL) return 0;
  if ((AnimFrameHdr->size<65535)&&(AnimFrameHdr->magic==0xf1fa)&&(AnimFrameHdr->chunks<128))
    return 1;
  return 0;
}

int PalShallBeMultiplied(char *PalData)
{
  int Result=1;
  for (ulong i=0;i<(3*256);i++)
    if (PalData[i]>64) Result=0;
  return Result;
}

void FixMainHeader(FLIMainHeader *AnimHeaderSrc,FLIAddHeader *AnimAddHeaderSrc,FLIMainHeader *AnimHeaderDest,FLIAddHeader *AnimAddHeaderDest,ulong FSize,int Options)
{
  //MainHdr
  AnimHeaderDest->size=AnimHeaderSrc->size;
  AnimHeaderDest->magic=AnimHeaderSrc->magic;
  AnimHeaderDest->frames=AnimHeaderSrc->frames;
  AnimHeaderDest->width=AnimHeaderSrc->width;
  AnimHeaderDest->height=AnimHeaderSrc->height;
  //AddHdr
  AnimAddHeaderDest->depth=AnimAddHeaderSrc->depth;
  AnimAddHeaderDest->flags=AnimAddHeaderSrc->flags;
  AnimAddHeaderDest->speed=AnimAddHeaderSrc->speed;
  AnimAddHeaderDest->next=AnimAddHeaderSrc->next;
  AnimAddHeaderDest->frit=AnimAddHeaderSrc->frit;
  AnimAddHeaderDest->creator=AnimAddHeaderSrc->creator;
  AnimAddHeaderDest->lastchange=AnimAddHeaderSrc->lastchange;
  AnimAddHeaderDest->changerserial=AnimAddHeaderSrc->changerserial;
  AnimAddHeaderDest->Xaspec=AnimAddHeaderSrc->Xaspec;
  AnimAddHeaderDest->Yaspec=AnimAddHeaderSrc->Yaspec;
  for (ulong i=0;i<sizeof(AnimAddHeaderDest->reserved1);i++)
    AnimAddHeaderDest->reserved1[i]=AnimAddHeaderSrc->reserved1[i];
  AnimAddHeaderDest->frame1=AnimAddHeaderSrc->frame1;
  AnimAddHeaderDest->frame2=AnimAddHeaderSrc->frame2;
  for (i=0;i<sizeof(AnimAddHeaderDest->reserved2);i++)
    AnimAddHeaderDest->reserved2[i]=AnimAddHeaderSrc->reserved2[i];
  if (Options & poFixMainHeader)
    {
    AnimHeaderDest->size=FSize;
    AnimAddHeaderDest->depth=8;
    };
}

void FixFrameHeader(FLIFrameHeader *AnimFrameHdr,ulong FramePos,ulong FrameEnd,int Options)
{
  ulong nFixes=0;
  if (AnimFrameHdr==NULL) return;
  //Rozmiar
  if ((AnimFrameHdr->size>65535) || (AnimFrameHdr->size<sizeof(FLIFrameHeader)))
    {
    AnimFrameHdr->size=FrameEnd-FramePos;
    nFixes++;
    };
  if (AnimFrameHdr->size>65535)
    {
    AnimFrameHdr->size=65535;
    nFixes++;
    };
  //Teraz trzeba zrobi† ilo˜† chunks¢w - jeszcze wr¢cimy do rozmiaru
  if (AnimFrameHdr->chunks>128)
    {
    AnimFrameHdr->chunks=16;
    nFixes++;
    };
  //No i znowu rozmiar
  if (AnimFrameHdr->size<sizeof(FLIFrameHeader))
    {
    if (AnimFrameHdr->chunks==0) AnimFrameHdr->size=sizeof(FLIFrameHeader);
     else AnimFrameHdr->size=sizeof(FLIFrameHeader)+1024;
    nFixes++;
    };

  if ((AnimFrameHdr->magic!=0xf1fa)&&(nFixes>0))
    {
    AnimFrameHdr->magic=0xf1fa;
    nFixes++;
    };
  if ((nFixes>2)||(Options & poFixFrameHeaders))
    {
    AnimFrameHdr->chunks=0;
    AnimFrameHdr->size=sizeof(FLIFrameHeader);
    };
  if ((nFixes>0)&&(Options & poDisplayAllInfo))
    printf("-->Frame header info incorrect - FIXED\n");
}

void FixChunkHeader(FLIChunkHeader *AnimChunkHdr,ulong MaxSize,int Options)
{
  ulong nFixes=0;
  if (AnimChunkHdr==NULL) return;
  //Rozmiar
  if ((AnimChunkHdr->size>65535))
    {
    AnimChunkHdr->size=MaxSize;
    nFixes++;
    };
  if (AnimChunkHdr->size<sizeof(FLIChunkHeader))
    {
    AnimChunkHdr->size=sizeof(FLIChunkHeader)+4;
    nFixes++;
    };
  if (AnimChunkHdr->size>65535)
    {
    AnimChunkHdr->size=65535;
    nFixes++;
    };
  //Typ
  if ((AnimChunkHdr->type>32)||(AnimChunkHdr->type<1))
    {
    AnimChunkHdr->type=4;
    nFixes++;
    }
  if ((nFixes>0)&&(Options & poDisplayAllInfo))
    printf("-->Chunk header info incorrect - FIXED\n");
}

void FixChunkData(FLIChunkHeader *AnimChunkHdr,char *ChunkData,int Options)
{
  if ((Options & poExpandPatette)&&(AnimChunkHdr->type==FLI_COLOR256))
     {
     for (ulong k=sizeof(FLIColor256Header);k<(AnimChunkHdr->size-sizeof(FLIChunkHeader));k++)
       if (ChunkData[k]<64) ChunkData[k]*=4; else ChunkData[k]=255;
     };
/*
!!!!

*/
}

#endif	// __FLIFile_Cpp
