/*
** NetXMS multiplatform core agent
** Copyright (C) 2003-2021 Victor Kirhenshtein
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
** File: exec.cpp
**
**/

#include "nxagentd.h"

#ifdef _WIN32
#include <winternl.h>
#include <WtsApi32.h>
#define WTS_DEBUG_TAG   _T("wts")
#else
#include <sys/wait.h>
#endif

#define EXEC_DEBUG_TAG  _T("exec")

/**
 * Information for process starter
 */
struct PROCESS_START_INFO
{
	char *cmdLine;
	pid_t *pid;
	CONDITION condProcessStarted;
};

/**
 * Execute external command
 */
#ifndef _WIN32  /* unix-only hack */
static THREAD_RESULT THREAD_CALL Waiter(void *arg)
{
	pid_t pid = *((pid_t *)arg);
	waitpid(pid, NULL, 0);
	free(arg);
	return THREAD_OK;
}

static THREAD_RESULT THREAD_CALL Worker(void *arg)
{
	char *cmd = ((PROCESS_START_INFO *)arg)->cmdLine;
	if (cmd == NULL)
	{
		return THREAD_OK;
	}

	char *pCmd[128];
	char *pTmp = cmd;
	int state = 0;
	int nCount = 0;

	pCmd[nCount++] = pTmp;
	int nLen = strlen(pTmp);
	for (int i = 0; i < nLen; i++)
	{
		switch(pTmp[i])
		{
			case ' ':
				if (state == 0)
				{
					pTmp[i] = 0;
					if (pTmp[i + 1] != 0)
					{
						pCmd[nCount++] = pTmp + i + 1;
					}
				}
				break;
			case '"':
				state == 0 ? state++ : state--;

				memmove(pTmp + i, pTmp + i + 1, nLen - i);
				i--;
				break;
			case '\\':
				if (pTmp[i+1] == '"')
				{
					memmove(pTmp + i, pTmp + i + 1, nLen - i);
				}
				break;
			default:
				break;
		}
	}
	pCmd[nCount] = NULL;

	// DO EXEC
	pid_t p;
	p = fork();
	switch(p)
	{
		case -1: // error
			*((PROCESS_START_INFO *)arg)->pid = -1;
			ConditionSet(((PROCESS_START_INFO *)arg)->condProcessStarted);
			break;
		case 0: // child
			{
				int sd = open("/dev/null", O_RDWR);
				if (sd == -1)
				{
					sd = open("/dev/null", O_RDONLY);
				}
				close(0); close(1); close(2);
				dup2(sd, 0); dup2(sd, 1); dup2(sd, 2);
				close(sd);
				execv(pCmd[0], pCmd);
				_exit(127);
			}
			break;
		default: // parent
			{
				*((PROCESS_START_INFO *)arg)->pid = p;
				ConditionSet(((PROCESS_START_INFO *)arg)->condProcessStarted);
				pid_t *pp = (pid_t *)malloc(sizeof(pid_t));
				*pp = p;
				ThreadCreate(Waiter, 0, (void *)pp);
			}
			break;
	}

	free(cmd);

	return THREAD_OK;
}
#endif

UINT32 ExecuteCommand(TCHAR *pszCommand, const StringList *args, pid_t *pid)
{
   TCHAR *pszCmdLine, *sptr;
   UINT32 i, dwSize, dwRetCode = ERR_SUCCESS;

   nxlog_debug_tag(EXEC_DEBUG_TAG, 4, _T("ExecuteCommand: expanding command \"%s\""), pszCommand);

   // Substitute $1 .. $9 with actual arguments
   if (args != NULL)
   {
      dwSize = ((UINT32)_tcslen(pszCommand) + 1) * sizeof(TCHAR);
      pszCmdLine = (TCHAR *)malloc(dwSize);
		for(sptr = pszCommand, i = 0; *sptr != 0; sptr++)
			if (*sptr == _T('$'))
			{
				sptr++;
				if (*sptr == 0)
					break;   // Single $ character at the end of line
				if ((*sptr >= _T('1')) && (*sptr <= _T('9')))
				{
					int argNum = *sptr - _T('1');

					if (argNum < args->size())
					{
						int iArgLength;

						// Extend resulting line
						iArgLength = (int)_tcslen(args->get(argNum));
						dwSize += iArgLength * sizeof(TCHAR);
						pszCmdLine = (TCHAR *)realloc(pszCmdLine, dwSize);
						_tcscpy(&pszCmdLine[i], args->get(argNum));
						i += iArgLength;
					}
				}
				else
				{
					pszCmdLine[i++] = *sptr;
				}
			}
			else
			{
				pszCmdLine[i++] = *sptr;
			}
		pszCmdLine[i] = 0;
	}
   else
   {
      pszCmdLine = pszCommand;
   }

   nxlog_debug_tag(EXEC_DEBUG_TAG, 4, _T("ExecuteCommand: executing \"%s\""), pszCmdLine);
#if defined(_WIN32)
   STARTUPINFO si;
   PROCESS_INFORMATION pi;

   // Fill in process startup info structure
   memset(&si, 0, sizeof(STARTUPINFO));
   si.cb = sizeof(STARTUPINFO);
   si.dwFlags = 0;

   // Create new process
   if (!CreateProcess(NULL, pszCmdLine, NULL, NULL, FALSE,
                      CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
   {
      TCHAR buffer[1024];
      nxlog_debug_tag(EXEC_DEBUG_TAG, 1, _T("Unable to create process \"%s\" (%s)"), pszCmdLine, GetSystemErrorText(GetLastError(), buffer, 1024));
      dwRetCode = ERR_EXEC_FAILED;
   }
   else
   {
		if (pid != NULL)
		{
			HMODULE dllKernel32;

			dllKernel32 = LoadLibrary(_T("KERNEL32.DLL"));
			if (dllKernel32 != NULL)
			{
				DWORD (WINAPI *fpGetProcessId)(HANDLE);

				fpGetProcessId = (DWORD (WINAPI *)(HANDLE))GetProcAddress(dllKernel32, "GetProcessId");
				if (fpGetProcessId != NULL)
				{
					*pid = fpGetProcessId(pi.hProcess);
				}
				else
				{
					HMODULE dllNtdll;

					dllNtdll = LoadLibrary(_T("NTDLL.DLL"));
					if (dllNtdll != NULL)
					{
						NTSTATUS (WINAPI *pfZwQueryInformationProcess)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

						pfZwQueryInformationProcess = (NTSTATUS (WINAPI *)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG))GetProcAddress(dllNtdll, "ZwQueryInformationProcess");
						if (pfZwQueryInformationProcess != NULL)
						{
							PROCESS_BASIC_INFORMATION pbi;
							ULONG len;

							if (pfZwQueryInformationProcess(pi.hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), &len) == 0)
							{
								*pid = (pid_t)pbi.UniqueProcessId;
							}
						}
						FreeLibrary(dllNtdll);
					}
				}
				FreeLibrary(dllKernel32);
			}
		}

      // Close all handles
      CloseHandle(pi.hThread);
      CloseHandle(pi.hProcess);
   }
#else
	PROCESS_START_INFO pi;
	pid_t tempPid;

#ifdef UNICODE
	pi.cmdLine = MBStringFromWideString(pszCmdLine);
#else
	pi.cmdLine = strdup(pszCmdLine);
#endif
	pi.pid = (pid != NULL) ? pid : &tempPid;
	pi.condProcessStarted = ConditionCreate(TRUE);
	if (ThreadCreate(Worker, 0, &pi))
	{
		if (ConditionWait(pi.condProcessStarted, 5000))
		{
			if (*pi.pid == -1)
		   	dwRetCode = ERR_EXEC_FAILED;
		}
		else
		{
	   	dwRetCode = ERR_EXEC_FAILED;
		}
		ConditionDestroy(pi.condProcessStarted);
	}
	else
	{
   	dwRetCode = ERR_EXEC_FAILED;
	}
#endif

   // Cleanup
   if (args != NULL)
      free(pszCmdLine);

   return dwRetCode;
}

/**
 * Structure for passing data to popen() worker
 */
struct POPEN_WORKER_DATA
{
	int status;
	TCHAR *cmdLine;
   StringList values;
	CONDITION finished;
	CONDITION released;
};

/**
 * Worker thread for executing external parameter handler
 */
static THREAD_RESULT THREAD_CALL POpenWorker(void *arg)
{
	FILE *hPipe;
	POPEN_WORKER_DATA *data = (POPEN_WORKER_DATA *)arg;

	if ((hPipe = _tpopen(data->cmdLine, _T("r"))) != NULL)
	{
      data->status = SYSINFO_RC_SUCCESS;

      TCHAR value[32768];
      while(true)
      {
         TCHAR *ret = safe_fgetts(value, 32768, hPipe);
         if (ret == nullptr)
         {
            if (!feof(hPipe))
            {
               nxlog_debug_tag(EXEC_DEBUG_TAG, 4, _T("H_ExternalParameter/POpenWorker: worker thread pipe read error: %s"), _tcserror(errno));
               data->status = SYSINFO_RC_ERROR;
            }
            break;
         }

         nxlog_debug_tag(EXEC_DEBUG_TAG, 4, _T("H_ExternalParameter/POpenWorker: worker thread pipe read result: %p"), ret);
         RemoveTrailingCRLF(value);
         if (value[0] != 0)
         {
            data->values.add(value);
         }
      }
		_pclose(hPipe);
	}
	else
	{
      nxlog_debug_tag(EXEC_DEBUG_TAG, 1, _T("Unable to create process \"%s\" (%s)"), data->cmdLine, _tcserror(errno));
		data->status = SYSINFO_RC_ERROR;
	}

	// Notify caller that data is available and wait while caller
	// retrieves the data, then destroy object
	ConditionSet(data->finished);
	ConditionWait(data->released, INFINITE);
	ConditionDestroy(data->finished);
	ConditionDestroy(data->released);
	delete data;

	return THREAD_OK;
}

/**
 * Exec function for external (user-defined) parameters and lists
 */
LONG RunExternal(const TCHAR *pszCmd, const TCHAR *pszArg, StringList *value)
{
	TCHAR *pszCmdLine, szBuffer[1024];
	const TCHAR *sptr;
	int i, iSize, iStatus;

   nxlog_debug_tag(EXEC_DEBUG_TAG, 4, _T("RunExternal called for \"%s\" \"%s\""), pszCmd, pszArg);

   // Substitute $1 .. $9 with actual arguments
   iSize = (int)_tcslen(pszArg) * sizeof(TCHAR);  // we don't need _tcslen + 1 because loop starts from &pszArg[1]
   pszCmdLine = (TCHAR *)malloc(iSize);
   for(sptr = &pszArg[1], i = 0; *sptr != 0; sptr++)
      if (*sptr == _T('$'))
      {
         sptr++;
         if (*sptr == 0)
            break;   // Single $ character at the end of line
         if ((*sptr >= _T('1')) && (*sptr <= _T('9')))
         {
            if (AgentGetParameterArg(pszCmd, *sptr - '0', szBuffer, 1024))
            {
               int iArgLength;

               // Extend resulting line
               iArgLength = (int)_tcslen(szBuffer);
               iSize += iArgLength * sizeof(TCHAR);
               pszCmdLine = (TCHAR *)realloc(pszCmdLine, iSize);
               _tcscpy(&pszCmdLine[i], szBuffer);
               i += iArgLength;
            }
         }
         else
         {
            pszCmdLine[i++] = *sptr;
         }
      }
      else
      {
         pszCmdLine[i++] = *sptr;
      }
   pszCmdLine[i] = 0;
   nxlog_debug_tag(EXEC_DEBUG_TAG, 4, _T("RunExternal: command line is \"%s\""), pszCmdLine);

#if defined(_WIN32)
	if (*pszArg == _T('E'))
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		SECURITY_ATTRIBUTES sa;
		HANDLE hOutput;
      TCHAR szTempFile[MAX_PATH];

		// Create temporary file to hold process output
		GetTempPath(MAX_PATH - 1, szBuffer);
		GetTempFileName(szBuffer, _T("nx"), 0, szTempFile);
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;
		hOutput = CreateFile(szTempFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);
		if (hOutput != INVALID_HANDLE_VALUE)
		{
			// Fill in process startup info structure
			memset(&si, 0, sizeof(STARTUPINFO));
			si.cb = sizeof(STARTUPINFO);
			si.dwFlags = STARTF_USESTDHANDLES;
			si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
			si.hStdOutput = hOutput;
			si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

			// Create new process
			if (CreateProcess(NULL, pszCmdLine, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
			{
				// Wait for process termination and close all handles
				if (WaitForSingleObject(pi.hProcess, g_execTimeout) == WAIT_OBJECT_0)
				{
					// Rewind temporary file for reading
					SetFilePointer(hOutput, 0, NULL, FILE_BEGIN);

					// Read process output
               DWORD size = GetFileSize(hOutput, NULL);
               char *buffer = (char *)malloc(size + 1);
					ReadFile(hOutput, buffer, size, &size, NULL);
					buffer[size] = 0;

               char *line = strtok(buffer, "\n");
               while(line != nullptr)
               {
					   char *eptr = strchr(line, '\r');
					   if (eptr != nullptr)
						   *eptr = 0;
                  StrStripA(line);

                  if (line[0] != 0)
                  {
#ifdef UNICODE
                     value->addPreallocated(WideStringFromUTF8String(line));
#else
                     value->addPreallocated(MBStringFromUTF8String(line));
#endif
                  }
                  line = strtok(nullptr, "\n");
               }
					iStatus = SYSINFO_RC_SUCCESS;
				}
				else
				{
					// Timeout waiting for external process to complete, kill it
					TerminateProcess(pi.hProcess, 127);
					nxlog_write_tag(NXLOG_WARNING, EXEC_DEBUG_TAG, _T("Process \"%s\" killed because of execution timeout"), pszCmdLine);
					iStatus = SYSINFO_RC_ERROR;
				}

				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);
			}
			else
			{
            TCHAR buffer[1024];
            nxlog_debug_tag(EXEC_DEBUG_TAG, 1, _T("RunExternal: unable to create process \"%s\" (%s)"), pszCmdLine, GetSystemErrorText(GetLastError(), buffer, 1024));
				iStatus = SYSINFO_RC_ERROR;
			}

			// Remove temporary file
			CloseHandle(hOutput);
			DeleteFile(szTempFile);
		}
		else
		{
         TCHAR buffer[1024];
         nxlog_debug_tag(EXEC_DEBUG_TAG, 1, _T("RunExternal: unable to create temporary file to hold process output (%s)"),
               GetSystemErrorText(GetLastError(), buffer, 1024));
			iStatus = SYSINFO_RC_ERROR;
		}
	}
	else
	{
#endif
		{
			POPEN_WORKER_DATA *data;

			data = new POPEN_WORKER_DATA;
			data->cmdLine = pszCmdLine;
			data->finished = ConditionCreate(TRUE);
			data->released = ConditionCreate(TRUE);
			ThreadCreate(POpenWorker, 0, data);
		   nxlog_debug_tag(EXEC_DEBUG_TAG, 4, _T("RunExternal (shell exec): worker thread created"));
			if (ConditionWait(data->finished, g_execTimeout))
			{
				iStatus = data->status;
				if (iStatus == SYSINFO_RC_SUCCESS)
            {
               value->addAll(&data->values);
            }
			}
			else
			{
				// Timeout
			   nxlog_debug_tag(EXEC_DEBUG_TAG, 4, _T("RunExternal (shell exec): execution timeout"));
				iStatus = SYSINFO_RC_ERROR;
			}
			ConditionSet(data->released);	// Allow worker to destroy data
		   nxlog_debug_tag(EXEC_DEBUG_TAG, 4, _T("RunExternal (shell exec): execution status %d"), iStatus);
		}

#ifdef _WIN32
	}
#endif

   free(pszCmdLine);
   return iStatus;
}

/**
 * Handler function for external (user-defined) parameters
 */
LONG H_ExternalParameter(const TCHAR *cmd, const TCHAR *arg, TCHAR *value, AbstractCommSession *session)
{
   session->debugPrintf(4, _T("H_ExternalParameter called for \"%s\" \"%s\""), cmd, arg);
   StringList values;
   LONG status = RunExternal(cmd, arg, &values);
   if (status == SYSINFO_RC_SUCCESS)
   {
      ret_string(value, values.size() > 0 ? values.get(0) : _T(""));
   }
   return status;
}

/**
 * Handler function for external (user-defined) lists
 */
LONG H_ExternalList(const TCHAR *cmd, const TCHAR *arg, StringList *value, AbstractCommSession *session)
{
   session->debugPrintf(4, _T("H_ExternalList called for \"%s\" \"%s\""), cmd, arg);
   StringList values;
   LONG status = RunExternal(cmd, arg, &values);
   if (status == SYSINFO_RC_SUCCESS)
   {
      value->addAll(&values);
   }
   return status;
}

/**
 * Parse data for external table
 */
void ParseExternalTableData(const ExternalTableDefinition& td, const StringList& data, Table *table)
{
   int numColumns = 0;
   TCHAR **columns = SplitString(data.get(0), td.separator, &numColumns);
   for(int n = 0; n < numColumns; n++)
   {
      bool instanceColumn = false;
      for(int i = 0; i < td.instanceColumnCount; i++)
         if (!_tcsicmp(td.instanceColumns[i], columns[n]))
         {
            instanceColumn = true;
            break;
         }
      table->addColumn(columns[n], DCI_DT_INT, columns[n], instanceColumn);
      MemFree(columns[n]);
   }
   MemFree(columns);

   for(int i = 1; i < data.size(); i++)
   {
      table->addRow();
      int count = 0;
      TCHAR **values = SplitString(data.get(i), td.separator, &count);
      for(int n = 0; n < count; n++)
      {
         if (n < numColumns)
            table->setPreallocated(n, values[n]);
         else
            MemFree(values[n]);
      }
      MemFree(values);
   }
}

/**
 * Handler function for external (user-defined) tables
 */
LONG H_ExternalTable(const TCHAR *cmd, const TCHAR *arg, Table *value, AbstractCommSession *session)
{
   const ExternalTableDefinition *td = reinterpret_cast<const ExternalTableDefinition*>(arg);
   session->debugPrintf(4, _T("H_ExternalTable called for \"%s\" (separator=0x%04X mode=%c cmd=\"%s\""), cmd, td->separator, td->cmdLine[0], &td->cmdLine[1]);
   StringList output;
   LONG status = RunExternal(cmd, td->cmdLine, &output);
   if (status == SYSINFO_RC_SUCCESS)
   {
      if (output.size() > 0)
      {
         ParseExternalTableData(*td, output, value);
      }
      else
      {
         session->debugPrintf(4, _T("H_ExternalTable(\"%s\"): empty output from command"), cmd);
         status = SYSINFO_RC_ERROR;
      }
   }
   return status;
}

#ifdef _WIN32

/**
 * Log error for ExecuteInAllSessions
 */
static inline void ExecuteInAllSessionsLogError(const TCHAR *function, const TCHAR *message)
{
   TCHAR buffer[1024];
   nxlog_debug_tag(WTS_DEBUG_TAG, 4, _T("%s: %s (%s)"),
         function, message, GetSystemErrorText(GetLastError(), buffer, 1024));
}

/**
 * Execute given command in specific session
 */
bool ExecuteInSession(WTS_SESSION_INFO *session, TCHAR *command, bool allSessions)
{
   const TCHAR *function = allSessions ? _T("ExecuteInAllSessions") : _T("ExecuteInSession");
   nxlog_debug_tag(WTS_DEBUG_TAG, 7, _T("%s: attempting to execute command in session #%u (%s)"),
         function, session->SessionId, session->pWinStationName);
 
   bool success = false;
   HANDLE sessionToken;
   if (WTSQueryUserToken(session->SessionId, &sessionToken))
   {
      HANDLE primaryToken;
      if (DuplicateTokenEx(sessionToken, TOKEN_ALL_ACCESS, NULL, SecurityDelegation, TokenPrimary, &primaryToken))
      {
         STARTUPINFO si;
         memset(&si, 0, sizeof(si));
         si.cb = sizeof(si);
         si.lpDesktop = _T("winsta0\\default");
         PROCESS_INFORMATION pi;
         if (CreateProcessAsUser(primaryToken, NULL, command, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, _T("C:\\"), &si, &pi))
         {
            nxlog_debug_tag(WTS_DEBUG_TAG, 7, _T("%s: process created in session #%u (%s)"),
                  function, session->SessionId, session->pWinStationName);
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
            success = true;
         }
         else
         {
            ExecuteInAllSessionsLogError(function, _T("call to CreateProcessAsUser failed"));
         }
         CloseHandle(primaryToken);
      }
      else
      {
         ExecuteInAllSessionsLogError(function, _T("call to DuplicateTokenEx failed"));
      }
      CloseHandle(sessionToken);
   }
   else
   {
      ExecuteInAllSessionsLogError(function, _T("call to WTSQueryUserToken failed"));
   }
   return success;
}

/**
 * Execute given command for all logged in users
 */
bool ExecuteInAllSessions(const TCHAR *command)
{
   WTS_SESSION_INFO *sessions;
   DWORD sessionCount;
   if (!WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &sessions, &sessionCount))
   {
      ExecuteInAllSessionsLogError(_T("ExecuteInAllSessions"), _T("call to WTSEnumerateSessions failed"));
      return false;
   }

   TCHAR cmdLine[4096];
   _tcslcpy(cmdLine, command, 4096);

   bool success = true;
   for (DWORD i = 0; i < sessionCount; i++)
   {
      if (sessions[i].SessionId == 0)
         continue;
      ExecuteInSession(&sessions[i], cmdLine, true);
   }

   WTSFreeMemory(sessions);
   return success;
}

#endif
