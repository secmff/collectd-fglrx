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
extern int ADL_Overdrive5_CurrentActivity_Get(int, ADLPMActivity *);
extern int ADL_Overdrive5_FanSpeed_Get(int, int, ADLFanSpeedValue *);
extern int ADL_Adapter_ID_Get(int, int *);

static LPAdapterInfo  ldAdapterInfo;
static int            iNumberAdapters;

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

static int fglrx_read_temperature(int i, value_list_t vl)
{
	ADLTemperature lpTemperature;

	sstrncpy (vl.type, "temperature", sizeof (vl.type));

	if (ADL_OK != ADL_Overdrive5_Temperature_Get(i, 0, &lpTemperature)) {
		WARNING("failed to get temperature for %d", i);
		return(1);
	}

	vl.values[0].gauge = (float)lpTemperature.iTemperature/1000;
	plugin_dispatch_values (&vl);
	return(0);
}

static int fglrx_read_performance(int i, value_list_t vl)
{
	ADLPMActivity lpActivity;
	sstrncpy (vl.type, "percent", sizeof (vl.type));
	if (ADL_OK != ADL_Overdrive5_CurrentActivity_Get(i, &lpActivity)) {
		WARNING("failed to get activity for %d", i);
		return(1);
	}
	vl.values[0].gauge = lpActivity.iActivityPercent;
	plugin_dispatch_values (&vl);
	return(0);
}

static int fglrx_read_fanspeed(int i, value_list_t vl)
{
	ADLFanSpeedValue lpFanSpeedValue;
	sstrncpy (vl.type, "fanspeed", sizeof (vl.type));
	lpFanSpeedValue.iSpeedType = ADL_DL_FANCTRL_SPEED_TYPE_RPM;
	if (ADL_OK != ADL_Overdrive5_FanSpeed_Get(i, 0, &lpFanSpeedValue)) {
		return(1);
	}
	vl.values[0].gauge = lpFanSpeedValue.iFanSpeed;
	plugin_dispatch_values (&vl);
	return(0);
}

static int fglrx_read(void)
{
	int i, adapter=0;
	value_t values[1];
	value_list_t vl = VALUE_LIST_INIT;
	char type_instance[DATA_MAX_NAME_LEN];

	vl.values = values;
	vl.values_len = 1;

	sstrncpy (vl.host, hostname_g, sizeof (vl.host));
	sstrncpy (vl.plugin, "fglrx", sizeof (vl.plugin));

	for (i=0; i<iNumberAdapters; i++) {
		if ( ldAdapterInfo[i].iDrvIndex != 0 ) {
			continue;
		}
		
		sprintf(type_instance, "Adapter/%d", adapter);
		sstrncpy (vl.type_instance, type_instance,
			  sizeof (vl.type_instance));
		fglrx_read_temperature(i, vl);
		fglrx_read_performance(i, vl);
		fglrx_read_fanspeed(i, vl);
		adapter++;
	}
	return(0);
}

void module_register (void)
{
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
	ldAdapterInfo = (LPAdapterInfo) malloc(sizeof(AdapterInfo) * 
					       iNumberAdapters);
	if ( ADL_OK != ADL_Adapter_AdapterInfo_Get(
		     ldAdapterInfo,
		     sizeof(AdapterInfo) * iNumberAdapters)) {
		ERROR("unable to get adapter info");
		return;
	}

	plugin_register_read ("load", fglrx_read);
} /* void module_register */
