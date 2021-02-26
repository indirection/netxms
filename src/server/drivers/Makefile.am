# Copyright (C) 2004 NetXMS Team <bugs@netxms.org>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

DRIVERS = at avaya cambium cisco dell-pwc dlink extreme \
	hpe huawei ignitenet juniper mikrotik moxa net-snmp netonix \
	ping3 qtech-olt rittal saf symbol_ws tb ubnt westerstrand

pkglib_LTLIBRARIES = at.la avaya.la cambium.la cisco.la dell-pwc.la dlink.la extreme.la \
	hpe.la huawei.la ignitenet.la juniper.la mikrotik.la moxa.la net-snmp.la netonix.la \
	ping3.la qtech-olt.la rittal.la saf.la symbol_ws.la tb.la ubnt.la westerstrand.la

at_la_SOURCES = at.cpp
at_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
at_la_LDFLAGS = -module -avoid-version
at_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

avaya_la_SOURCES = avaya.cpp baystack.cpp ers8000.cpp ntws.cpp vlan-if.cpp
avaya_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
avaya_la_LDFLAGS = -module -avoid-version
avaya_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

cambium_la_SOURCES = cambium.cpp cnpilot.cpp epmp.cpp
cambium_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
cambium_la_LDFLAGS = -module -avoid-version
cambium_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

cisco_la_SOURCES = cat2900xl.cpp catalyst.cpp cisco.cpp esw.cpp generic.cpp nexus.cpp sb.cpp wlc.cpp
cisco_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
cisco_la_LDFLAGS = -module -avoid-version
cisco_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

dell_pwc_la_SOURCES = dell-pwc.cpp
dell_pwc_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
dell_pwc_la_LDFLAGS = -module -avoid-version
dell_pwc_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

dlink_la_SOURCES = dlink.cpp
dlink_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
dlink_la_LDFLAGS = -module -avoid-version
dlink_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

extreme_la_SOURCES = extreme.cpp
extreme_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
extreme_la_LDFLAGS = -module -avoid-version
extreme_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

hpe_la_SOURCES = h3c.cpp hpe.cpp hpsw.cpp procurve.cpp
hpe_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
hpe_la_LDFLAGS = -module -avoid-version
hpe_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

huawei_la_SOURCES = huawei.cpp optix.cpp
huawei_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
huawei_la_LDFLAGS = -module -avoid-version
huawei_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

ignitenet_la_SOURCES = ignitenet.cpp
ignitenet_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
ignitenet_la_LDFLAGS = -module -avoid-version
ignitenet_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

juniper_la_SOURCES = juniper.cpp netscreen.cpp
juniper_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
juniper_la_LDFLAGS = -module -avoid-version
juniper_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

mikrotik_la_SOURCES = mikrotik.cpp
mikrotik_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
mikrotik_la_LDFLAGS = -module -avoid-version
mikrotik_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

moxa_la_SOURCES = edr.cpp moxa.cpp
moxa_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
moxa_la_LDFLAGS = -module -avoid-version
moxa_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

net_snmp_la_SOURCES = net-snmp.cpp
net_snmp_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
net_snmp_la_LDFLAGS = -module -avoid-version
net_snmp_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

netonix_la_SOURCES = netonix.cpp
netonix_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
netonix_la_LDFLAGS = -module -avoid-version
netonix_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

ping3_la_SOURCES = ping3.cpp
ping3_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
ping3_la_LDFLAGS = -module -avoid-version
ping3_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

qtech_olt_la_SOURCES = qtech-olt.cpp
qtech_olt_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
qtech_olt_la_LDFLAGS = -module -avoid-version
qtech_olt_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

rittal_la_SOURCES = data.cpp rittal.cpp
rittal_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
rittal_la_LDFLAGS = -module -avoid-version
rittal_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

saf_la_SOURCES = integra-b.cpp saf.cpp
saf_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
saf_la_LDFLAGS = -module -avoid-version
saf_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

symbol_ws_la_SOURCES = symbol-ws.cpp
symbol_ws_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
symbol_ws_la_LDFLAGS = -module -avoid-version
symbol_ws_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

tb_la_SOURCES = tb.cpp
tb_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
tb_la_LDFLAGS = -module -avoid-version
tb_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

ubnt_la_SOURCES = ubnt.cpp
ubnt_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
ubnt_la_LDFLAGS = -module -avoid-version
ubnt_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

westerstrand_la_SOURCES = westerstrand.cpp
westerstrand_la_CPPFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src/server/include -I@top_srcdir@/build
westerstrand_la_LDFLAGS = -module -avoid-version
westerstrand_la_LIBADD = ../../libnetxms/libnetxms.la \
	../../snmp/libnxsnmp/libnxsnmp.la \
	../libnxsrv/libnxsrv.la

EXTRA_DIST = \
	at.h at.vcxproj at.vcxproj.filters \
	avaya.h avaya.vcxproj avaya.vcxproj.filters \
	cambium.h cambium.vcxproj cambium.vcxproj.filters \
	cisco.h cisco.vcxproj cisco.vcxproj.filters \
	dell-pwc.h dell-pwc.vcxproj dell-pwc.vcxproj.filters \
	dlink.h dlink.vcxproj dlink.vcxproj.filters \
	extreme.h extreme.vcxproj extreme.vcxproj.filters \
	hpe.h hpe.vcxproj hpe.vcxproj.filters \
	huawei.h huawei.vcxproj huawei.vcxproj.filters \
	ignitenet.h ignitenet.vcxproj ignitenet.vcxproj.filters \
	juniper.h juniper.vcxproj juniper.vcxproj.filters \
	mikrotik.h mikrotik.vcxproj mikrotik.vcxproj.filters \
	moxa.h moxa.vcxproj moxa.vcxproj.filters \
	net-snmp.h net-snmp.vcxproj net-snmp.vcxproj.filters \
	netonix.h netonix.vcxproj netonix.vcxproj.filters \
	ping3.h ping3.vcxproj ping3.vcxproj.filters \
	qtech-olt.h qtech-olt.vcxproj qtech-olt.vcxproj.filters \
	rittal.h rittal.vcxproj rittal.vcxproj.filters \
	saf.h saf.vcxproj saf.vcxproj.filters \
	symbol-ws.h symbol-ws.vcxproj symbol-ws.vcxproj.filters \
	tb.h tb.vcxproj tb.vcxproj.filters \
	ubnt.h ubnt.vcxproj ubnt.vcxproj.filters \
	westerstrand.h westerstrand.vcxproj westerstrand.vcxproj.filters

install-exec-hook:
	mkdir -p $(DESTDIR)$(pkglibdir)/ndd ; for _DRIVER in $(DRIVERS); do if test "x`uname -s`" = "xAIX"; then OBJECT_MODE=@OBJECT_MODE@ $(AR) x $(DESTDIR)$(pkglibdir)/$$_DRIVER.a $(DESTDIR)$(pkglibdir)/$$_DRIVER@SHLIB_SUFFIX@ ; rm -f $(DESTDIR)$(pkglibdir)/$$_DRIVER.a; fi ; mv -f $(DESTDIR)$(pkglibdir)/$$_DRIVER@SHLIB_SUFFIX@ $(DESTDIR)$(pkglibdir)/ndd/$$_DRIVER.ndd ; rm -f $(DESTDIR)$(pkglibdir)/$$_DRIVER.la; done
