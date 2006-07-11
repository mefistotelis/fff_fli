/*
    FLI Filetype functions
    (c) 2004 Tomasz Lis
*/
#if !defined( __FLIFile_Cpp )
#define __FLIFile_Cpp

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <io.h>
#ifdef __BORLANDC__
#include <values.h>
#else
#include <limits.h>
#define MAXLONG LONG_MAX
#endif
#include "FLIFile.h"


//SaveBlockToFile("!debug.dta",ChunkPacketHdr,DataSize-ChunkPos);
//printf("Saved. Press N key \n");getch();


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

void DisplayMainHeaderInfo(FLIMainHeader *AnimHeader,ulong HStart, ulong FSize)
{
  printf("-------Main header starts on byte %lu ---------\n",HStart);
  printf("Filesize from header: %7lu    Real filesize:%lu\n",AnimHeader->size,FSize);
  printf("Magic number: %15u    Shall be:%u or %u\n",AnimHeader->magic,int(0x0af11),int(0x0af12));
  printf("Frames count: %15u    Value should be positive & big\n",AnimHeader->frames);
  printf("Width: %22u    Value should be positive & <1000\n",AnimHeader->width);
  printf("Height: %21u    Value should be positive & <800\n",AnimHeader->height);
}

void DisplayAddHeaderInfo(FLIAddHeader *AnimAddHeader,ulong HEnd)
{
  printf("------- Main Header - Additional informations ---------\n");
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
  printf("  Bytes in frame: %11lu    Shall be <chunks*65536\n",CurrFrameHdr->size);
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
  LoadDataFromFile(AnimFile,AnimHeader,sizeof(FLIMainHeader),errMainHdr|errCritical,Options);
  if (HasAddHeader(AnimHeader))
    {
    LoadDataFromFile(AnimFile,AnimAddHeader,sizeof(FLIAddHeader),errMainHdr,Options);
    }
   else
    {
    FLIAddHdrDefaults(AnimAddHeader);
//    if ((Options & poDisplayAllInfo))
      printf("-->Additional header doesn't exist - FIXED\n");
    };
}

void LoadFrameHeader(int AnimFile,FLIFrameHeader *AnimFrameHdr,ulong StartOffset,ulong CurrFrame,ulong TotalFrames,int Options)
{
  ClearFLIFrameHdr(AnimFrameHdr);
  lseek(AnimFile, StartOffset, SEEK_SET);
  LoadDataFromFile(AnimFile,AnimFrameHdr,sizeof(FLIFrameHeader),errFrameHdr|errCritical,Options);
  if (Options & poDisplayAllInfo) DisplayFrameInfo(AnimFrameHdr,CurrFrame,TotalFrames,StartOffset,tell(AnimFile)-1);
   else
    if (!(Options & poIgnoreExceptions)) printf("  Restoring frame no %8lu\n",CurrFrame);
}

void LoadChunkHeader(int AnimFile,FLIChunkHeader *AnimChunkHdr,ulong StartOffset,ulong ChunkNumber,int Options)
{
  ClearFLIChunkHdr(AnimChunkHdr);
  lseek(AnimFile, StartOffset, SEEK_SET);
  LoadDataFromFile(AnimFile,AnimChunkHdr,sizeof(FLIChunkHeader),errChunkHdr,Options);
  if (Options & poDisplayAllInfo) DisplayChunkInfo(AnimChunkHdr,ChunkNumber,StartOffset);
}

void LoadChunkData(int AnimFile,void *AnimChunkData,ulong Size,int Options)
{
  char *Buf=(char *)AnimChunkData;
  ClearNBufferBytes(Buf,Size);
  LoadDataFromFile(AnimFile,AnimChunkData,Size,errChunkData,Options);
}

/*
===========================================================
Fixing structures
===========================================================
*/

int ValidChunk(FLIChunkHeader *AnimChunkHdr,ulong MaxChunkSize)
{
  if (AnimChunkHdr==NULL) return 0;

  //Najpierw kontrolujemy rozmiar chunk¢w
  if ((AnimChunkHdr->size)>(65536)) return 0;
  if ( (AnimChunkHdr->type==FLI_COLOR256) || (AnimChunkHdr->type==FLI_COLOR) )
      if ((AnimChunkHdr->size) > MaxPalSize) return 0;
  if ((AnimChunkHdr->size)>(MaxChunkSize)) return 0;

  //No i pozostaˆ tylko typ...
    if ((AnimChunkHdr->size)>(sizeof(FLIChunkHeader)-1))
    {
    if ( (AnimChunkHdr->type==FLI_COLOR256) ||
         (AnimChunkHdr->type==FLI_SS2)      ||
         (AnimChunkHdr->type==FLI_COLOR)    ||
         (AnimChunkHdr->type==FLI_LC)       ||
         (AnimChunkHdr->type==FLI_BLACK)    ||
         (AnimChunkHdr->type==FLI_BRUN)     ||
         (AnimChunkHdr->type==FLI_COPY)     ||
         (AnimChunkHdr->type==FLI_PSTAMP) )
    return 1;
    };
  return 0;
}

int ValidColor(void *ChunkData,ulong DataSize)
{
  if (DataSize<sizeof(FLIColorHeader)) return 0;
  ulong nAllColors=0;
  ulong nChangedColors=0;
  ulong ChunkPos=sizeof(FLIColorHeader);
  FLIColorHeader *ChunkDataHdr=(FLIColorHeader *)(ChunkData);
  FLIColorPacketHeader *ChunkPacketHdr;
  ulong nColorsInPack=0;

  //Przegl¥damy pakiety
  for (ulong i=0;i<ChunkDataHdr->nPackets;i++)
    {
    if (DataSize<ChunkPos)
      {
      return 0;
      };
    //Tworzymy wska«nik na nagˆ¢wek pakietu
    ChunkPacketHdr=(FLIColorPacketHeader *)&(((char *)ChunkData)[ChunkPos]);
    //Teraz czytamy z nagˆ¢wka ilo˜† kolor¢w
    if (ChunkPacketHdr->nChangeColors>0)
      nColorsInPack=ChunkPacketHdr->nChangeColors;
     else
      nColorsInPack=256;
    nAllColors+=ChunkPacketHdr->nSkipColors+nColorsInPack;
    nChangedColors+=nColorsInPack;
    if (nAllColors>256)
      {
      return 0;
      };
    ChunkPos=sizeof(FLIColorHeader)+(i+1)*sizeof(FLIColorPacketHeader)+nChangedColors*sizeof(ColorDefinition);
    }; //end for
  if ((nAllColors>0)&&(ChunkPos<=DataSize)) return 1;
  return 0;
}

int ValidChunkData(unsigned int type,void *ChunkData,ulong DataSize)
{
  if (ChunkData==NULL) return 0;
  switch (type)
    {
    case FLI_COLOR256:return ValidColor(ChunkData,DataSize);
    case      FLI_SS2:return 1;
    case    FLI_COLOR:return 1;
    case       FLI_LC:return 1;
    case    FLI_BLACK:return 1;
    case     FLI_BRUN:return 1;
    case     FLI_COPY:return 1;
    case   FLI_PSTAMP:return 1;
    };
  return 0;
}

int StrictValidFrame(FLIFrameHeader *AnimFrameHdr,ulong MaxLength)
{
  if (AnimFrameHdr==NULL) return 0;
  if (AnimFrameHdr->chunks > 3) return 0;
  for (ulong i=0;i<FLIFrameExpandSize;i++)
     {
     if (AnimFrameHdr->expand[i]!=0) return 0;
     };
  if ((AnimFrameHdr->size>(1229600))||(AnimFrameHdr->size>MaxLength)) return 0;
  if ((AnimFrameHdr->magic==0xf1fa))
    return 1;
  return 0;
}

int ValidFrame(FLIFrameHeader *AnimFrameHdr,ulong MaxLength)
{
  if (AnimFrameHdr==NULL) return 0;
  if (AnimFrameHdr->chunks > 64) return 0;
  if (AnimFrameHdr->size>2*MaxLength) return 0;
  if (AnimFrameHdr->size>(AnimFrameHdr->chunks*65536)) return 0;
  if ((AnimFrameHdr->magic==0xf1fa)&&(AnimFrameHdr->chunks<7))
    return 1;
  return 0;
}

int PalShallBeMultiplied(void *PalData,ulong DataSize)
{
  int Result=1;
  if (!ValidColor(PalData,DataSize)) return 0;

  ulong nChangedColors=0;
  ulong curChangedColors=0;
  ulong ChunkPos=sizeof(FLIColorHeader);
  FLIColorHeader *ChunkDataHdr=(FLIColorHeader *)PalData;
  FLIColorPacketHeader *ChunkPacketHdr;
  ColorDefinition *ColorData;
  for (ulong i=0;i<ChunkDataHdr->nPackets;i++)
    {
    if (DataSize<ChunkPos) return 0;
    //Na pocz¥tek ustalmy poˆorzenia wska«nik¢w
    ChunkPacketHdr=(FLIColorPacketHeader *)&(((char *)PalData)[ChunkPos]);
    ColorData=(ColorDefinition *)&(((char *)PalData)[ChunkPos+sizeof(FLIColorPacketHeader)]);
    if (ChunkPacketHdr->nChangeColors>0)
      curChangedColors=ChunkPacketHdr->nChangeColors;
     else
      curChangedColors=256;
    for (ulong k=0;k<curChangedColors;k++)
      {
      if (ColorData[k].Red>64) Result=0;
      if (ColorData[k].Green>64) Result=0;
      if (ColorData[k].Blue>64) Result=0;
      nChangedColors++;
      };
    ChunkPos=sizeof(FLIColorHeader)+(i+1)*sizeof(FLIColorPacketHeader)+nChangedColors*sizeof(ColorDefinition);
    };
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
  if (!(Options & poFixFrameHeaders)) return;
  //Rozmiar
  if ((AnimFrameHdr->size>1229600) || (AnimFrameHdr->size<sizeof(FLIFrameHeader)))
    {
    AnimFrameHdr->size=FrameEnd-FramePos;
    nFixes++;
    };
  if ((AnimFrameHdr->size>1229600))
    {
    AnimFrameHdr->size=65535;
    nFixes++;
    };
  //Teraz trzeba zrobi† ilo˜† chunks¢w - jeszcze wr¢cimy do rozmiaru
  if (AnimFrameHdr->chunks>128)
    {
    AnimFrameHdr->chunks=3;
    nFixes++;
    };
  //No i znowu rozmiar
  if (AnimFrameHdr->size<sizeof(FLIFrameHeader))
    {
    if (AnimFrameHdr->chunks==0) AnimFrameHdr->size=sizeof(FLIFrameHeader);
     else AnimFrameHdr->size=sizeof(FLIFrameHeader)+1024;
    nFixes++;
    };
  //Parametry dodatkowe
  int ExpandTmp=0;
  for (ulong i=0;i<FLIFrameExpandSize;i++)
     {
     if (AnimFrameHdr->expand[i]!=0)
       {
       AnimFrameHdr->expand[i]=0;
       ExpandTmp++;
       };
     };
  if (ExpandTmp) nFixes++;
  //I zostaˆ tylko Magic
  if ((AnimFrameHdr->magic!=0xf1fa)&&(nFixes>0))
    {
    AnimFrameHdr->magic=0xf1fa;
    nFixes++;
    };
  //Jak jest du¾o bˆ©d¢w, to lepiej nie szuka† zawarto˜ci
  if ((nFixes>2)||(Options & poRadicalFrameHdrFix))
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
    AnimChunkHdr->size=sizeof(FLIChunkHeader);
    nFixes++;
    };
  if (AnimChunkHdr->size>65535)
    {
    AnimChunkHdr->size=65535;
    nFixes++;
    };
  //Typ
  if ((AnimChunkHdr->type>18)||(AnimChunkHdr->type<1))
    {
    if ((AnimChunkHdr->type<1)&&(AnimChunkHdr->size==sizeof(FLIChunkHeader)))
      AnimChunkHdr->type=FLI_BLACK;
     else
     AnimChunkHdr->type=FLI_SS2;
    nFixes++;
    };
  if ((nFixes>0)&&(Options & poDisplayAllInfo))
    printf("-->Chunk header info incorrect - FIXED\n");
}

void FixChunkData(FLIChunkHeader *AnimChunkHdr,void *&ChunkData,ulong &DataSize,int Options)
{
  //Naprawiamy palety
  if ((AnimChunkHdr->type==FLI_COLOR256)||(AnimChunkHdr->type==FLI_COLOR))
    {
  //Czy to jest paleta do zmiany typu ?
    if ((Options & poExpandPatette)&&(AnimChunkHdr->type==FLI_COLOR256))
      AnimChunkHdr->type=FLI_COLOR;


    };
/*
!!!!

*/
}

void FixFrameHdrToAddChunks(FLIFrameHeader *AnimFrameHdr,ulong FrameNum,int Options)
{
  if ((Options & poRecostructPatette)&&(FrameNum==1))
    {
    AnimFrameHdr->size+=256*3+sizeof(FLIChunkHeader)+4;
    AnimFrameHdr->chunks++;
    };
}

/*
===========================================================
Writing structures
===========================================================
*/

void RewriteChunkHeader(int DestFile,ulong DestLastChunkStart,FLIChunkHeader *ChunkHdr)
{
  ulong LastFilePos=tell(DestFile);
  lseek(DestFile,DestLastChunkStart ,SEEK_SET);
  ChunkHdr->size=LastFilePos-DestLastChunkStart;
  SaveDataToFile(ChunkHdr,sizeof(FLIChunkHeader),DestFile);
  lseek(DestFile,LastFilePos,SEEK_SET);
}

void RewriteFrameHeader(int DestFile,ulong DestLastFramePos,ulong FrameNum,FLIFrameHeader *FrameHdr,int Options)
{
  ulong LastFilePos=tell(DestFile);
  lseek(DestFile,DestLastFramePos ,SEEK_SET);
  FixFrameHdrToAddChunks(FrameHdr,FrameNum,Options);
  FrameHdr->size=LastFilePos-DestLastFramePos;
  SaveDataToFile(FrameHdr,sizeof(FLIFrameHeader),DestFile);
  lseek(DestFile,LastFilePos,SEEK_SET);
}

void RewriteMainHeader(int DestFile,ulong nFrames,FLIMainHeader *MainHdr)
{
  ulong LastFilePos=tell(DestFile);
  lseek(DestFile,0,SEEK_SET);
  MainHdr->size=filelength(DestFile);
  MainHdr->frames=nFrames;
  SaveDataToFile(MainHdr,sizeof(FLIMainHeader),DestFile);
  lseek(DestFile,LastFilePos,SEEK_SET);
}

void AddRequiredChunksAtFrameStart(int DestFile,ulong FrameNumber,int Options)
{
//Ta funkcja tylko zapisuje chunksa. Modyfikacja nagˆ¢wka nale¾y do FixFrameHdrToAddChunks
  if ((Options & poRecostructPatette)&&(FrameNumber==1))
    {
    //Deklarujemy nagˆ¢wki
    ulong PalDataSize=256*sizeof(ColorDefinition);
    FLIChunkHeader *CurrChkHdr=(FLIChunkHeader *)AllocateMem(sizeof(FLIChunkHeader)+1,errChunkHdr,Options);
    FLIColorHeader *CurrPalChkHdr=(FLIColorHeader *)AllocateMem(sizeof(FLIColorHeader)+1,errChunkData,Options);
    FLIColorPacketHeader *CurrPcktChkHdr=(FLIColorPacketHeader *)AllocateMem(sizeof(FLIColorPacketHeader)+1,errChunkData,Options);
    //Wstepnie czy˜cimy ich zawarto˜†
    ClearFLIChunkHdr(CurrChkHdr);
    ClearNBufferBytes((char *)CurrPalChkHdr,sizeof(FLIColorHeader));
    ClearNBufferBytes((char *)CurrPcktChkHdr,sizeof(FLIColorPacketHeader));
    //Teraz ustawiamy zawarto˜†: 1 pakiet, wszystkie kolory w pakiecie
    CurrChkHdr->size=PalDataSize+sizeof(FLIChunkHeader)+sizeof(FLIColorHeader)+sizeof(FLIColorPacketHeader);
    CurrChkHdr->type=FLI_COLOR256;
    CurrPalChkHdr->nPackets=1;
    CurrPcktChkHdr->nSkipColors=0;
    CurrPcktChkHdr->nChangeColors=0;//to znaczy 256
    //Pozostaje wszystko zapisa†
    SaveDataToFile(CurrChkHdr,sizeof(FLIChunkHeader),DestFile);
    free(CurrChkHdr);
    SaveDataToFile(CurrPalChkHdr,sizeof(FLIColorHeader),DestFile);
    free(CurrPalChkHdr);
    SaveDataToFile(CurrPcktChkHdr,sizeof(FLIColorPacketHeader),DestFile);
    free(CurrPcktChkHdr);
    //Oddzielnie radzimy sobie z zawartosci¥ palety
    void *CurrChkData=AllocateMem(PalDataSize+1,errChunkData,Options);
    LoadPalette("FLIFix.pal",CurrChkData,PalDataSize,Options);
    SaveDataToFile(CurrChkData,PalDataSize,DestFile);
    free(CurrChkData);
    };
}


#endif	// __FLIFile_Cpp
