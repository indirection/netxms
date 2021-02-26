/* 
** NetXMS - Network Management System
** Driver for Avaya ERS switches (except ERS 8xxx) (former Nortel/Bay Networks BayStack)
** Copyright (C) 2003-2011 Victor Kirhenshtein
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
** File: baystack.cpp
**/

#include "avaya.h"

/**
 * Get driver name
 */
const TCHAR *BayStackDriver::getName()
{
	return _T("BAYSTACK");
}

/**
 * Check if given device can be potentially supported by driver
 *
 * @param oid Device OID
 */
int BayStackDriver::isPotentialDevice(const TCHAR *oid)
{
	return (_tcsncmp(oid, _T(".1.3.6.1.4.1.45.3"), 17) == 0) ? 127 : 0;
}

/**
 * Check if given device is supported by driver
 *
 * @param snmp SNMP transport
 * @param oid Device OID
 */
bool BayStackDriver::isDeviceSupported(SNMP_Transport *snmp, const TCHAR *oid)
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
void BayStackDriver::analyzeDevice(SNMP_Transport *snmp, const TCHAR *oid, NObject *node, DriverData **driverData)
{
	UINT32 slotSize;
	
	if (!_tcsncmp(oid, _T(".1.3.6.1.4.1.45.3.74"), 20) ||	// 56xx
	    !_tcsncmp(oid, _T(".1.3.6.1.4.1.45.3.65"), 20))	// 5530-24TFD
	{
		slotSize = 128;
	}
	/* TODO: should OPtera Metro 1200 be there? */
	else if ((!_tcsncmp(oid, _T(".1.3.6.1.4.1.45.3.35"), 20)) ||	// BayStack 450
	         (!_tcsncmp(oid, _T(".1.3.6.1.4.1.45.3.40"), 20)) ||	// BPS2000
	         (!_tcsncmp(oid, _T(".1.3.6.1.4.1.45.3.43"), 20)))		// BayStack 420
	{
		slotSize = 32;
	}
	else
	{
		slotSize = 64;
	}

	node->setCustomAttribute(_T(".baystack.slotSize"), slotSize);

	UINT32 numVlans;
	if (SnmpGet(snmp->getSnmpVersion(), snmp, _T(".1.3.6.1.4.1.2272.1.3.1.0"), NULL, 0, &numVlans, sizeof(UINT32), 0) == SNMP_ERR_SUCCESS)
		node->setCustomAttribute(_T(".baystack.rapidCity.vlan"), numVlans);
	else
		node->setCustomAttribute(_T(".baystack.rapidCity.vlan"), (UINT32)0);
}

/**
 * Get slot size from object's custom attributes. Default implementation always return constant value 64.
 *
 * @param node object to get custom attribute
 * @return slot size
 */
UINT32 BayStackDriver::getSlotSize(NObject *node)
{
	return node->getCustomAttributeAsUInt32(_T(".baystack.slotSize"), 64);
}

/**
 * Get list of interfaces for given node
 *
 * @param snmp SNMP transport
 * @param node Node
 */
InterfaceList *BayStackDriver::getInterfaces(SNMP_Transport *snmp, NObject *node, DriverData *driverData, int useAliases, bool useIfXTable)
{
	// Get interface list from standard MIB
	InterfaceList *ifList = NetworkDeviceDriver::getInterfaces(snmp, node, driverData, useAliases, useIfXTable);
	if (ifList == NULL)
		return NULL;

   // Translate interface names 
	// TODO: does it really needed?
   for(int i = 0; i < ifList->size(); i++)
   {
		InterfaceInfo *iface = ifList->get(i);

		const TCHAR *ptr;
      if ((ptr = _tcsstr(iface->name, _T("- Port"))) != NULL)
		{
			ptr += 2;
         memmove(iface->name, ptr, _tcslen(ptr) + 1);
		}
      else if ((ptr = _tcsstr(iface->name, _T("- Unit"))) != NULL)
		{
			ptr += 2;
         memmove(iface->name, ptr, _tcslen(ptr) + 1);
		}
      else if ((_tcsstr(iface->name, _T("BayStack")) != NULL) ||
               (_tcsstr(iface->name, _T("Nortel Ethernet Switch")) != NULL))
      {
         ptr = _tcsrchr(iface->name, _T('-'));
         if (ptr != NULL)
         {
            ptr++;
            while(*ptr == _T(' '))
               ptr++;
            memmove(iface->name, ptr, _tcslen(ptr) + 1);
         }
      }
		StrStrip(iface->name);
   }
	
	// Calculate slot/port pair from ifIndex
	UINT32 slotSize = node->getCustomAttributeAsUInt32(_T(".baystack.slotSize"), 64);
	for(int i = 0; i < ifList->size(); i++)
	{
		UINT32 slot = ifList->get(i)->index / slotSize + 1;
		if ((slot > 0) && (slot <= 8))
		{
			ifList->get(i)->location.module = slot;
			ifList->get(i)->location.port = ifList->get(i)->index % slotSize;
			ifList->get(i)->isPhysicalPort = true;
		}
	}

	if (node->getCustomAttributeAsUInt32(_T(".baystack.rapidCity.vlan"), 0) > 0)
		getVlanInterfaces(snmp, ifList);

	UINT32 mgmtIpAddr, mgmtNetMask;
	if ((SnmpGet(snmp->getSnmpVersion(), snmp, _T(".1.3.6.1.4.1.45.1.6.4.2.2.1.2.1"), NULL, 0, &mgmtIpAddr, sizeof(UINT32), 0) == SNMP_ERR_SUCCESS) &&
	    (SnmpGet(snmp->getSnmpVersion(), snmp, _T(".1.3.6.1.4.1.45.1.6.4.2.2.1.3.1"), NULL, 0, &mgmtNetMask, sizeof(UINT32), 0) == SNMP_ERR_SUCCESS))
	{
		// Get switch base MAC address
		BYTE baseMacAddr[MAC_ADDR_LENGTH];
		SnmpGet(snmp->getSnmpVersion(), snmp, _T(".1.3.6.1.4.1.45.1.6.4.2.2.1.10.1"), NULL, 0, baseMacAddr, MAC_ADDR_LENGTH, SG_RAW_RESULT);

		if (mgmtIpAddr != 0)
		{
			int i;

			// Add management virtual interface if management IP is missing in interface list
			for(i = 0; i < ifList->size(); i++)
         {
            if (ifList->get(i)->hasAddress(mgmtIpAddr))
					break;
         }
			if (i == ifList->size())
			{
            InterfaceInfo *iface = new InterfaceInfo(0);
            iface->ipAddrList.add(InetAddress(mgmtIpAddr, mgmtNetMask));
				_tcscpy(iface->name, _T("MGMT"));
				memcpy(iface->macAddr, baseMacAddr, MAC_ADDR_LENGTH);
				ifList->add(iface);
			}
		}

		// Update wrongly reported MAC addresses
		for(int i = 0; i < ifList->size(); i++)
		{
			InterfaceInfo *curr = ifList->get(i);
			if ((curr->location.module != 0) &&
				 (!memcmp(curr->macAddr, "\x00\x00\x00\x00\x00\x00", MAC_ADDR_LENGTH) ||
			     !memcmp(curr->macAddr, baseMacAddr, MAC_ADDR_LENGTH)))
			{
				memcpy(curr->macAddr, baseMacAddr, MAC_ADDR_LENGTH);
				curr->macAddr[5] += (BYTE)curr->location.port;
			}
		}
	}

	return ifList;
}
