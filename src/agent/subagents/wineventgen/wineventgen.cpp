/*
** Windows Event Log generation NetXMS subagent
** Used only for tests
** Copyright (C) 2020 Raden Solutions
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
** File: wineventgen.cpp
**
**/

#include <nms_common.h>
#include <nms_agent.h>
#include <nms_threads.h>


/**
 * Request ID
 */
static int64_t s_requestId = static_cast<int64_t>(time(nullptr)) << 24;

//Default is 10484640 bytes of load (should be 10485760)
static int s_eventSize = 2427;
static int s_eventInterval = 20; //in sec
static int s_xmlSize = 543;
static const TCHAR *s_template = _T("<Event xmlns=\"http://schemas.microsoft.com/win/2004/08/events/event\">")
                        _T("<System>")
                        _T("<Provider Name=\"ESENT\" />")
                        _T("<EventID Qualifiers=\"0\">326</EventID>")
                        _T("<Version>0</Version>")
                        _T("<Level>4</Level>")
                        _T("<Task>1</Task>")
                        _T("<Opcode>0</Opcode>")
                        _T("<Keywords>0x80000000000000</Keywords>")
                        _T("<TimeCreated SystemTime=\"2020-10-26T08:26:55.6177145Z\" />")
                        _T("<EventRecordID>3908</EventRecordID>")
                        _T("<Correlation />")
                        _T("<Execution ProcessID=\"0\" ThreadID=\"0\" />")
                        _T("<Channel>Application</Channel>")
                        _T("<Computer>XPS13</Computer>")
                        _T("<Security />")
                        _T("</System>")
                        _T("<EventData>")
                        _T("<Data>%s</Data>")
                        _T("</EventData>")
                        _T("</Event>");

//Thread vars
static bool s_shutdown = false;
static Condition s_wakeup(false);

#define DEBUG_TAG _T("sa.wineventgen")

static NX_CFG_TEMPLATE s_cfgTemplate[] =
{
   { _T("Size"), CT_LONG, 0, 0, 0, 0, &s_eventSize, nullptr },
   { _T("Interval"), CT_LONG, 0, 0, 0, 0, &s_eventInterval, nullptr },
   { _T(""), CT_END_OF_LIST, 0, 0, 0, 0, nullptr, nullptr }
};

static TCHAR *generateText(int num, TCHAR *text)
{
   int i;
   for (i = 0; i < num; i++)
   {   
      int r = rand() % 26;
      text[i] = _T('a') + r; 
   }
   text[i]=0;
   return text;
}

THREAD_RESULT THREAD_CALL SenderThread(void *data)
{
   UINT32 wait = s_eventInterval*1000;
   int generatedTextSize = std::max(0, s_eventSize - s_xmlSize);
   int xmlSize = generatedTextSize + s_xmlSize;
   TCHAR *buffer = MemAllocString(xmlSize+1);
   TCHAR *text = MemAllocString(generatedTextSize+1);
   while (!s_shutdown)
   {
      int64_t id = s_requestId++;
      NXCPMessage *msg = new NXCPMessage(CMD_WINDOWS_EVENT, id);
      msg->setField(VID_REQUEST_ID, id);
      msg->setField(VID_LOG_NAME, _T("NetXMS"));
      msg->setFieldFromTime(VID_TIMESTAMP, time(nullptr));
      msg->setField(VID_EVENT_SOURCE, _T("NetXMS"));
      msg->setField(VID_SEVERITY, EVENTLOG_INFORMATION_TYPE);
      msg->setField(VID_EVENT_CODE, 105);
      generateText(generatedTextSize, text);
      msg->setField(VID_MESSAGE, text);   
      _sntprintf(buffer, xmlSize+1, s_template, text);
      msg->setField(VID_RAW_DATA, buffer);
      AgentQueueNotifictionMessage(msg);  // Takes ownership of message object
      nxlog_debug_tag(DEBUG_TAG, 5, _T("New log message sent"));

      s_wakeup.wait(wait);        
   }
   MemFree(buffer);
   MemFree(text);
   return THREAD_OK;
}

/**
 * Initialize subagent
 */
static bool SubAgentInit(Config *config)
{
   srand(time(nullptr));   
   nxlog_debug_tag(DEBUG_TAG, 5, _T("Start Windows event generator"));
   ThreadCreate(SenderThread, 0, NULL);
   return true;
}

/**
 * Handler for subagent unload
 */
static void SubAgentShutdown()
{
   nxlog_debug_tag(DEBUG_TAG, 5, _T("Stop Windows event generator"));
   s_shutdown = true;
   s_wakeup.set();
}

/**
 * Subagent information
 */
static NETXMS_SUBAGENT_INFO m_info =
{
   NETXMS_SUBAGENT_INFO_MAGIC,
   _T("WinEventGen"), NETXMS_VERSION_STRING,
   SubAgentInit, SubAgentShutdown, nullptr, nullptr,     // handlers
   0, nullptr,             // parameters
   0, nullptr,  //lists
   0, nullptr,	// tables
   0, nullptr,	// actions
   0, nullptr	// push parameters
};

/**
 * Entry point for NetXMS agent
 */
DECLARE_SUBAGENT_ENTRY_POINT(WINEVENTGEN)
{
   *ppInfo = &m_info;
   return true;
}
