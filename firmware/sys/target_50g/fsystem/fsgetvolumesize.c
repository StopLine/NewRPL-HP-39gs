/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"


int FSGetVolumeSize(int Volnumber)
{
int error=FSInit();
if(error!=FS_OK) return error;

if(FSystem.Volumes[Volnumber]==NULL) return 0;
return FSystem.Volumes[Volnumber]->TotalSectors << FSystem.Volumes[Volnumber]->SectorSize;

}