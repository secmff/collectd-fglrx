/**
 * collectd - src/fglrx.c
 * Copyright (C) 2011       Mathijs Mohlmann
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; only version 2 of the License is applicable.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Authors:
 *     Mathijs Mohlmann
 **/

#define _BSD_SOURCE
#define LINUX

#include "collectd.h"
#include "common.h"
#include "plugin.h"
#include <adl_sdk.h>

extern int ADL_Main_Control_Create(void *, int);
extern int ADL_Adapter_NumberOfAdapters_Get(int *);
extern int ADL_Adapter_AdapterInfo_Get(LPAdapterInfo, int);
extern int ADL_Overdrive5_Temperature_Get(int, int, ADLTemperature *);
extern int ADL_Adapter_ID_Get(int, int *);

/* Memory allocation functions */
void* ADL_Main_Memory_Alloc ( int iSize )
{
	void* lpBuffer = malloc ( iSize );
	return lpBuffer;
}

void  ADL_Main_Memory_Free ( void ** lpBuffer )
{
	if ( NULL != *lpBuffer ) {
		free ( *lpBuffer );
		*lpBuffer = NULL;
	}
}

static int fglrx_read(void)
{
	return(0);
}

void module_register (void)
{
	int            iNumberAdapters, i;
	LPAdapterInfo  ldAdapterInfo;
	ADLTemperature lpTemperature;

	if ( ADL_OK != ADL_Main_Control_Create((*ADL_Main_Memory_Alloc), 1)) {
		ERROR("unable to init fglrx");
		return;
	}
	if ( ADL_OK != ADL_Adapter_NumberOfAdapters_Get ( &iNumberAdapters )) {
		ERROR("unable to get number of adapters");
		return;
	}
	if (iNumberAdapters <= 0) {
		WARNING("no fglrx adapters found");
		return;
	}
	printf("Number of adapters: %d\n", iNumberAdapters);

	ldAdapterInfo = (LPAdapterInfo) malloc(sizeof(AdapterInfo) * 
					       iNumberAdapters);
	if ( ADL_OK != ADL_Adapter_AdapterInfo_Get(
		     ldAdapterInfo,
		     sizeof(AdapterInfo) * iNumberAdapters)) {
		ERROR("unable to get adapter info");
		return;
	}

	for (i=0; i<iNumberAdapters; i++) {
		if ( ldAdapterInfo[i].iDrvIndex != 0 ) {
			continue;
		}

		printf("strAdapterName:%s (%d:%d.%d) -> %d\n",
		       ldAdapterInfo[i].strAdapterName,
		       ldAdapterInfo[i].iBusNumber,
		       ldAdapterInfo[i].iDeviceNumber,
		       ldAdapterInfo[i].iFunctionNumber,
		       ldAdapterInfo[i].iDrvIndex);
		if (ADL_OK != ADL_Overdrive5_Temperature_Get(i, 0,
							     &lpTemperature)) {
			printf("failed to get temp\n");
			continue;
		}
		printf("temp   :       %0.2f\n",
		       (float)lpTemperature.iTemperature/1000);
	}

	plugin_register_read ("load", fglrx_read);
} /* void module_register */
