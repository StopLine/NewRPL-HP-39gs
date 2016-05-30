/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"







// FILLS IN A FS_FILE STRUCTURE WITH THE DIRECTORY ENTRY
// name MUST BE A VALID NAME (NO ABSOLUTE PATH ALLOWED, NO VALIDITY CHECK)
// caseflag== 0 --> CASE-SENSITIVE NAME COMPARISON
// 	       == 1 --> CASE-SENSITIVE W/SEMICOLON STRIPPING (CALCULATOR-FRIENDLY)
//         == 2 --> CASE-INSENSITIVE NAME COMPARISON
// returns FS_EOF if not found, FS_OK if found or FS_ERROR if there's a problem

int FSFindForCreation(char *name,FS_FILECREATE *cr,FS_FILE *dir)
{
FS_VOLUME *fs;
int order,diroffset;
int namelen;
int nentries;
char checksum;
int nbytesread;
char buffer[32],*morebuff;
char *ptr;
char shortn[13];
char *shname;
char *newname;
char *entryname;

if(!FSystem.Init) return FS_ERROR;
if(!dir || !cr) return FS_ERROR;

fs=FSystem.Volumes[dir->Volume];

if(!fs) return FS_ERROR;

newname=(char *)malloc(257);
if(!newname) return FS_ERROR;
shname=(char *)malloc(257);
if(!shname) {free(newname); return FS_ERROR; }
entryname=(char *)malloc(257);
if(!entryname) { free(newname); free(shname); return FS_ERROR; }

// INITIALIZE COUNTERS

memset((void *)cr,0,sizeof(FS_FILECREATE));

if(dir->Mode&FSMODE_NOGROW) cr->DirMaxEntries=dir->FileSize>>5;
else cr->DirMaxEntries=65536;

// DETERMINE LENGTH OF STRING
namelen=(int)strlen((char *)name);

// SAFE OBTAIN NAME OF NEW FILE
if(namelen>255) {
	memcpy(newname,name,255);
	newname[255]=0;		// TRUNCATE FILE NAME IF >255 CHARS
}
else strcpy((char *)newname,(char *)name);

// STRIP SEMICOLONS IF PRESENT
if( FSystem.CaseMode==FSCASE_SENSHP || (FSystem.CaseMode==FSCASE_SENSHPTRUE)) FSStripSemi(newname);

namelen=strlen((char *)newname);

// EXTRACT ROOT NAME TO CHECK FOR CONFLICTS

strcpy(shname,newname);

cr->NameFlags=FSConvert2ShortEntry(shname,0);

if(cr->NameFlags&1) cr->ShortNum=1;			// MINIMUM NUMBER NEEDED =1



// READY TO BEGIN DIRECTORY SCAN

//printf("Search... %s\n",name);
//printf("dir size=%d\n",dir->FileSize);
//keyb_getkeyM(1);
FSSeek(dir,0,FSSEEK_SET);

while((nbytesread=FSReadLL(buffer,32,dir,fs))==32)
{
//printf("x");
if(buffer[0]==0) break;
++cr->DirUsedEntries;
if(buffer[0]==0xe5) continue;	// DELETED ENTRY, USE NEXT ENTRY
if( (buffer[11]&FSATTR_LONGMASK) == FSATTR_LONGNAME) {

//	printf("LFN entry found\n");
	// TREAT AS LONG FILENAME
	if(!(buffer[0]&0X40)) continue;		// ORPHAN ENTRY, SKIP
	diroffset=dir->CurrentOffset-32;
//	printf("last entry\n");
	// FOUND LAST ENTRY OF A NAME
	nentries=buffer[0]&0x3f;
	morebuff=(char *)malloc(32*nentries);
	if(morebuff==NULL) {
		free(newname);
		free(shname);
		free(entryname);
		if(cr->Entry) free(cr->Entry); 
		return FS_ERROR;
	}
	if( (nbytesread=FSReadLL(morebuff,32*nentries,dir,fs))!=32*nentries) { 
		free(morebuff);
		if(nbytesread>0) cr->DirUsedEntries+=nbytesread>>5;

		if(FSEof(dir)) break;
		free(newname);
		free(shname);
		free(entryname);
		if(cr->Entry) free(cr->Entry); 
		return FS_ERROR;
	}
	
	cr->DirUsedEntries+=nentries;

//	printf("Read %d entries\n",nentries);
//	keyb_getkeyM(1);

//	printf("Checking validity\n");
	// VERIFY THAT ENTRIES ARE VALID
	ptr=morebuff;
	for(order=nentries-1;(order!=0)&& ((*ptr&0x3f)==order) ;--order,ptr+=32)
	{
		if(ptr[13]!=buffer[13]) break;		// VERIFY CHECKSUM
	}
//	printf("entries valid\n");
	if(order) {
//		printf("failed entries checksum test\n");
//		keyb_getkeyM(1);
		// ENTRIES ARE ORPHANS, DISCARD AND CONTINUE SEARCHING
		FSSeek(dir,-32*(order+1),FSSEEK_CUR);		// REWIND TO NEXT UNKNOWN ENTRY
		cr->DirUsedEntries-=order+1;
		free(morebuff);
		continue;
	}
	// VERIFY THAT SHORT ENTRY FOLLOWS LONG NAME
	
	if( ((ptr[11]&FSATTR_LONGMASK) == FSATTR_LONGNAME) || (*ptr==0) || (*ptr==0xe5)) {
//	printf("no valid shortname follows\n");
//	keyb_getkeyM(1);

	// VALID SHORT ENTRY NOT FOUND
	free(morebuff);
	if(*ptr==0) { cr->DirUsedEntries--; break; }
	if(*ptr!=0xe5) { FSSeek(dir,-32,FSSEEK_CUR);		// REWIND LAST ENTRY
					cr->DirUsedEntries--;
	}
	continue;
	}
	
//	printf("calculating checksum\n");
	// CALCULATE CHECKSUM
	checksum=0;
	for(order=0;order<11;++order,++ptr)
	{
//	printf("%c",*ptr);
	checksum= (((checksum<<7)&0x80) | ((checksum>>1)&0x7f)) + *ptr;
	}
	
//	printf("Calc. checksum=%02X\n",checksum);
	if(checksum!=buffer[13]) {
	// FAILED CHECKSUM, SKIP ORPHANS AND CONTINUE
//	printf("failed checksum\n");
//	keyb_getkeyM(1);
	free(morebuff);
	FSSeek(dir,-32,FSSEEK_CUR);		// REWIND LAST ENTRY
	--cr->DirUsedEntries;
	continue;
	}
//	printf("All valid!!!\n");
	
	// VALID ENTRY FOUND, FILL STRUCTURE AND RETURN

	cr->DirValidEntries+=nentries+1;

	ptr-=11;
	// REPACK LONG NAME
	for(order=1;order<nentries;++order)
	{
	FSPackName(entryname+13*(order-1),ptr-(order<<5));
	}
	FSPackName(entryname+13*(order-1),buffer);
	entryname[13*nentries]=0;		// FORCE NULL-TERMINATED STRING

	memcpy(buffer,ptr,32);		// COPY MAIN (SHORT) ENTRY TO buffer
	free(morebuff);
	


	// OBTAINED ENTRY, NOW COMPARE WITH NEW ENTRY
		FSPackShortName(shortn,buffer);


	// FIRST CHECK IF ENTRY IS THE FILE WE SEARCH FOR
	
	
		// CHECK IF LONG NAME MATCHES
		if(FSNameCompare(entryname,newname,(FSystem.CaseMode==FSCASE_SENS)? FSCASE_INSENS:FSystem.CaseMode) ||
		// CHECK IF SHORT NAME MATCHES
		FSNameCompare(shortn,newname,(FSystem.CaseMode==FSCASE_SENS)? FSCASE_INSENS:FSystem.CaseMode))

		{		// FILE EXISTS


		if(!cr->FileExists) {		// DON'T CHECK FOR DUPLICATED FILES
		cr->FileExists=TRUE;
		cr->Entry=(FS_FILE *)malloc(sizeof(FS_FILE));

		if(!cr->Entry) {
		free(newname);
		free(shname);
		free(entryname);
		return FS_ERROR;
		}

		cr->Entry->Name=(char *)malloc(strlen(entryname)+1);

		if(!cr->Entry->Name) {
		free(newname);
		free(shname);
		free(entryname);
		return FS_ERROR;
		}

		strcpy(cr->Entry->Name,entryname);
		if(FSystem.CaseMode==FSCASE_SENSHP) FSStripSemi(cr->Entry->Name);

		cr->Entry->Mode=0;
		cr->Entry->Volume=fs->VolNumber;
		cr->Entry->Attr=buffer[11];
		cr->Entry->NTRes=buffer[12];
		cr->Entry->CrtTmTenth=buffer[13];
		cr->Entry->LastAccDate=ReadInt16((char *)buffer+18);
		cr->Entry->CreatTimeDate=ReadInt32((char *)buffer+14);
		cr->Entry->WriteTimeDate=ReadInt32((char *)buffer+22);
		cr->Entry->FirstCluster=buffer[26]+(buffer[27]<<8)+(buffer[20]<<16)+(buffer[21]<<24);
		cr->Entry->FileSize=ReadInt32((char *)buffer+28);
		cr->Entry->CurrentOffset=0;
		cr->Entry->DirEntryOffset=diroffset;
		cr->Entry->DirEntryNum=nentries+1;
		cr->Entry->Dir=dir;
		memset((void *)&(cr->Entry->Chain),0,sizeof(FS_FRAGMENT));
		memset((void *)&(cr->Entry->RdBuffer),0,sizeof(FS_BUFFER));
		memset((void *)&(cr->Entry->WrBuffer),0,sizeof(FS_BUFFER));
		}

		}
		else {		// IF NOT EXACT MATCH, CHECK FOR APPROXIMATE MATCHES

	// CHECK FOR POSSIBLE CASE CONFLICT

			if(FSystem.CaseMode!=FSCASE_INSENS) {
				if(FSNameCompare(entryname,newname,(FSystem.CaseMode==FSCASE_SENS)? FSCASE_INSENS:FSCASE_INSENSHP)) {
					// CASE CONFLICT - DETERMINE HOW MANY SEMICOLONS TO ADD
					char *tmp=entryname;
					int semis=0;

					while(*tmp) ++tmp;
					--tmp;
					while( (*tmp==';') && (tmp>=entryname)) { tmp--; ++semis; }

					if(cr->NeedSemi<=semis) cr->NeedSemi=semis+1;

				}
			}


	// CHECK FOR POSSIBLE CASE CONFLICT (SHORT)
			if(FSystem.CaseMode!=FSCASE_INSENS) {
				if(FSNameCompare(shortn,newname,(FSystem.CaseMode==FSCASE_SENS)? FSCASE_INSENS:FSCASE_INSENSHP)) {
					// CASE CONFLICT - DETERMINE HOW MANY SEMICOLONS TO ADD
					if(!cr->NeedSemi) cr->NeedSemi=1;

				}
			}


	// CHECK FOR POSSIBLE ROOT/NUMBER CONFLICT
			{
				int rootn=FSNameCompareRoot(shortn,shname);
				if(cr->ShortNum<=rootn)
					cr->ShortNum=rootn+1;
			}



		}
		}
	
	else {
		FSPackShortName(shortn,buffer);
		
		diroffset=dir->CurrentOffset-32;

		++cr->DirValidEntries;


	// FIRST CHECK IF ENTRY IS THE FILE WE SEARCH FOR
	
	
		// CHECK IF SHORT NAME MATCHES
			if(FSNameCompare(shortn,newname,FSystem.CaseMode))

		{		// FILE EXISTS


		if(!cr->FileExists) {		// DON'T CHECK FOR DUPLICATED FILES
		cr->FileExists=TRUE;
		cr->Entry=(FS_FILE *)malloc(sizeof(FS_FILE));

		if(!cr->Entry) {
		free(newname);
		free(shname);
		free(entryname);
		return FS_ERROR;
		}

		cr->Entry->Name=(char *)malloc(strlen(shname)+1);

		if(!cr->Entry->Name) {
		free(newname);
		free(shname);
		free(entryname);
		return FS_ERROR;
		}

		strcpy(cr->Entry->Name,shname);
		if(FSystem.CaseMode==FSCASE_SENSHP) FSStripSemi(cr->Entry->Name);

		cr->Entry->Mode=0;
		cr->Entry->Volume=fs->VolNumber;
		cr->Entry->Attr=buffer[11];
		cr->Entry->NTRes=buffer[12];
		cr->Entry->CrtTmTenth=buffer[13];
		cr->Entry->LastAccDate=ReadInt16((char *)buffer+18);
		cr->Entry->CreatTimeDate=ReadInt32((char *)buffer+14);
		cr->Entry->WriteTimeDate=ReadInt32((char *)buffer+22);
		cr->Entry->FirstCluster=buffer[26]+(buffer[27]<<8)+(buffer[20]<<16)+(buffer[21]<<24);
		cr->Entry->FileSize=ReadInt32((char *)buffer+28);
		cr->Entry->CurrentOffset=0;
		cr->Entry->DirEntryOffset=diroffset;
		cr->Entry->DirEntryNum=1;
		cr->Entry->Dir=dir;
		memset((void *)&(cr->Entry->Chain),0,sizeof(FS_FRAGMENT));
		memset((void *)&(cr->Entry->RdBuffer),0,sizeof(FS_BUFFER));
		memset((void *)&(cr->Entry->WrBuffer),0,sizeof(FS_BUFFER));
		}

		}
		else {		// IF NOT EXACT MATCH, CHECK FOR APPROXIMATE MATCHES

	// CHECK FOR POSSIBLE CASE CONFLICT

			if(FSystem.CaseMode!=FSCASE_INSENS) {
				if(FSNameCompare(shortn,newname,(FSystem.CaseMode==FSCASE_SENS)? FSCASE_INSENS:FSCASE_INSENSHP)) {
					// CASE CONFLICT - DETERMINE HOW MANY SEMICOLONS TO ADD
					char *tmp=entryname;
					int semis=0;

					while(*tmp) ++tmp;
					--tmp;
					while( (*tmp==';') && (tmp>=entryname)) { tmp--; ++semis; }

					if(cr->NeedSemi<=semis) cr->NeedSemi=semis+1;

				}
			}


	// CHECK FOR POSSIBLE CASE CONFLICT (SHORT)
			if(FSystem.CaseMode!=FSCASE_INSENS) {
				if(FSNameCompare(shortn,newname,(FSystem.CaseMode==FSCASE_SENS)? FSCASE_INSENS:FSCASE_INSENSHP)) {
					// CASE CONFLICT - DETERMINE HOW MANY SEMICOLONS TO ADD
					if(!cr->NeedSemi) cr->NeedSemi=1;

				}
			}


	// CHECK FOR POSSIBLE ROOT/NUMBER CONFLICT
			{
				int rootn=FSNameCompareRoot(shortn,shname);
				if(cr->ShortNum<rootn) cr->ShortNum=rootn+1;
			}

		}
	}
	
}

if(cr->NeedSemi) cr->NameFlags|=3; // FORCE LONG NAME W/SHORT NAME TAIL


free(entryname);
free(shname);

// ANALIZE WHETHER DIRECTORY NEEDS REPACKING

if(cr->FileExists) {
free(newname);
	return FS_OK;	// DON'T REPACK IF FILE EXISTS, NO CREATION WILL BE DONE
}
// REPACK IF MORE THAN 100 ENTRIES TOTAL AND (BAD_ENTRIES>=VALID_ENTRIES)
if( ((cr->DirUsedEntries-cr->DirValidEntries>cr->DirValidEntries>>1) && (cr->DirUsedEntries>100))
   // ALSO REPACK IF BAD_ENTRIES>500
   || (cr->DirUsedEntries-cr->DirValidEntries>500)
   // ALSO REPACK IF LESS THAN 100 ENTRIES LEFT TO USE IN DIRECTORY
   || ((cr->DirUsedEntries-cr->DirValidEntries>0) && (cr->DirMaxEntries-cr->DirUsedEntries<100)))
{
int error=FSPackDir(dir);
if(error!=FS_OK) {
free(newname);
return error;
}
// DONE WITH PACKING

cr->DirUsedEntries=cr->DirValidEntries;
}


// CREATE ENTRY

		cr->Entry=(FS_FILE *)malloc(sizeof(FS_FILE));

		if(!cr->Entry) {
		free(newname);
		return FS_ERROR;
		}

		cr->Entry->Name=(char *)malloc(strlen(newname)+1+cr->NeedSemi);

		if(!cr->Entry->Name) {
		free(newname);
		return FS_ERROR;
		}

		strcpy((char *)cr->Entry->Name,(char *)newname);

		free(newname);

		// ADD SEMICOLONS

		entryname=cr->Entry->Name;

		while(*entryname) ++entryname;

		int f=cr->NeedSemi;
		while(f) {
		*entryname=';';
		++entryname;
		--f;
		}
		*entryname=0;

		cr->Entry->Volume=dir->Volume;
		cr->Entry->Mode=0;
		cr->Entry->Attr=0;
		cr->Entry->NTRes=0;
		FSGetDateTime(&cr->Entry->CreatTimeDate,&cr->Entry->WriteTimeDate);
		cr->Entry->CrtTmTenth=cr->Entry->WriteTimeDate;
		cr->Entry->LastAccDate=cr->Entry->CreatTimeDate>>16;
		cr->Entry->WriteTimeDate=cr->Entry->CreatTimeDate;
		cr->Entry->FirstCluster=0;
		cr->Entry->FileSize=0;
		cr->Entry->CurrentOffset=0;
		cr->Entry->DirEntryOffset=cr->DirUsedEntries<<5;	// DirEntryNum initialized later
		cr->Entry->Dir=dir;
		memset((void *)&(cr->Entry->Chain),0,sizeof(FS_FRAGMENT));
		memset((void *)&(cr->Entry->RdBuffer),0,sizeof(FS_BUFFER));
		memset((void *)&(cr->Entry->WrBuffer),0,sizeof(FS_BUFFER));

		// OTHERWISE RETURN WITH ALL INFORMATION COLLECTED
		


return FS_OK;

}