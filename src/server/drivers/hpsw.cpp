/* 
** NetXMS - Network Management System
** Driver for HP switches with HH3C MIB support
** Copyright (C) 2003-2016 Victor Kirhenshtein
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
** File: hpsw.cpp
**/

#include "hpe.h"

/**
 * Get driver name
 */
const TCHAR *HPSwitchDriver::getName()
{
	return _T("HPSW");
}

/**
 * Get driver version
 */
const TCHAR *HPSwitchDriver::getVersion()
{
	return NETXMS_BUILD_TAG;
}

/**
 * Check if given device can be potentially supported by driver
 *
 * @param oid Device OID
 */
int HPSwitchDriver::isPotentialDevice(const TCHAR *oid)
{
	return (_tcsncmp(oid, _T(".1.3.6.1.4.1.25506.11.1."), 24) == 0) ? 255 : 0;
}

/**
 * Check if given device is supported by driver
 *
 * @param snmp SNMP transport
 * @param oid Device OID
 */
bool HPSwitchDriver::isDeviceSupported(SNMP_Transport *snmp, const TCHAR *oid)
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
void HPSwitchDriver::analyzeDevice(SNMP_Transport *snmp, const TCHAR *oid, NObject *node, DriverData **driverData)
{
}

/**
 * Handler for port walk
 */
static UINT32 PortWalkHandler(SNMP_Variable *var, SNMP_Transport *snmp, void *arg)
{
   InterfaceList *ifList = (InterfaceList *)arg;
   UINT32 ifIndex = var->getValueAsUInt();
   for(int i = 0; i < ifList->size(); i++)
   {
      InterfaceInfo *iface = ifList->get(i);
      if (iface->index == ifIndex)
      {
         iface->isPhysicalPort = true;
         iface->location.chassis = var->getName().getElement(14);
         iface->location.module = var->getName().getElement(15);
         iface->location.pic = var->getName().getElement(16);
         iface->location.port = var->getName().getElement(17);
         break;
      }
   }
   return SNMP_ERR_SUCCESS;
}

/**
 * Get list of interfaces for given node
 *
 * @param snmp SNMP transport
 * @param node Node
 */
InterfaceList *HPSwitchDriver::getInterfaces(SNMP_Transport *snmp, NObject *node, DriverData *driverData, int useAliases, bool useIfXTable)
{
	// Get interface list from standard MIB
	InterfaceList *ifList = NetworkDeviceDriver::getInterfaces(snmp, node, driverData, useAliases, useIfXTable);
	if (ifList == NULL)
		return NULL;

	// Find physical ports (walk hh3cLswPortIfindex)
   SnmpWalk(snmp, _T(".1.3.6.1.4.1.25506.8.35.18.4.5.1.3"), PortWalkHandler, ifList);
	return ifList;
}
