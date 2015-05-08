#include <stdio.h>


typedef struct {
unsigned int FileSize;
unsigned int Reserved;
unsigned int Offset;
unsigned int HeaderSize;
unsigned int Width;
         int Height;
unsigned short Planes;
unsigned short BitsPixel;
unsigned int Compression;
unsigned int ImageSize;
unsigned int XPels_Meter;
unsigned int YPels_Meter;
unsigned int NumClr;
unsigned int ClrImp;
} BMP_Header;

#define MAX_GLYPHS 65536

// GET FONT INFORMATION

int width[MAX_GLYPHS];
int offset[MAX_GLYPHS];
int codeidx[0x110000];
char txtbuff[256*1024]; // MAX. 256K FOR THE TEXT FILE


int main(int argc,char *argv[])
{
if(argc<4) {
    printf("bmp2font 1.0\n------------\n\n");
    printf("Syntax: bmp2font <font-bitmap-file.bmp> <font-text-file.txt> <output.c>\n\n");
    return 0;
}
char *bmpfile=argv[1],*txtfile=argv[2],*outfile=argv[3];
char *bmpdata,*txtdata;

FILE *han=fopen(bmpfile,"rb");

if(!han) {
    printf("Unable to open file '%s'\n",bmpfile);
    return 1;
}

int bmpsize,invert;
BMP_Header hdr;
unsigned int Palette[256];



fseek(han,0,SEEK_END);
bmpsize=ftell(han);
fseek(han,2,SEEK_SET);
fread(&hdr,sizeof(BMP_Header),1,han);

if(hdr.BitsPixel!=8 || hdr.Planes!=1) {
    fclose(han);
    printf("Only grayscale or 256-color bitmaps supported\n");
    return 0;
}



fread(Palette,4,256,han);


if(hdr.Height<0) { hdr.Height=-hdr.Height; invert=0; }
else invert=1;


int rowwords=((hdr.Width*hdr.BitsPixel)/8+3)>>2;

bmpdata=malloc(rowwords*4*hdr.Height);

if(!bmpdata) {
fclose(han);
printf("Memory allocation error\n");
return 1;
}

fseek(han,hdr.Offset,SEEK_SET);
fread(bmpdata,4,rowwords*hdr.Height,han);
fclose(han);

printf("Bitmap read correctly\n");

// *********************************************************
// BEGIN FONT PROCESSING
// *********************************************************


// SCAN THE BITMAP FOR WIDTH AND OFFSET INFO

int x;
char *addr;
char change=0xaa;
int idx=0;
addr=bmpdata+(invert? 0:(rowwords*4*(hdr.Height-1)));

for(x=0;x<hdr.Width;++x) {
    if(addr[x]!=change) {
        if(idx>0) width[idx-1]=x-offset[idx-1];
        offset[idx]=x;
        ++idx;
        change=addr[x];
    }
}
if(idx>0) width[idx-1]=x-offset[idx-1];
//offset[idx]=x;
//++idx;

printf("Detected %d symbols in bitmap\n",idx);

// CONVERT THE BITMAP TO MONOCHROME

unsigned char *monobitmap=calloc( (hdr.Height-1),((hdr.Width+7)>>3));
if(!monobitmap) {
    printf("Memory allocation error.\n");
    free(bmpdata);
    return 1;
}




int color;
unsigned char byte;
unsigned char *ptr=monobitmap;
int y,ystart,yend,yinc,rowlen;

rowlen=(hdr.Width+7)>>3;

if(invert) { ystart=hdr.Height-1; yend=0; yinc=-1; }
else { ystart=0; yend=hdr.Height-1; yinc=1; }

for(y=ystart;y!=yend;y+=yinc) {
    addr=bmpdata+y*rowwords*4;
for(x=0;x<hdr.Width;++x)
{
    color=Palette[addr[x]];
    if(!color) {
        ptr[x>>3]|=1<<(x&7);
    }
}
ptr+=rowlen;
}

// FINISHED BITMAP CONVERSION


// READ THE GLYPHS CODES
han=fopen(txtfile,"rb");

if(!han) {
    printf("Unable to open file '%s'\n",txtfile);
    free(monobitmap);
    free(bmpdata);
    return 1;
}
int txtsize;
txtsize=fread(txtbuff,1,256*1024,han);
fclose(han);

// PROCESS THE TEXT FILE
char *txtend=txtbuff+txtsize;
char *lineend,*linestart;
addr=txtbuff;
int txtidx=0,usedline=0,usedflag;
int base,ndigits;

// RESET ALL UNICODE CHARACTERS TO POINT TO SYMBOL 0
for(y=0;y<0x110000;++y) codeidx[y]=0;

// SCAN THE FILE FOR CODES
while(addr<txtend) {

    lineend=addr;
    usedflag=0;
    while( (*lineend!='\n') && (lineend<txtend)) ++lineend;

    do {
        // SKIP ANY BLANKS
        while( ((*addr==' ')||(*addr=='\t')) && (addr<lineend) ) ++addr;

        if(addr>=txtend) break;

        if( (addr[0]=='/')|| (addr[0]=='\n')) {
            // DISCARD REST OF THE LINE
            addr=lineend;
            ++txtidx;
            continue;
        }

        base=10;
        if(addr[0]=='0') {
            if((addr[1]=='x')||(addr[1]=='X')) { base=16; addr+=2; }
            // TODO: ADD OTHER FORMATS HERE
        }
        // CONVERT THE NUMBER
        unsigned int result=0;
        int digit=-1;
        ndigits=0;

        do {

            if( (addr[0]>='0')&&(addr[0]<='9')) digit=addr[0]-'0';
            else {
                if(base==16) {
                    if( (addr[0]>='A')&&(addr[0]<='F')) digit=addr[0]-'A'+10;
                    else  if( (addr[0]>='a')&&(addr[0]<='f')) digit=addr[0]-'a'+10;
                }
            }

            if(digit<0) {
                if((addr[0]!=',')&&(addr[0]!='/')&&(addr[0]!='\n')&&(addr[0]!='\r')&&(addr[0]!=' ')&&(addr[0]!='\t'))
                {
                    printf("Syntax error in text file, line=%d\n",txtidx+1);
                    free(bmpdata);
                    free(monobitmap);
                    return 1;
                }
                if(ndigits) {
                    if( (result<0)||(result>0x10ffff)) {
                        printf("Number out of range in text file, line=%d\n",txtidx+1);
                        free(bmpdata);
                        free(monobitmap);
                        return 1;
                    }
                    codeidx[result]=usedline;
                    usedflag=1;
                }
                break;
            }
            else {
                result=result*base+digit;
                ++ndigits;
                digit=-1;
                ++addr;
            }

        } while(addr<lineend);

        // POSSIBLY SKIP BLANKS BEFORE A COMMA
        while( ((*addr==' ')||(*addr=='\t')) && (addr<lineend) ) ++addr;


    } while(*addr++==',');


    // END OF LINE REACHED
        if(usedflag) ++usedline;
        addr=lineend+1;
}

printf("Number of codes in text file: %d\n",usedline);

if(usedline!=idx) {
    int references[idx];

    for(y=0;y<idx;++y) references[y]=0;

    for(y=0;y<0x110000;++y) references[codeidx[y]]++;

    for(y=0;y<idx;++y) {
        if(references[y]==0) {
            printf("**Warning**: Symbol %d at X=%d is not referenced from text file (unused).\n",y,offset[y]);
        }

    }


}

// PACK AND SAVE THE DATA

// FONT STRUCTURE
/*
 * 4-BYTES PROLOG W/SIZE (COMPATIBLE WITH NEWRPL OBJECT FORMAT)
 * 4-BYTES:
 *          0xHHHHWWWW --> H=FONT HEIGHT, W=TOTAL BITMAP ROW WIDTH IN BYTES
 * 4-BYTES:
 *          2-BYTES: OFFSET IN WORDS FROM PROLOG TO WIDTH&OFFSET TABLE
 *          2-BYTES: OFFSET IN WORDS FROM PROLOG TO FONT BITMAP
 * ------ TABLE OF UNICODE->GLYPH MAPPING -----
 * 4-BYTE RANGES: 0xNNNNNOOO, WITH N=NUMBER OF CODES IN THIS RANGE, OOO=INDEX INTO WIDTH&OFFSET TABLE (0-4094, 4095 IS RESERVED FOR UNMAPPED)
 *                0xNNNNNFFF, WHEN OOO=0XFFF, THE ENTIRE RANGE OF CODES IS MAPPED TO INDEX 0 OF THE WIDTH&OFFSET TABLE.
 * REPEAT UNTIL IT COVERS THE ENTIRE UNICODE RANGE 0x110000
 *
 * ------ TABLE OF WIDTH & OFFSET
 * 2-BYTES VALUES 0xWOOO, WITH W=WIDTH IN PIXELS, OOO=x COORDINATE WITHIN THE BITMAP
 * REPEAT AS NEEDED
 *
 * ------ BITMAP
 * TOTAL NUMBER OF BYTES=HEIGHT*ROW WIDTH IN BYTES
 *
 * ----- PADDING FOR WORD-ALIGNMENT
*/


return 0;
}

