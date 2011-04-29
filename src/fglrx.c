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

#include "collectd.h"
#include "common.h"
#include "plugin.h"

extern int ADL_Main_Control_Create(void *, int);
extern int ADL_Adapter_NumberOfAdapters_Get(int *);
#define ADL_OK                                  0

/* Memory allocation function */
void* ADL_Main_Memory_Alloc ( int iSize )
{
    void* lpBuffer = malloc ( iSize );
    return lpBuffer;
}

static int fglrx_read(void)
{
	return(0);
}

void module_register (void)
{
	int iNumberAdapters;

	WARNING("about to init fglrx");
	if ( ADL_OK != ADL_Main_Control_Create((*ADL_Main_Memory_Alloc), 1)) {
		ERROR("unable to init fglrx");
		return;
	}
	if ( ADL_OK != ADL_Adapter_NumberOfAdapters_Get ( &iNumberAdapters )) {
		ERROR("unable to get number of adapters");
		return;
	}
	printf("Number of adapters: %d\n", iNumberAdapters);
	WARNING("Number of adapters: %d", iNumberAdapters);
	plugin_register_read ("load", fglrx_read);
} /* void module_register */
