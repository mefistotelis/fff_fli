#include "FLIFile.h"
#include "PrgTools.h"
#include <io.h>
#include <stdio.h>
#include <conio.h>
#include <FCNTL.H>
#include <SYS\STAT.H>
#include <mem.h>

#include "PrgTools.cpp"
#include "FLIFile.cpp"

int Options;
char *DestFName="repaired.fli";

void SayHello()
{
  printf("This program can repair damaged or modified FLI files.\n");
  printf("Repaired file is saved as '%s'\n",DestFName);
}

void OpenFLIFile(char *FName,int &File,int Mode)
{
  File=open(FName,Mode|O_BINARY,S_IREAD|S_IWRITE);
  if (File<1) ShowError(errFileOpen+errCritical,strerror(errno));
}

void WaitForKeypress(int &Options)
{
  if (Options & poDisplayAllInfo)
    {
    printf("\n (Press any key to continue, Q quits, ESC stops holding)\n");
    char X=getch();
    if (X==kbEscape) Options&=(0xffff-poDisplayAllInfo);
    if ((X=='q')||(X=='Q')) exit(EXIT_FAILURE);
    };
}

void CloseFLIFiles(int &AnimFile,int &DestFile)
{
  printf("-------- Analyst complete - all frames has been processed ---------\n");
  close(AnimFile);
  close(DestFile);
  printf("\n (Press any key to leave program)");
  getch();
  printf("\n");
}

void AskToDisplayInfo(int &Options)
{
  printf(" (Press ENTER to display additional informations)\n");
  printf(" (Other key cancels full displaying)\n");
  char X=getch();
  if (X==kbEnter) Options|=poDisplayAllInfo; else Options&=(0xffff-poDisplayAllInfo);
}

void SaveData(void *BufDest,ulong Size,int DestFile)
{
  ulong nWritten=write(DestFile,BufDest,Size);
  if (nWritten<Size) ShowError(errFileWrite+errCritical,strerror(errno));
}

int ProcessMainHeader(int AnimFile,int DestFile,ulong &FrameCount,int Options)
{
  void *BufSrc=NULL;
  void *BufDest=NULL;
  ulong Increment=sizeof(FLIMainHeader);
  BufSrc=malloc(sizeof(FLIMainHeader)+sizeof(FLIAddHeader)+2);
  BufDest=malloc(sizeof(FLIMainHeader)+sizeof(FLIAddHeader)+2);
  if ((BufSrc==NULL)||(BufDest==NULL)) ShowError(errCritical|errMainHdr|errMemAlloc,"Cannot allocate memory for main header. Try reboot your machine.");
  FLIMainHeader *AnimHeaderSrc=(FLIMainHeader *)BufSrc;
  FLIAddHeader *AnimAddHeaderSrc=(FLIAddHeader *)((ulong)(BufSrc)+sizeof(FLIMainHeader));
  FLIMainHeader *AnimHeaderDest=(FLIMainHeader *)BufDest;
  FLIAddHeader *AnimAddHeaderDest=(FLIAddHeader *)((ulong)(BufDest)+sizeof(FLIMainHeader));
  ulong StartPos=tell(AnimFile);
  //Deklarujemy, czytamy i wyswietlamy nagˆ¢wek
  LoadMainHeader(AnimHeaderSrc,AnimAddHeaderSrc,AnimFile,Options);
  FixMainHeader(AnimHeaderSrc,AnimAddHeaderSrc,AnimHeaderDest,AnimAddHeaderDest,filelength(AnimFile),Options);
  DisplayHeaderInfo(AnimHeaderSrc,AnimAddHeaderSrc,StartPos,tell(AnimFile)-1,filelength(AnimFile));
  if (HasAddHeader(AnimHeaderSrc))
    Increment+=sizeof(FLIAddHeader);
  SaveData(BufDest,sizeof(FLIMainHeader)+sizeof(FLIAddHeader),DestFile);
  FrameCount=AnimHeaderDest->frames;
  free(BufSrc);
  free(BufDest);
  return Increment;
}

void FixFrameHdrToAddChunks(FLIFrameHeader *AnimFrameHdr,ulong FrameNum,int Options)
{
  if ((Options & poRecostructPatette)&&(FrameNum==1))
    {
    AnimFrameHdr->size+=256*3+sizeof(FLIChunkHeader)+4;
    AnimFrameHdr->chunks++;
    };
}

void AddRequiredChunksAtFrameStart(int DestFile,ulong FrameNumber,int Options)
{
//Ta funkcja tylko zapisuje chunksa. Modyfikacja nagˆ¢wka nale¾y do FixFrameHdrToAddChunks
  if ((Options & poRecostructPatette)&&(FrameNumber==1))
    {
    ulong PalSize=3*256;
    FLIChunkHeader *PalChkHdr=(FLIChunkHeader *)malloc(sizeof(FLIChunkHeader)+1);
    FLIColor256Header *PalPalChkHdr=(FLIColor256Header *)malloc(sizeof(FLIColor256Header)+1);
    void *PalChkData=malloc(PalSize+1);
    if ((PalChkHdr==NULL)||(PalPalChkHdr==NULL)||(PalChkData==NULL))
      {
      ShowError(errChunkData|errMemAlloc,"Cannot allocate memory for additional palette chunk. Output file will have errors.");
      return;
      };
    ClearFLIChunkHdr(PalChkHdr);
    ClearNBufferBytes((char *)PalPalChkHdr,sizeof(FLIColor256Header));
    PalChkHdr->size=PalSize+sizeof(FLIChunkHeader)+sizeof(FLIColor256Header);
    PalChkHdr->type=FLI_COLOR256;
    PalPalChkHdr->val1=1;

    SaveData(PalChkHdr,sizeof(FLIChunkHeader),DestFile);
    free(PalChkHdr);
    SaveData(PalPalChkHdr,sizeof(FLIColor256Header),DestFile);
    free(PalPalChkHdr);

    LoadPalette("FLIFix.pal",PalChkData,PalSize,Options);
    SaveData(PalChkData,PalSize,DestFile);
    free(PalChkData);
    };
}


ulong ProcessFrame(int AnimFile,int DestFile,FLIFrameHeader *AnimFrameHdr,ulong FramePos,ulong FrameEnd,ulong FrameNum,ulong TotalFrames,int Options)
{
  ulong Increment=sizeof(FLIFrameHeader);
  LoadFrameHeader(AnimFile,AnimFrameHdr,FramePos,FrameNum,TotalFrames,Options);
  FixFrameHeader(AnimFrameHdr,FramePos,FrameEnd,Options);

  //Robimy kopi© nagˆ¢wna pod nazw¥ DestFrameHdr
  FLIFrameHeader *DestFrameHdr=(FLIFrameHeader *)malloc(sizeof(FLIFrameHeader)+1);
  if (DestFrameHdr==NULL) ShowError(errCritical|errFrameHdr|errMemAlloc,"Cannot allocate memory for frame header. Try reboot.");
  memmove(DestFrameHdr,AnimFrameHdr,sizeof(FLIFrameHeader));

  //Zapisujemy t¥ kopi© - po drobnych modyfikacjach
  FixFrameHdrToAddChunks(DestFrameHdr,FrameNum,Options);
  SaveData(DestFrameHdr,sizeof(FLIFrameHeader),DestFile);
  AddRequiredChunksAtFrameStart(DestFile,FrameNum,Options);
  free(DestFrameHdr);
  return Increment;
}

void FillFramePosTable(ulong *FramePosTable,int AnimFile,ulong FrameCount,int Options)
{
  ulong Offset=tell(AnimFile);
  FLIFrameHeader *AnimFrameHdr=(FLIFrameHeader *)malloc(sizeof(FLIFrameHeader)+1);
  if (AnimFrameHdr==NULL) ShowError(errCritical|errFrameHdr|errMemAlloc,"Cannot allocate memory for frame header required to create Frame Position Table. Try reboot.");
  for (ulong n=0;n<=FrameCount;n++)
    {
    //Szukamy offsetu na kt¢rym jest FrameHeader
    signed long delta=0;
    signed long lastDelta;
    do
      {
      lastDelta=delta;
      //delta ma by† ci¥giem: 0 -1 1 -2 2 -3 3 ....
      if (delta>=0)
        { delta++; delta*=(-1); }
       else
        { delta*=(-1); };
      //Dla kolejnych delt pr¢bujemy wczytywa† FrameHeader
      if (((Offset-delta)>(sizeof(FLIMainHeader)-2)) && ((Offset-delta)<(filelength(AnimFile)-sizeof(FLIFrameHeader)+2)))
        LoadFrameHeader(AnimFile,AnimFrameHdr,Offset+lastDelta,n+1,FrameCount,(Options & (0xffff - poDisplayAllInfo)) | poIgnoreExceptions);
       else
        ClearFLIFrameHdr(AnimFrameHdr);
      }
     while ((!ValidFrame(AnimFrameHdr))&&(delta<65535));
    //Tu mamy kandydata na Framea - mo¾emy go zaakceptowa† lub nie
    if (ValidFrame(AnimFrameHdr))
      {
      if ((n>0)&&((Offset+lastDelta)<FramePosTable[n-1]))
        FramePosTable[n]=Offset;
       else
        FramePosTable[n]=Offset+lastDelta;
      }
     else
      FramePosTable[n]=Offset;
    //Ustawili˜my offset Frame'a. Trzeba sie jeszcze przygotowac do kolejnych poszukiwaä
/*
    printf(">FillFramePosTable< Klatka %lu, offset wejsciowy %lu, delta %lu \n",n,Offset,lastDelta);
    getch();
*/
    LoadFrameHeader(AnimFile,AnimFrameHdr,FramePosTable[n],n+1,FrameCount,(Options & (0xffff - poDisplayAllInfo)) | poIgnoreExceptions);
    Offset=FramePosTable[n]+AnimFrameHdr->size;
    };
  FramePosTable[FrameCount+1]=filelength(AnimFile);
  free(AnimFrameHdr);
}

void SetRequiredOptions(ulong *FramePosTable,int AnimFile,ulong FrameCount,int &Options)
{
  if (FrameCount>0)
    {
    FLIChunkHeader *AnimChunkHdr=(FLIChunkHeader *)malloc(sizeof(FLIChunkHeader)+1);
    if (AnimChunkHdr==NULL) return;
    LoadChunkHeader(AnimFile,AnimChunkHdr,FramePosTable[0]+sizeof(FLIFrameHeader),0,(Options & (0xffff - poDisplayAllInfo)) | poIgnoreExceptions);
    if ((AnimChunkHdr->type!=FLI_COLOR)&&(AnimChunkHdr->type!=FLI_COLOR256))
      {
      Options|=poRecostructPatette;
      }
     else
      if (AnimChunkHdr->type==FLI_COLOR256)
        {
        //W takim wypadku trzeba sprawdzi†, czy nie mno¾y† palety *4
        //Tworzymy bufor do wczytania zawarto˜ci chunka
        ulong DataSize=AnimChunkHdr->size-sizeof(FLIChunkHeader);
        void *ChunkData=malloc(DataSize+2);
        if (ChunkData!=NULL)
          {
          //Wczytujemy
          LoadChunkData(AnimFile,ChunkData,DataSize,Options);
	  if (PalShallBeMultiplied((char *)ChunkData+sizeof(FLIColor256Header)))
            Options|=poExpandPatette;
          free(ChunkData);
          };
        };
    free(AnimChunkHdr);
    };
}


/*
void AdjustChunkOffset(ulong ChunkPos,int AnimFile,ulong RangeStart,ulong RangeEnd,int Options)
{
  ulong Offset=tell(AnimFile);
  FLIChunkHeader *AnimChunkHdr=(FLIChunkHeader *)malloc(sizeof(FLIChunkHeader)+1);
  for (ulong n=0;n<=FrameCount;n++)
    {
    //Szukamy offsetu na kt¢rym jest ChunkHeader
    ulong delta=0;
    ulong lastDelta;
    do
      {
      lastDelta=delta;
      //delta ma by† ci¥giem: 0 -1 1 -2 2 -3 3 ....
      if (delta>=0)
	{ delta++; delta*=(-1); }
       else
	{ delta*=(-1); };
      //Dla kolejnych delt pr¢bujemy wczytywa† ChunkHeader
      if (((Offset-delta)>(sizeof(FLIMainHeader)-2)) && ((Offset-delta)<(filelength(AnimFile)-sizeof(FLIMainHeader)+2)))
	LoadChunkHeader(AnimFile,AnimChunkHdr,Offset+lastDelta,Options | poIgnoreExceptions);
       else
	ClearFLIChunkHdr(AnimChunkHdr);
      }
     while ((!ValidChunk(AnimChunkHdr))&&(delta<65535));
    //Tu mamy kandydata na Chunka - mo¾emy go zaakceptowa† lub nie
    if (ValidChunk(AnimChunkHdr))
      {
      if ((n>0)&&((Offset+lastDelta)<FramePosTable[n-1]))
	FramePosTable[n]=Offset;
       else
	FramePosTable[n]=Offset+lastDelta;
      }
     else
      FramePosTable[n]=Offset;
    //Ustawili˜my offset chunka. Trzeba sie przygotowac do kolejnych poszukiwaä
    printf("Klatka numer %lu, offset wejsciowy %lu, delta %lu \n",n,Offset,lastDelta);
    getch();
    LoadChunkHeader(AnimFile,AnimChunkHdr,FramePosTable[n],Options | poIgnoreExceptions);
    Offset=FramePosTable[n]+AnimChunkHdr->size;
    };
  free(AnimChunkHdr);
}
*/

ulong ProcessChunk(int AnimFile,int DestFile,ulong ChunkPos,ulong MaxSize,ulong ChunkNum,int Options)
{
  //Ustawienia i testy pocz¥tkowe
  ulong Increment=sizeof(FLIChunkHeader);

  //Dopasowujeny offset Chunka
  //!!!!AdjustChunkOffset(AnimFile,,,Options);

  //Wczytujemy jego nagˆ¢wek i zapisujemy przy okazji
  FLIChunkHeader *AnimChunkHdr=(FLIChunkHeader *)malloc(sizeof(FLIChunkHeader)+1);
  if (AnimChunkHdr==NULL) ShowError(errCritical|errChunkHdr|errMemAlloc,"Cannot allocate memory for chunk header. Try reboot your machine.");
  LoadChunkHeader(AnimFile,AnimChunkHdr,ChunkPos,ChunkNum,Options);
  FixChunkHeader(AnimChunkHdr,MaxSize,Options);
  SaveData(AnimChunkHdr,sizeof(FLIChunkHeader),DestFile);

  //Tworzymy bufor do wczytania zawarto˜ci
  ulong DataSize=AnimChunkHdr->size-sizeof(FLIChunkHeader);
  void *ChunkData=malloc(DataSize+2);
  if (ChunkData==NULL) ShowError(errCritical|errChunkHdr|errMemAlloc,"Cannot allocate memory for chunk data. Chunk is too big?");

  //Wczytujemy i zapisujemy
  LoadChunkData(AnimFile,ChunkData,DataSize,Options);
  FixChunkData(AnimChunkHdr,(char *)ChunkData,Options);
  SaveData(ChunkData,DataSize,DestFile);

  //Zwalniamy pami©†
  free(ChunkData);
  free(AnimChunkHdr);
  return Increment;
}

void ProcessFrameChunks(int AnimFile,int DestFile,FLIFrameHeader *FrameHdr,ulong CountedPos,ulong FrameEnd,int Options)
{
  ulong ChunkStartOffs=0;
  for (ulong k=0;k<(FrameHdr->chunks);k++)
    {
    if (Options & poManualSeeking)
      ChunkStartOffs=CountedPos;
     else
      ChunkStartOffs=tell(AnimFile);
    CountedPos+=ProcessChunk(AnimFile,DestFile,ChunkStartOffs,FrameEnd-ChunkStartOffs,k,Options);
    //printf(">>Pozycje: AnimFile %lu DestFile %lu\n",tell(AnimFile),tell(DestFile));
    };
  //Niekt¢re klatki wymagaj¥ drobnej korekcji
  if ((FrameHdr->chunks>1)&&((FrameHdr->size%2)>0))
    {
    lseek(DestFile,-1 ,SEEK_CUR);
    };
}

void ProcessFLIFile(int AnimFile,int DestFile)
/*
  Czyta wcze˜niej otwarty AnimFile i przepisuje
  do wcze˜niej otwartego DestFile
*/
{
  ulong CountedPos=0;//Ten parametr jest u¾ywany do ManualSeeking
  ulong FrameCount=0;

  //Przetwarzamy nagˆ¢wek
  CountedPos+=ProcessMainHeader(AnimFile,DestFile,FrameCount,Options);
  AskToDisplayInfo(Options);

  //Robimy tablic© poˆorzeä klatek
  ulong *FramePosTable=(ulong *)malloc(sizeof(ulong)*(FrameCount+2));
  if (FramePosTable==NULL) ShowError(errCritical|errFrameHdr|errMemAlloc,"Cannot allocate memory for frame-position table. Frames count too high.");
  FillFramePosTable(FramePosTable,AnimFile,FrameCount,Options);
  SetRequiredOptions(FramePosTable,AnimFile,FrameCount,Options);
  FLIFrameHeader *FrameHdr=(FLIFrameHeader *)malloc(sizeof(FLIFrameHeader)+1);
  if (FrameHdr==NULL) ShowError(errCritical|errFrameHdr|errMemAlloc,"Cannot allocate memory for frame header. Frames count too high?");

  //i przetwarzamy kolejne klatki wraz z chunksami
  for (ulong i=0;i<=FrameCount;i++)
    {
    CountedPos+=ProcessFrame(AnimFile,DestFile,FrameHdr,FramePosTable[i],FramePosTable[i+1],i+1,FrameCount+1,Options);
    ProcessFrameChunks(AnimFile,DestFile,FrameHdr,CountedPos,FramePosTable[i+1],Options);
    WaitForKeypress(Options);
    };

  //To wszystko - jeszcze zwalniamy pami©†
  free(FrameHdr);
  free(FramePosTable);
}

int main(int argc, char *argv[])
{
  //Otwieramy pliki
  int AnimFile;
  int DestFile;
  SayHello();
  if (argc<2) ShowError(errFileOpen+errCritical,"U did not specified source FLI filename.");
  OpenFLIFile(argv[1],AnimFile,O_RDONLY);
  OpenFLIFile(DestFName,DestFile,O_RDWR|O_CREAT|O_TRUNC);

  //Inicjujemy zmienne pocz¥tkowe
  Options=0;
  //Opcja !poManualSeeking znaczy brak seekingu (leci po kolei w pliku)
  //Opcja !poDisplayAllInfo znaczy nie zatrzymywanie si© i nie wy˜wietlanie info

  //Analizujemy plik
  ProcessFLIFile(AnimFile,DestFile);

  //No i zamykamy
  CloseFLIFiles(AnimFile,DestFile);
  return 0;
}
