/* 
** NetXMS - Network Management System
** Driver for Symbol WS series wireless switches
** Copyright (C) 2013-2019 Raden Solutions
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** File: symbol-ws.h
**
**/

#ifndef _symbol_ws_h_
#define _symbol_ws_h_

#include <nddrv.h>


/**
 * Driver's class
 */
class SymbolDriver : public NetworkDeviceDriver
{
public:
   virtual const TCHAR *getName() override;
   virtual const TCHAR *getVersion() override;

   virtual int isPotentialDevice(const TCHAR *oid) override;
   virtual bool isDeviceSupported(SNMP_Transport *snmp, const TCHAR *oid) override;
   virtual int getClusterMode(SNMP_Transport *snmp, NObject *node, DriverData *driverData) override;
   virtual bool isWirelessController(SNMP_Transport *snmp, NObject *node, DriverData *driverData) override;
   virtual ObjectArray<AccessPointInfo> *getAccessPoints(SNMP_Transport *snmp, NObject *node, DriverData *driverData) override;
   virtual ObjectArray<WirelessStationInfo> *getWirelessStations(SNMP_Transport *snmp, NObject *node, DriverData *driverData) override;
   virtual AccessPointState getAccessPointState(SNMP_Transport *snmp, NObject *node, DriverData *driverData,
                                                UINT32 apIndex, const MacAddress& macAddr, const InetAddress& ipAddr,
						const ObjectArray<RadioInterfaceInfo> *radioInterfaces) override;
};

#endif
