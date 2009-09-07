/* 
** NetXMS - Network Management System
** Copyright (C) 2003-2009 Victor Kirhenshtein
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
** File: nxcore_logs.h
**
**/

#ifndef _nxcore_logs_h_
#define _nxcore_logs_h_


//
// Column types
//

#define LC_TEXT            0
#define LC_SEVERITY        1
#define LC_OBJECT_ID       2
#define LC_USER_ID         3
#define LC_EVENT_CODE      4
#define LC_TIMESTAMP       5


//
// Column filter types
//

#define FILTER_EQUALS      0
#define FILTER_RANGE       1
#define FILTER_SET         2
#define FILTER_LIKE        3


//
// Log definition structure
//

struct LOG_COLUMN
{
	const TCHAR *name;
	const TCHAR *description;
	int type;
};

struct NXCORE_LOG
{
	const TCHAR *name;
	const TCHAR *table;
	DWORD requiredAccess;
	LOG_COLUMN columns[32];
};


//
// Log filter
//

class ColumnFilter
{
private:
	int m_varCount;	// Number of variables read from NXCP message during construction
	int m_type;
	TCHAR *m_column;
	union t_ColumnFilterValue
	{
		INT64 equalsTo;
		struct
		{
			INT64 start;
			INT64 end;
		} range;
		TCHAR *like;
		struct
		{
			int operation;
			int count;
			ColumnFilter **filters;
		} set;
	} m_value;

public:
	ColumnFilter(CSCPMessage *msg, DWORD baseId);
	~ColumnFilter();

	int getVariableCount() { return m_varCount; }

	TCHAR *generateSql();
};

class LogFilter
{
private:
	int m_numColumnFilters;
	ColumnFilter **m_columnFilters;

public:
	LogFilter(CSCPMessage *msg);
	~LogFilter();

	int getNumColumnFilter()
	{
		return m_numColumnFilters;
	}

	ColumnFilter *getColumnFilter(int offset)
	{
		return m_columnFilters[offset];
	}
};


//
// Log handle - object used to access log
//

class LogHandle
{
private:
	NXCORE_LOG *m_log;
	bool m_isDataReady;
	TCHAR m_tempTable[64];
	INT64 m_rowCount;
	LogFilter *m_filter;
	MUTEX m_lock;
	DB_HANDLE m_dbHandle;
   TCHAR m_queryColumns[MAX_DB_STRING];

	void deleteQueryResults();

public:
	LogHandle(NXCORE_LOG *log);
	~LogHandle();

	void lock() { MutexLock(m_lock, INFINITE); }
	void unlock() { MutexUnlock(m_lock); }

	bool query(LogFilter *filter);
	Table *getData(INT64 startRow, INT64 numRows);
};


//
// API functions
//

void InitLogAccess();
int OpenLog(const TCHAR *name, ClientSession *session, DWORD *rcc);
DWORD CloseLog(ClientSession *session, int logHandle);
LogHandle *AcquireLogHandleObject(ClientSession *session, int logHandle);


#endif
