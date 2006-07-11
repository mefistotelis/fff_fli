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
  printf("\nThis program can repair damaged or modified FLI files.\n");
  printf("Repaired file is saved as '%s'\n",DestFName);
  printf("%20s (c) by Tomasz Lis, Gdaäsk, Poland 2004\n\n","");
}

void OpenFLIFile(char *FName,int &File,int Mode)
{
  File=open(FName,Mode|O_BINARY,S_IREAD|S_IWRITE);
  if (File<1) ShowError(errFileOpen+errCritical,strerror(errno));
}

void WaitForKeypress(int &Options)
{
  if ((Options & poDisplayAllInfo)&&(!(Options & poNeverWaitForKey)))
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
  if (!(Options & poNeverWaitForKey))
    {
    printf("\n (Press any key to leave program)");
    getch();
    };
  printf("\n");
}

void AskToDisplayInfo(int &Options)
{
  if (!(Options & poNeverWaitForKey))
    {
    printf(" (Press ENTER to display additional informations)\n");
    printf(" (Other key cancels full displaying)\n");
    char X=getch();
    if (X==kbEnter) Options|=poDisplayAllInfo; else Options&=(0xffff-poDisplayAllInfo);
    };
}

int ProcessMainHeader(int AnimFile,int DestFile,FLIMainHeader *AnimHeaderDest,int &Options)
{
  //Zmienne lokalne
  ulong Increment=sizeof(FLIMainHeader);
  ulong StartPos=tell(AnimFile);

  //Allokujemy pami©† na nagˆ¢wek
  FLIMainHeader *AnimHeaderSrc=(FLIMainHeader *)AllocateMem(sizeof(FLIMainHeader)+1,errCritical|errMainHdr,Options);
  FLIAddHeader *AnimAddHeaderSrc=(FLIAddHeader *)AllocateMem(sizeof(FLIAddHeader)+1,errCritical|errAddHdr,Options);
  //Czytamy nagˆ¢wek
  LoadMainHeader(AnimHeaderSrc,AnimAddHeaderSrc,AnimFile,Options);

  //Tworzymy poprawion¥ kopie nagˆ¢wk¢w
  FLIAddHeader *AnimAddHeaderDest=(FLIAddHeader *)AllocateMem(sizeof(FLIAddHeader)+1,errCritical|errAddHdr,Options);
  FixMainHeader(AnimHeaderSrc,AnimAddHeaderSrc,AnimHeaderDest,AnimAddHeaderDest,filelength(AnimFile),Options);

  //Wy˜wietlamy informacje o nagˆ¢wkach
  DisplayMainHeaderInfo(AnimHeaderSrc,StartPos,filelength(AnimFile));
  AskToDisplayInfo(Options);
  if (Options & poDisplayAllInfo)
    DisplayAddHeaderInfo(AnimAddHeaderSrc,tell(AnimFile)-1);

  //Zapisujemy nagˆ¢wki
  SaveDataToFile(AnimHeaderDest,sizeof(FLIMainHeader),DestFile);
  SaveDataToFile(AnimAddHeaderDest,sizeof(FLIAddHeader),DestFile);

  //Aktualizujemy zmienne wyj˜ciowe
  if (HasAddHeader(AnimHeaderSrc))
    Increment+=sizeof(FLIAddHeader);

  //Pozostaje zwolnienie pami©ci
  free(AnimHeaderSrc);
  free(AnimAddHeaderSrc);
  free(AnimAddHeaderDest);

  return Increment;
}

ulong ProcessFrame(int AnimFile,int DestFile,FLIFrameHeader *AnimFrameHdr,ulong FramePos,ulong FrameEnd,ulong FrameNum,ulong TotalFrames,int Options)
{
  ulong Increment=sizeof(FLIFrameHeader);
  LoadFrameHeader(AnimFile,AnimFrameHdr,FramePos,FrameNum,TotalFrames,Options);
  FixFrameHeader(AnimFrameHdr,FramePos,FrameEnd,Options);

  //Robimy kopi© nagˆ¢wna pod nazw¥ DestFrameHdr
  FLIFrameHeader *DestFrameHdr=(FLIFrameHeader *)AllocateMem(sizeof(FLIFrameHeader)+1,errCritical|errFrameHdr,Options);
  memmove(DestFrameHdr,AnimFrameHdr,sizeof(FLIFrameHeader));

  //Zapisujemy t¥ kopi© - po drobnych modyfikacjach
  FixFrameHdrToAddChunks(DestFrameHdr,FrameNum,Options);
  SaveDataToFile(DestFrameHdr,sizeof(FLIFrameHeader),DestFile);
  AddRequiredChunksAtFrameStart(DestFile,FrameNum,Options);
  free(DestFrameHdr);
  return Increment;
}

long PosInFrameTable(ulong Offset,ulong *FramePosTable,ulong TableSize)
{
  ulong ElemPos=0;
  while ((ElemPos<TableSize)&&(FramePosTable[ElemPos]<Offset))
    ElemPos++;
  //Sprawdzamy czy klatki nie ma ju¾ w tablicy
  if (FramePosTable[ElemPos]==Offset)
    return ElemPos;
   else
    return -1;
}

long InsertFrameToTable(ulong *&FramePosTable,unsigned int FrameCount,ulong &FoundedFrames,ulong Offset)
{
  //Znajdujemy pozycj© do wstawienia nowej klatki
  ulong InsertPos=0;
  while ((InsertPos<FoundedFrames)&&(FramePosTable[InsertPos]<Offset))
    {
    InsertPos++;
    };
  //Sprawdzamy czy klatki nie ma ju¾ w tablicy
  if (FramePosTable[InsertPos]==Offset)
    {
    return 0;
    };
  //Je¾eli tablica jest caˆa peˆna, musimy przeallokowa† pami©†
  if (FoundedFrames>FrameCount)
    {
    return -1;//Na razie tego nie robi© - powinno nie by† potrzebne
    };
  //Przenosimy o jedno pole wszystkie elementy za miejscem wstawienia wˆ¥cznie
  for (long i=FrameCount+1;i>=(long)InsertPos;i--)
    {
//printf("Przenosz© el. %li by wstawi† na poz. %lu\n",i,InsertPos);
    FramePosTable[i+1]=FramePosTable[i];
    };
  //I wstawiamy nasz¥ klatk© w utworzone miejsce
  FramePosTable[InsertPos]=Offset;
  //Oczywi˜cie ilo˜† klatek wzrosˆa
  FoundedFrames++;
  return InsertPos+1;
}

const long deltaRange=65535;

void FindLostFrames(ulong *&FramePosTable,int AnimFile,unsigned int FrameCount,ulong &FoundedFrames,ulong FullFrameSize,int Options)
{
  printf("    Some frames are missing. Parsing whole source file...\n");
  //Przygotowujemy bufory
  ulong BufSize=sizeof(FLIFrameHeader)+sizeof(FLIChunkHeader);
  void *Buf=AllocateMem(BufSize+1,errFrameHdr|errOnlyParsing,Options);
  if (Buf==NULL) return;
  FLIFrameHeader *FrameHdr=(FLIFrameHeader *)&(((char *)Buf)[0]);
  FLIChunkHeader *ChunkHdr=(FLIChunkHeader *)&(((char *)Buf)[sizeof(FLIFrameHeader)]);
  //Przegl¥damy kolejne offsety w poszukiwaniu klatek
  for (ulong Offset=0;Offset<=(filelength(AnimFile)-BufSize);Offset++)
    {
    //Wczytujemy nagˆ¢wki
    lseek(AnimFile,Offset ,SEEK_SET);
    if (!LoadDataFromFile(AnimFile,Buf,BufSize,errFrameHdr|errOnlyParsing,Options))
      {
      free(Buf);
      return;
      };
    //Sprawdzamy czy nagˆ¢wki pasuj¥ do wygl¥du klatek
    if (StrictValidFrame(FrameHdr,FullFrameSize+2*MaxPalSize+sizeof(FLIFrameHeader)+3*sizeof(FLIChunkHeader)))
      {
      if ((FrameHdr->chunks==0) || ((FrameHdr->chunks>0) && ValidChunk(ChunkHdr,FullFrameSize+sizeof(FLIChunkHeader))) )
        {
        if (Options & poDisplayAllInfo)
          printf("    ->Founded valid frame at %9lu, ",Offset);
        long InsResult=InsertFrameToTable(FramePosTable,FrameCount,FoundedFrames,Offset);
        if (Options & poDisplayAllInfo)
          {
          if (InsResult>0)  printf("added to frame table as frame %li.\n",InsResult);
          if (InsResult==0) printf("already is in frame table.\n");
          if (InsResult<0)  printf("cannot add, frame table full.\n");
          };
        };
      };//end if (StrictValidFrame(...
    };//end for (ulong Offset...
  printf("    After searching, we have %lu frames (the file informed of %u frames)\n",FoundedFrames,FrameCount+1);
  free(Buf);
}

enum BestFrameChoise
  {
   cbfOldOffset,
   cbfFixedOffset,
   cbfNothing
  };

ulong FindBestPosition(ulong *FramePosTable,int AnimFile,ulong lastFrame,ulong foundedFrames,ulong Offset,int ParseOptions)
{
  signed long delta;
  signed long newDelta=0;
  int FrameIsValid=0;
  ulong NewOffset;
  FLIFrameHeader *AnimFrameHdr=(FLIFrameHeader *)AllocateMem(sizeof(FLIFrameHeader)+1,errCritical|errFrameHdr,Options);
      do
	{
	delta=newDelta;
	//delta ma by† ci¥giem: 0 -1 1 -2 2 -3 3 ....
	if (newDelta>=0)
	  { newDelta++; newDelta*=(-1); }
	 else
	  { newDelta*=(-1); };
	//Dla kolejnych delt pr¢bujemy wczytywa† FrameHeader
	//Nie ma wi©kszego znaczenia czy b©dziemy tu por¢wnywa† (Offset-Delta) czy (Offset+Delta)
	NewOffset=Offset+delta;
	if ( (NewOffset>(sizeof(FLIMainHeader)-2)) && (NewOffset<=(filelength(AnimFile)-sizeof(FLIFrameHeader))) )
	  LoadFrameHeader(AnimFile,AnimFrameHdr,NewOffset,foundedFrames+1,0,ParseOptions);
	 else
	  ClearFLIFrameHdr(AnimFrameHdr);
//if ((delta>0)&&((delta%20000)==0)) printf("Problemy ze znalezieniem klatki %lu z pozycji %lu\n",foundedFrames+1,Offset);
        FrameIsValid=ValidFrame(AnimFrameHdr,filelength(AnimFile)-FramePosTable[lastFrame]);
	if ((FrameIsValid)&&(PosInFrameTable(NewOffset,FramePosTable,foundedFrames)<0))
          {
          free(AnimFrameHdr);
          return NewOffset;
          };
	}
       while ((delta<deltaRange));
  free(AnimFrameHdr);
  return Offset;
}

int ChooseBestFrame(int AnimFile,ulong MaxFrameSize,ulong Offset,ulong NewOffset,int Options)
/*
 Zwraca    cbfOldOffset, cbfFixedOffset lub  cbfNothing
*/
{
  //Bufory na obydwie klatki
  FLIFrameHeader *NewFrameHdr=(FLIFrameHeader *)AllocateMem(sizeof(FLIFrameHeader)+1,errCritical|errFrameHdr|errOnlyParsing,Options);
  FLIFrameHeader *OldFrameHdr=(FLIFrameHeader *)AllocateMem(sizeof(FLIFrameHeader)+1,errCritical|errFrameHdr|errOnlyParsing,Options);
  //Ustawienia wst©pne zmiennych
  int AllowOld=1;
  int AllowFix=1;
  int AllowNot=1;
  //adujemy klatki
  LoadFrameHeader(AnimFile,NewFrameHdr,NewOffset,0,0,Options);
  LoadFrameHeader(AnimFile,OldFrameHdr,   Offset,0,0,Options);
  //i robimy testy - najpierw proste
  if (Options & poNeverSkipFrames)
    AllowNot=0;
  if ((Offset==NewOffset)||(!(Options & poFixFramePositions)))
    AllowFix=0;
  if (!ValidFrame(NewFrameHdr,MaxFrameSize))
    AllowFix=0;
  if (AllowFix||AllowNot)
    if (!ValidFrame(OldFrameHdr,MaxFrameSize))
      AllowOld=0;
  //Je˜li si© nie rozwi¥zaˆo, trzeba mu bardziej dowali†
  if (AllowOld||AllowNot)
    if (NewOffset>filelength(AnimFile)-sizeof(FLIFrameHeader))
      AllowFix=0;
  if (AllowFix||AllowNot)
    if (Offset>filelength(AnimFile)-sizeof(FLIFrameHeader))
      AllowOld=0;
  if (AllowFix||AllowOld)
    if (ValidFrame(OldFrameHdr,MaxFrameSize)||ValidFrame(NewFrameHdr,MaxFrameSize))
      AllowNot=0;
  if (AllowFix||AllowNot)
    if (!StrictValidFrame(OldFrameHdr,MaxFrameSize))
      AllowOld=0;
  if (AllowOld||AllowNot)
    if (!StrictValidFrame(NewFrameHdr,MaxFrameSize))
      AllowFix=0;
  if (AllowFix&&AllowOld)
    if (NewOffset<Offset)
      AllowOld=0;
     else
      AllowFix=0;
  //Zwalniamy klatki
  free(NewFrameHdr);
  free(OldFrameHdr);
  //I zwracamy co trzeba
  if (AllowFix) return cbfFixedOffset;
  if (AllowNot) return cbfNothing;
  return cbfOldOffset;
}

void FillFramePosTable(ulong *&FramePosTable,int AnimFile,unsigned int &FrameCount,ulong FullFrameSize,int Options)
/*
  Szuka pocz¥tk¢w klatek i zapisuje je w tablicy.
  Potrzebuje nie zwi©kszonej o 1 ilo˜ci klatek, wprost z nagˆ¢wka (FrameCount).
  Zakˆada, ¾e jest w pliku na pozycji zaraz po nagˆ¢wku
*/
{
  printf("Creating Frame-Position Table...\n");
  if (Options & poFixFramePositions)
    printf("*Frames position fixing active.\n");
  //Zmienna pami©taj¥ca miejsce gdzie szukamy klatki
  ulong Offset=tell(AnimFile);
  ulong foundedFrames=0;
  signed long InsResult=0;
  //Uproszczone opcje, dla przegl¥dania pliku
  int ParseOptions=(Options & (0xffff - poDisplayAllInfo)) | poIgnoreExceptions;
  //Wyczy˜†my tablic©
  ClearNBufferBytes((char *)FramePosTable,(FrameCount+2)*sizeof(ulong));
  //Nagˆ¢wek klatki
  FLIFrameHeader *AnimFrameHdr=(FLIFrameHeader *)AllocateMem(sizeof(FLIFrameHeader)+1,errCritical|errFrameHdr,Options);
  //Teraz trzeba przegl¥da† kolejne klatki
  while (Offset<filelength(AnimFile))
    {
    //Alternatywny offset pocz¥tku klatki
    ulong fixedOffset=0;
    //Do sprawdzenia czy nie nakˆadamy ni© na poprzedni trzeba dodatkow¥ zmienn¥, bo
    //FramePosTable[lastFrame] musi by† zawsze zdefiniowane - po to tyle piepszenia
    ulong lastFrame;
    if (foundedFrames>0) lastFrame=foundedFrames-1; else lastFrame=0;
    //Tablica jest wyczyszczona, wi©c w tym momencie FramePosTable[foundedFrames]=0
    if (Options & poFixFramePositions) fixedOffset=FindBestPosition(FramePosTable,AnimFile,lastFrame,foundedFrames,Offset,ParseOptions);
    //Teraz nale¾y wybra† kt¢ry˜ z nich, lub po prostu zwi©kszy† Offset i szuka† nast©pnego
	if (Options & poDisplayAllInfo)
	  printf("    Possible frame no %4lu, ",(ulong)foundedFrames+1);
    switch (ChooseBestFrame(AnimFile,FullFrameSize+2*MaxPalSize+sizeof(FLIFrameHeader)+3*sizeof(FLIChunkHeader),Offset,fixedOffset,ParseOptions))
      {
      case cbfOldOffset:
	{
	printf("founded at expected offset %7lu, ",Offset);
	LoadFrameHeader(AnimFile,AnimFrameHdr,Offset,0,0,ParseOptions);
	InsResult=InsertFrameToTable(FramePosTable,FrameCount,foundedFrames,Offset);
	Offset+=AnimFrameHdr->size;
	break;
	};
      case cbfFixedOffset:
	{
	printf("founded at adjusted offset %7lu, ",fixedOffset);
	LoadFrameHeader(AnimFile,AnimFrameHdr,fixedOffset,0,0,ParseOptions);
	InsResult=InsertFrameToTable(FramePosTable,FrameCount,foundedFrames,fixedOffset);
	Offset=fixedOffset+AnimFrameHdr->size;
	break;
	};
      case cbfNothing:
	{
	printf("not founded near offset   %7lu, ",Offset);
	Offset+=deltaRange;
	InsResult=-255;
	break;
	};
      default:
	ShowError(errImpossible,"");
      };
    if (Options & poDisplayAllInfo)
      {
      if (InsResult==-255) printf("skipped.\n");
       else
      if (InsResult>0)  printf("added to frame table.\n");
       else
      if (InsResult==0) printf("already is in frame table.\n");
       else
      if (InsResult<0)  printf("cannot add, frame table full.\n");
      };
    };
  if (foundedFrames<FrameCount+1)
    {
    if (Options & poUseFrameFinder)
	{
        //Wywoˆanie FrameFindera znajdzie pozostaˆe klatki
	FindLostFrames(FramePosTable,AnimFile,FrameCount,foundedFrames,FullFrameSize,Options);
        };
    };
  if (foundedFrames<FrameCount+1)
    {
    printf("-->Only %lu frames out of %lu founded. Frames count FIXED.\n",foundedFrames,(ulong)FrameCount+1);
    }
   else
    printf("  All frames listed in table.\n");
  FramePosTable[FrameCount+2]=filelength(AnimFile);
  free(AnimFrameHdr);
}

void ParseFileToSetOptions(ulong *FramePosTable,int AnimFile,ulong FrameCount,int &Options)
{
  printf("Parsing file to set optimal options...\n");
  int ParseOptions=(Options & (0xffff - poDisplayAllInfo)) | poIgnoreExceptions;
  int FirstFrameHasPalette=0;
  int ExpandPalette=1;
  //Robimy sobie pami©† na klatk© i nagˆ¢wek chunka
  FLIFrameHeader *FrameHdr=(FLIFrameHeader *)malloc(sizeof(FLIFrameHeader)+1);
  FLIChunkHeader *AnimChunkHdr=(FLIChunkHeader *)malloc(sizeof(FLIChunkHeader)+1);
  if ((FrameHdr==NULL)||(AnimChunkHdr==NULL))
    {
    ShowError(errMemAlloc,"Cannot allocate memory for parsing (to auto-configure). Options set do defaults.");
    free(AnimChunkHdr);
    free(FrameHdr);
    return;
    };
  //i przegl¥damy kolejne klatki wraz z chunksami
  for (ulong i=0;i<=FrameCount;i++)
    {
    //Klatka
    LoadFrameHeader(AnimFile,FrameHdr,FramePosTable[i],i+1,FrameCount+1,ParseOptions);
    FixFrameHeader(FrameHdr,FramePosTable[i],FramePosTable[i+1],ParseOptions);
    //Teraz Chunksy
    ulong ChunkStartOffs;
    for (ulong k=0;k<(FrameHdr->chunks);k++)
      {
      ChunkStartOffs=tell(AnimFile);
      //Dopasowujeny offset Chunka
      //!!!!AdjustChunkOffset(AnimFile,,,Options);
      //Wczytujemy nagˆ¢wek chunka
      LoadChunkHeader(AnimFile,AnimChunkHdr,ChunkStartOffs,k+1,ParseOptions);
      FixChunkHeader(AnimChunkHdr,FramePosTable[i+1]-ChunkStartOffs,ParseOptions);
      //Tworzymy bufor do wczytania zawarto˜ci
      ulong DataSize=AnimChunkHdr->size-sizeof(FLIChunkHeader);
      void *ChunkData=malloc(DataSize+2);
      if (ChunkData==NULL)
        {
        ShowError(errChunkHdr|errMemAlloc,"Cannot allocate memory for chunk data when parsing. Options set to defaults - detection failed.");
        free(AnimChunkHdr);
        free(FrameHdr);
        return;
        };
      //Wczytujemy zawarto˜† chunka
      LoadChunkData(AnimFile,ChunkData,DataSize,ParseOptions);
      FixChunkData(AnimChunkHdr,ChunkData,DataSize,ParseOptions);
      /*
      Czas by sprawdzi† co nam ten chunk da
        Mamy: i - nr klatki         k - nr chunksa            FramePosTable[] - granice
              AnimChunkHdr          ChunkData                 FrameHdr
      */
      //Opcja: czy dodawa† palet©
      if ((i==0)&&((AnimChunkHdr->type==FLI_COLOR)||(AnimChunkHdr->type==FLI_COLOR256)))
        FirstFrameHasPalette=1;
      //Opcja: czy zmienia† typ palety
      if (AnimChunkHdr->type==FLI_COLOR256)
        {
        if (!PalShallBeMultiplied(ChunkData,DataSize))
          {
          //printf("Zrezygnowano z pal*4 po chunksie %lu z klatki %lu\n",k,i);
          ExpandPalette=0;
          };
        };
      //Koniec Opcji - Zwalniamy pami©†
      free(ChunkData);
      };
    };
  //Teraz mo¾emy spokojnie ustawi† wˆa˜ciwe opcje
  if (!FirstFrameHasPalette)
    {
    printf("*Patette reconstruction from .PAL file activated.\n");
    Options|=poRecostructPatette;
    };
  if ((FirstFrameHasPalette)&&(ExpandPalette))
    {
    printf("*Palette type indicator correction activated.\n");
    Options|=poExpandPatette;
    };

  //To wszystko - jeszcze zwalniamy pami©†
  free(AnimChunkHdr);
  free(FrameHdr);
}


/*
void AdjustChunkOffset(ulong ChunkPos,int AnimFile,ulong RangeStart,ulong RangeEnd,int Options)
{
}
*/

ulong ProcessChunk(int AnimFile,int DestFile,ulong ChunkPos,ulong MaxSize,ulong ChunkNum,int Options)
{
  //Ustawienia i testy pocz¥tkowe
  ulong Increment=sizeof(FLIChunkHeader);
  ulong DestLastChunkStart=tell(DestFile);
  int ShallRemoveChunk=0;

  //Dopasowujeny offset Chunka
  //!!!!AdjustChunkOffset(AnimFile,,,Options);

  //Wczytujemy jego nagˆ¢wek
  FLIChunkHeader *AnimChunkHdr=(FLIChunkHeader *)AllocateMem(sizeof(FLIChunkHeader)+1,errCritical|errChunkHdr,Options);
  if (ChunkPos+sizeof(FLIChunkHeader) <= filelength(AnimFile))
    LoadChunkHeader(AnimFile,AnimChunkHdr,ChunkPos,ChunkNum,Options);
   else
    {
    ClearFLIChunkHdr(AnimChunkHdr);
    ShallRemoveChunk=1;
    };
  //Zapami©tujemy czy chunk byˆ poprawny przed naprawieniem
  if ((Options & poRemoveBadChunks)&&(!ShallRemoveChunk))
    ShallRemoveChunk=(!ValidChunk(AnimChunkHdr,MaxSize));
  if (Options & poFixChunkHeaders)
    FixChunkHeader(AnimChunkHdr,MaxSize,Options);

  //Tworzymy bufor do wczytania zawarto˜ci
  ulong DataSize=AnimChunkHdr->size-sizeof(FLIChunkHeader);
  void *ChunkData=AllocateMem(DataSize+2,errCritical|errChunkData,Options);

  //Wczytujemy zawarto˜† chunka
  LoadChunkData(AnimFile,ChunkData,DataSize,Options);
  FixChunkData(AnimChunkHdr,ChunkData,DataSize,Options);

if ( (Options & poRemoveBadChunks) &&
     ((!ValidChunkData(AnimChunkHdr->type,ChunkData,DataSize))||ShallRemoveChunk) )
    {
    Increment=0;
//    if ((Options & poDisplayAllInfo))
      printf("-->Chunk no %lu looks suspicious - REMOVED\n",ChunkNum);
    }
   else
    {
    //Zapisujemy i urealniamy
    SaveDataToFile(AnimChunkHdr,sizeof(FLIChunkHeader),DestFile);
    SaveDataToFile(ChunkData,DataSize,DestFile);
    RewriteChunkHeader(DestFile,DestLastChunkStart,AnimChunkHdr);
    };

  //Zwalniamy pami©†
  free(ChunkData);
  free(AnimChunkHdr);
  return Increment;
}

void ProcessFrameChunks(int AnimFile,int DestFile,FLIFrameHeader *FrameHdr,ulong CountedPos,ulong FrameEnd,int Options)
{
  ulong ChunkStartOffs=0;
  ulong RealChunksNumber=0;
  ulong ChunkSize;
  for (ulong k=0;k<(FrameHdr->chunks);k++)
    {
    if (Options & poManualSeeking)
      ChunkStartOffs=CountedPos;
     else
      ChunkStartOffs=tell(AnimFile);
    ChunkSize=ProcessChunk(AnimFile,DestFile,ChunkStartOffs,FrameEnd-ChunkStartOffs,k+1,Options);
    if (ChunkSize>0)
      {
      CountedPos+=ChunkSize;
      RealChunksNumber++;
      };
    //printf(">>Pozycje: AnimFile %lu DestFile %lu\n",tell(AnimFile),tell(DestFile));
    };
  //Niekt¢re klatki wymagaj¥ drobnej korekcji
  FrameHdr->chunks=RealChunksNumber;
  if ((FrameHdr->chunks>1)&&((FrameHdr->size%2)>0))
    {
    lseek(DestFile,-1 ,SEEK_CUR);
    };
}

void ProcessFLISimple(int AnimFile,int DestFile,ulong StartFrame,ulong EndFrame,int &Options)
/*
  Wersja uproszczona ProcessFLIFile - do test¢w
*/
{
  ulong CountedPos=0;//Ten parametr jest u¾ywany do ManualSeeking

  //Przetwarzamy nagˆ¢wek
  FLIMainHeader *AnimMainHeader=(FLIMainHeader *)AllocateMem(sizeof(FLIMainHeader)+1,errCritical|errMainHdr,Options);
  CountedPos+=ProcessMainHeader(AnimFile,DestFile,AnimMainHeader,Options);

  //Po prostu przepisujemy zawarto˜† pliku do DestFile
  ulong BufSize=4096;
  void *Buf=AllocateMem(BufSize+1,errPlainData|errCritical,Options);
  unsigned int nRead=1;
  while (nRead!=0)
    {
    nRead=read(AnimFile,Buf,BufSize);
    if (nRead!=0)
      SaveDataToFile(Buf,nRead,DestFile);
    };
  //Na koniec poprawiamy MainHeader
  if (StartFrame>AnimMainHeader->frames) StartFrame=AnimMainHeader->frames;
  if (EndFrame>AnimMainHeader->frames) EndFrame=AnimMainHeader->frames;
  if (StartFrame>EndFrame) EndFrame=StartFrame;
  RewriteMainHeader(DestFile,EndFrame-StartFrame,AnimMainHeader);

  //To wszystko - jeszcze zwalniamy pami©†
  free(AnimMainHeader);
}

void ProcessFLIFile(int AnimFile,int DestFile,ulong StartFrame,ulong EndFrame)
/*
  Czyta wcze˜niej otwarty AnimFile i przepisuje
  do wcze˜niej otwartego DestFile
*/
{
  ulong CountedPos=0;//Ten parametr jest u¾ywany do ManualSeeking
  ulong DestLastFramePos=0;

  //Przetwarzamy nagˆ¢wek
  FLIMainHeader *AnimMainHeader=(FLIMainHeader *)AllocateMem(sizeof(FLIMainHeader)+1,errCritical|errMainHdr,Options);
  CountedPos+=ProcessMainHeader(AnimFile,DestFile,AnimMainHeader,Options);

  //Robimy tablic© poˆorzeä klatek
  ulong *FramePosTable=(ulong *)AllocateMem(sizeof(ulong)*(AnimMainHeader->frames+3),errCritical|errFramePosTbl,Options);
  FillFramePosTable(FramePosTable,AnimFile,AnimMainHeader->frames,AnimMainHeader->width*AnimMainHeader->height,Options);
  ParseFileToSetOptions(FramePosTable,AnimFile,AnimMainHeader->frames,Options);
  FLIFrameHeader *FrameHdr=(FLIFrameHeader *)AllocateMem(sizeof(FLIFrameHeader)+1,errCritical|errFramePosTbl,Options);

  //i przetwarzamy kolejne klatki wraz z chunksami
  if (StartFrame>AnimMainHeader->frames) StartFrame=AnimMainHeader->frames;
  if (EndFrame>AnimMainHeader->frames) EndFrame=AnimMainHeader->frames;
  if (StartFrame>EndFrame) EndFrame=StartFrame;
  for (ulong i=StartFrame;i<=EndFrame;i++)
    {
    DestLastFramePos=tell(DestFile);
    CountedPos+=ProcessFrame(AnimFile,DestFile,FrameHdr,FramePosTable[i],FramePosTable[i+1],i+1,AnimMainHeader->frames+1,Options);
    ProcessFrameChunks(AnimFile,DestFile,FrameHdr,CountedPos,FramePosTable[i+1],Options);
    RewriteFrameHeader(DestFile,DestLastFramePos,i+1,FrameHdr,Options);
    WaitForKeypress(Options);
    };

  //Na koniec poprawiamy MainHeader
  RewriteMainHeader(DestFile,EndFrame-StartFrame,AnimMainHeader);

  //To wszystko - jeszcze zwalniamy pami©†
  free(AnimMainHeader);
  free(FrameHdr);
  free(FramePosTable);
}

int main(int argc, char *argv[])
{
  //Inicjujemy zmienne
  int AnimFile;
  int DestFile;
  //Klatki numerujemy od 0
  ulong StartFrame=0;
  ulong EndFrame=MAXLONG;
  //Opis opcji jest w pliku z definicj¥ ich typu
  Options=poDisplayAllInfo|poFixMainHeader|poRemoveBadChunks|
          poFixFramePositions|poUseFrameFinder|poFixFrameHeaders;
  Options|=poRemoveBadChunks|poFixChunkHeaders|poNeverWaitForKey;
  //
  //Options=poSimpleFix;
  //Powitanie
  SayHello();

  //Tu powinna by† analiza parametr¢w wej˜ciowych
  if (argc<2) ShowError(errFileOpen+errCritical,"U did not specified source FLI filename.");
//  StartFrame=622;
//  EndFrame=9999;

  //Otwieramy pliki
  OpenFLIFile(argv[1],AnimFile,O_RDONLY);
  OpenFLIFile(DestFName,DestFile,O_RDWR|O_CREAT|O_TRUNC);

  //Analizujemy plik
  if (Options & poSimpleFix)
    ProcessFLISimple(AnimFile,DestFile,StartFrame,EndFrame,Options);
   else
    ProcessFLIFile(AnimFile,DestFile,StartFrame,EndFrame);

  //No i zamykamy
  CloseFLIFiles(AnimFile,DestFile);
  return 0;
}
