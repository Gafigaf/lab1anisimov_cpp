#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

class ProcessManager {
private:
    struct ProcessInfo {
        PROCESS_INFORMATION processInfo;
        STARTUPINFOW startupInfo;
        std::wstring commandLine;
        DWORD maxExecutionTime;
    };

    std::vector<ProcessInfo> processes;

public:
    ProcessManager() {}

    bool CreateNewProcess(const std::wstring& commandLine, DWORD maxExecutionTimeMs = 10000) {
        ProcessInfo procInfo;
        ZeroMemory(&procInfo.startupInfo, sizeof(procInfo.startupInfo));
        procInfo.startupInfo.cb = sizeof(procInfo.startupInfo);
        ZeroMemory(&procInfo.processInfo, sizeof(procInfo.processInfo));

        procInfo.commandLine = commandLine;
        procInfo.maxExecutionTime = maxExecutionTimeMs;

        std::vector<wchar_t> cmdLine(commandLine.begin(), commandLine.end());
        cmdLine.push_back(L'\0');

        BOOL success = CreateProcessW(
            NULL,           
            cmdLine.data(), 
            NULL,           
            NULL,          
            FALSE,          
            0,     
            NULL,     
            NULL,
            &procInfo.startupInfo,
            &procInfo.processInfo
        );

        if (!success) {
            std::wcerr << L"Process could not be created. Error code: " << GetLastError() << std::endl;
            return false;
        }

        processes.push_back(procInfo);
        return true;
    }

    void WaitForProcesses() {
        for (auto& proc : processes) {
            DWORD waitResult = WaitForSingleObject(
                proc.processInfo.hProcess,
                proc.maxExecutionTime
            );

            switch (waitResult) {
            case WAIT_OBJECT_0:
                HandleProcessCompletion(proc);
                break;
            case WAIT_TIMEOUT:
                HandleProcessTimeout(proc);
                break;
            default:
                std::wcerr << L"Error waiting for a process. Code: " << GetLastError() << std::endl;
            }
        }
    }

private:
    void HandleProcessCompletion(ProcessInfo& proc) {
        DWORD exitCode;
        if (GetExitCodeProcess(proc.processInfo.hProcess, &exitCode)) {
            std::wcout << L"Process \"" << proc.commandLine << L"\" is complete. Exit code: "
                << exitCode << std::endl;
        }

        CloseHandle(proc.processInfo.hProcess);
        CloseHandle(proc.processInfo.hThread);
    }

    void HandleProcessTimeout(ProcessInfo& proc) {
        std::wcout << L"Process \"" << proc.commandLine << L"\" has exceeded the time limit. Terminating..." << std::endl;

        TerminateProcess(proc.processInfo.hProcess, 1);

        CloseHandle(proc.processInfo.hProcess);
        CloseHandle(proc.processInfo.hThread);
    }
};

int main() {

    ProcessManager procManager;

    procManager.CreateNewProcess(L"notepad.exe");
    procManager.CreateNewProcess(L"calc.exe");
    procManager.WaitForProcesses();

    return 0;
}