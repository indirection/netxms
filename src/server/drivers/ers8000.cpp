/* 
** NetXMS - Network Management System
** Driver for Avaya ERS 8xxx switches (former Nortel/Bay Networks Passport)
** Copyright (C) 2003-2018 Victor Kirhenshtein
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** File: ers8000.cpp
**/

#include "avaya.h"

/**
 * Get driver name
 */
const TCHAR *PassportDriver::getName()
{
	return _T("ERS8000");
}

/**
 * Check if given device can be potentially supported by driver
 *
 * @param oid Device OID
 */
int PassportDriver::isPotentialDevice(const TCHAR *oid)
{
	return (_tcsncmp(oid, _T(".1.3.6.1.4.1.2272"), 17) == 0) ? 127 : 0;
}

/**
 * Check if given device is supported by driver
 *
 * @param snmp SNMP transport
 * @param oid Device OID
 */
bool PassportDriver::isDeviceSupported(SNMP_Transport *snmp, const TCHAR *oid)
{
	return true;
}

/**
 * Do additional checks on the device required by driver.
 * Driver can set device's custom attributes from within
 * this function.
 *
 * @param snmp SNMP transport
 * @param node Node
 */
void PassportDriver::analyzeDevice(SNMP_Transport *snmp, const TCHAR *oid, NObject *node, DriverData **driverData)
{
	int model = _tcstol(&oid[18], NULL, 10);
	if ((model == 43) || (model == 44) || (model == 45))	// Passport 1600 series
	{
		node->setCustomAttribute(_T(".rapidCity.is1600"), _T("true"), StateChange::IGNORE);
		node->setCustomAttribute(_T(".rapidCity.maxSlot"), _T("1"), StateChange::IGNORE);
	}
	else if ((model == 31) || (model == 33) || (model == 48) || (model == 50))
	{
		node->setCustomAttribute(_T(".rapidCity.maxSlot"), _T("6"), StateChange::IGNORE);
	}
	else if (model == 34)
	{
		node->setCustomAttribute(_T(".rapidCity.maxSlot"), _T("3"), StateChange::IGNORE);
	}
}

/**
 * Get list of interfaces for given node
 *
 * @param snmp SNMP transport
 * @param node Node
 */
InterfaceList *PassportDriver::getInterfaces(SNMP_Transport *snmp, NObject *node, DriverData *driverData, int useAliases, bool useIfXTable)
{
	// Get interface list from standard MIB
	InterfaceList *ifList = AvayaERSDriver::getInterfaces(snmp, node, driverData, useAliases, useIfXTable);
	if (ifList == NULL)
		return NULL;

	UINT32 maxSlot = node->getCustomAttributeAsUInt32(_T(".rapidCity.maxSlot"), 10);
	bool is1600Series = node->getCustomAttributeAsBoolean(_T(".rapidCity.is1600"), false);
	
	// Calculate slot/port pair from ifIndex
	for(int i = 0; i < ifList->size(); i++)
	{
		UINT32 slot = ifList->get(i)->index / 64;

		// Some 1600 may report physical ports with indexes started at 1, not 64
		if ((slot == 0) && is1600Series)
		{
			ifList->get(i)->location.module = slot;
			ifList->get(i)->location.port = ifList->get(i)->index;
			ifList->get(i)->isPhysicalPort = true;
		}
		else if ((slot > 0) && (slot <= maxSlot))
		{
			ifList->get(i)->location.module = slot;
			ifList->get(i)->location.port = ifList->get(i)->index % 64 + 1;
			ifList->get(i)->isPhysicalPort = true;
		}
	}

	getVlanInterfaces(snmp, ifList);

	return ifList;
}
