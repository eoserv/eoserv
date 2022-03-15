/* extra/ntservice.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "ntservice.hpp"

#include "../fwd/socket.hpp" // OSErrorString
#include "../console.hpp"

#include <cstdlib>
#include <string>

#include "../eoserv_windows.h"

SERVICE_STATUS_HANDLE service_handle;
HANDLE service_event;

const char *service_name;

int service_state = SERVICE_RUNNING;

extern int eoserv_main(int argc, char *argv[]);
extern volatile bool eoserv_running;

void service_update_status(int state, int winexitcode, int servexitcode, int checkpoint, int waithint)
{
	SERVICE_STATUS service_status;
	service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	service_status.dwCurrentState = state;

	if (service_state == SERVICE_START_PENDING)
	{
		service_status.dwControlsAccepted = 0;
	}
	else
	{
		service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	}

	if (servexitcode == 0)
	{
		service_status.dwWin32ExitCode = winexitcode;
	}
	else
	{
		service_status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
	}

	service_status.dwServiceSpecificExitCode = servexitcode;
	service_status.dwCheckPoint = checkpoint;
	service_status.dwWaitHint = waithint;

	if (!SetServiceStatus(service_handle, &service_status))
	{
		Console::Err("Could not update service status: %s", OSErrorString());
		std::exit(1);
	}
}

void service_handler(int code)
{
	service_state = code;

	switch (code)
	{
		case SERVICE_CONTROL_SHUTDOWN:
		case SERVICE_CONTROL_STOP:
			service_update_status(SERVICE_STOPPED, NO_ERROR, 0, 0, 0);
			eoserv_running = false;
			return;
	}

	service_update_status(service_state, NO_ERROR, 0, 0, 0);
}

void service_main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	if (!(service_handle = RegisterServiceCtrlHandler(service_name, reinterpret_cast<LPHANDLER_FUNCTION>(service_handler))))
	{
		Console::Err("Could not register service handle: %s", OSErrorString());
		std::exit(1);
	}

	// Ugly hack to make sure EOSERV instances don't overlap
	service_update_status(SERVICE_START_PENDING, NO_ERROR, 0, 1, 6000);
	Sleep(5000);

	service_update_status(SERVICE_RUNNING, NO_ERROR, 0, 0, 0);

	eoserv_main(0, 0);
}

void service_init(const char *name)
{
	service_name = name;

	SERVICE_TABLE_ENTRY service_table[] = {
		{const_cast<char *>(name), reinterpret_cast<LPSERVICE_MAIN_FUNCTION>(service_main)},
		{0, 0}
	};

	StartServiceCtrlDispatcher(service_table);
}

bool service_install(const char *name)
{
	std::string file;
	char buf[MAX_PATH];

	GetModuleFileName(0, buf, MAX_PATH);

	file = buf;
	file += " service ";
	file += name;

	SC_HANDLE scm = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);
	SC_HANDLE service = CreateService(scm, name, name, 0, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, file.c_str(), 0, 0, 0, 0, 0);

	if (service != 0)
	{
		CloseServiceHandle(service);
		CloseServiceHandle(scm);
		return true;
	}
	else
	{
		CloseServiceHandle(scm);
		return false;
	}
}

bool service_uninstall(const char *name)
{
	SC_HANDLE scm = OpenSCManager(0, 0, 0);
	SC_HANDLE service = OpenService(scm, name, DELETE);

	if (service == 0)
	{
		CloseServiceHandle(scm);
		return false;
	}

	bool result = DeleteService(service);
	CloseServiceHandle(service);
	CloseServiceHandle(scm);
	return result;
}
