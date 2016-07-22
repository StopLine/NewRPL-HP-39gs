/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"


// MOUNT A VOLUME IN A CARD
// Disk IS A CARD. MUST BE INITIALIZED AND SELECTED
// fs WILL BE FILLED WITH THE VOLUME INFORMATION
// VolNumber = 0-3 --> VOLUME TO MOUNT WITHIN PARTITION TABLE
// RETURNS FALSE IF VolNumber!=0 AND CARD IS NOT PARTITIONED

int FSMountVolume(SD_CARD *Disk,FS_VOLUME *fs, int VolNumber)
{
unsigned char *TempData,*TempData2;
int temp,datasect,resvd,rootdir;

// USE SMALL DATA BLOCKS
//if(!SDDSetBlockLen(Disk,5)) { /*printf("block failed\n");*/ return FALSE;}
//printf("block OK\n");


memsetb((void *)fs,0,sizeof(FS_VOLUME));

fs->Disk=Disk;
fs->VolNumber=VolNumber;

TempData=simpmallocb(512);
if(!TempData) return FALSE;

TempData2=TempData+440;


do {
//printf("Vaddr=%08X\n",fs->VolumeAddr);
//keyb_getkeyM(1);
if(!SDDRead((((uint64_t)fs->VolumeAddr)<<9),512,TempData,Disk)) { simpfree(TempData); return FALSE;}
//printf("Read1 OK\n");

//if(!SDDRead(fs->VolumeAddr+440,18*4,TempData2,fs->Disk)) { return FALSE;}

//printf("Read2 OK\n");

if(TempData2[70]!=0x55 || TempData2[71]!=0xaa) { simpfree(TempData); return FALSE;}		// NOT A BOOT/PARTITION SECTOR!


if((TempData[0]==0xeb || TempData[0]==0xe9)&&(TempData[21]==0xf8)) {

if( (fs->VolumeAddr==0) && (VolNumber!=0) ) { simpfree(TempData); return FALSE; }	// FAIL IF NO PARTITIONED CARD
// PROCESS AS A BOOT SECTOR
//printf("Boot Sector Found\n");
break;
}

// NOT A BOOT SECTOR, PROCESS AS A PARTITION, MOUNT THE FIRST PARTITION
//printf("Partition analysis\n");
if(fs->VolumeAddr!=0) VolNumber=0;			// IF THIS IS AN EXTENDED PARTITION, MOUNT ONLY THE FIRST LOGICAL DRIVE

if(    TempData2[10+(VolNumber<<4)]!=1			// FAT 12
    && TempData2[10+(VolNumber<<4)]!=4			// FAT 16
    && TempData2[10+(VolNumber<<4)]!=5			// EXTENDED FAT16
	&& TempData2[10+(VolNumber<<4)]!=6			// FAT 16 BIGDOS
	&& TempData2[10+(VolNumber<<4)]!=0xb		// FAT 32
	&& TempData2[10+(VolNumber<<4)]!=0xc		// FAT 32 LBA
	&& TempData2[10+(VolNumber<<4)]!=0xe		// FAT 16 LBA
	&& TempData2[10+(VolNumber<<4)]!=0xf)		// EXTENDED LBA
	                                       { simpfree(TempData); return FALSE;	}	// NOT A VALID FAT PARTITION

//printf("Found Valid filesystem %d\n",TempData2[10+(VolNumber<<4)]);

fs->VolumeAddr+=ReadInt32(TempData2+14+(VolNumber<<4));		// HARD-CODED FOR 512-byte SECTORS, SD CARD SectorSize NOT RELIABLE

} while(1);

//printf("Begin analysis of Boot sector\n");
//SDSelect(0); 		// DESELECT CARD

// APPARENT BOOT SIGNATURE FOUND
// PROCESS BPB

fs->SectorSize=ReadInt16(TempData+11);
if( fs->SectorSize!=512 &&
	fs->SectorSize!=1024 &&
	fs->SectorSize!=2048 &&
	fs->SectorSize!=4096) 	{ simpfree(TempData); return FALSE;}		// NOT A VALID SECTOR SIZE = BPB CORRUPTED OR NOT THERE
	
	
temp=9;
fs->SectorSize>>=9;
while(fs->SectorSize>1) { fs->SectorSize>>=1; temp++; }
fs->SectorSize=temp;											// CONVERT SECTORSIZE INTO POWER OF 2

fs->ClusterSize=0;
temp=TempData[13];
while(temp>1) { ++fs->ClusterSize; temp>>=1; }			// CONVERT INTO POWER OF 2

fs->NumFATS=TempData[16];
fs->TotalSectors= ReadInt16(TempData+19);
if(!fs->TotalSectors) fs->TotalSectors= ReadInt32(TempData+32);
fs->FATSize= ReadInt16(TempData+22);
if(!fs->FATSize) fs->FATSize= ReadInt32(TempData+36);

resvd=ReadInt16(TempData+14);		// GET RESERVED SECTORS
rootdir=((ReadInt16(TempData+17)<<5) + (1<<fs->SectorSize) -1) >> fs->SectorSize;  // CALCULATE ROOT DIR SIZE

datasect=(resvd + (fs->NumFATS*fs->FATSize) + rootdir);
fs->Cluster0Addr=fs->VolumeAddr+ ( (datasect- (2<<fs->ClusterSize)) << (fs->SectorSize-9));

datasect=fs->TotalSectors-datasect;
fs->TotalClusters=datasect >> fs->ClusterSize;

fs->FirstFATAddr=fs->VolumeAddr+ (resvd << (fs->SectorSize-9));

// FINAL CLUSTERSIZE IS IN BYTES
fs->ClusterSize+=fs->SectorSize;

// RESET ROOTDIR ENTRY
memsetb((void *)&(fs->RootDir),0,sizeof(FS_FILE));
fs->RootDir.Volume=fs->VolNumber;
fs->RootDir.Mode=FSMODE_READ | FSMODE_MODIFY;
fs->RootDir.Attr=FSATTR_DIR;


if(fs->TotalClusters>=65525) {
// FAT32
fs->FATType=3;
// SPECIAL FAT32 MIRRORING HANDLING
temp=TempData[40];				// GET BITS 0-7 OF EXTENDED FLAGS
if(temp&0x80) {
// USE ONLY ACTIVE FAT IF MIRRORING IS DISABLED
fs->NumFATS=1;
fs->FirstFATAddr+= ((temp&0xf)*fs->FATSize) << (fs->SectorSize-9);
}



// ROOTDIR FOR FAT32 INIT HERE

fs->RootDir.FirstCluster=ReadInt32(TempData+44);
if(!FSGetChain(fs->RootDir.FirstCluster,&(fs->RootDir.Chain),fs)) return FALSE;
fs->RootDir.FileSize=FSGetChainSize(&(fs->RootDir.Chain));



}
else {
	if(fs->TotalClusters< 4085) {
	fs->FATType=1;
	// ADD FAT12 SPECIFIC ROUTINES HERE
	}
	else {
	fs->FATType=2;
	// ADD FAT16 SPECIFIC ROUTINES HERE
	}
	// ADD COMMON FAT12/FAT16 INIT HERE


    fs->RootDir.Chain.StartAddr= fs->FirstFATAddr + ((fs->NumFATS*fs->FATSize)<<(fs->SectorSize-9));
    fs->RootDir.Chain.EndAddr=fs->RootDir.Chain.StartAddr+(rootdir<<(fs->SectorSize-9));
	fs->RootDir.Chain.NextFragment=NULL;
	fs->RootDir.FileSize=rootdir<<fs->SectorSize;
	fs->RootDir.Mode|=FSMODE_NOGROW;

}

// FINAL STAGE
fs->CurrentDir=&fs->RootDir;
fs->NextFreeCluster=2;

fs->InitFlags=1;

simpfree(TempData);

FSCalcFreeSpace(fs);

return TRUE;
}
