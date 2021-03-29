#include <iostream>
#include <thread>
#include <windows.h>
#include <string>

using namespace std;

HANDLE g_hProcessMutex;
HANDLE g_hFile;

void GetCharToContinute()
{
    cout << "Press Enter to continute...\n";
    char ch;
    ch = getchar();
    cout << ch;
}

void WriteLog()
{
    LPCTSTR szLogFilePath = L"D:\\Mutex_Semaphore\\write_log.log";
    g_hFile = CreateFileW(szLogFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (g_hFile == INVALID_HANDLE_VALUE)
    {
        printf("errCode : %d", GetLastError());
    }
    
    string msg = "Process B\n";
    int iCnt = 0;
    while (iCnt < 300000)
    {
        for (int i = 0; i < 5; i++)
        {
            SetFilePointer(g_hFile, 0, NULL, FILE_END);
            WriteFile(g_hFile, msg.c_str(), strlen(msg.c_str()), NULL, NULL);
        }
        iCnt++;
    }

    if (g_hFile != NULL)
    {
        CloseHandle(g_hFile);
        g_hFile = NULL;
    }
}

void WriteLog_mutex()
{
    LPCTSTR szLogFilePath = L"D:\\Mutex_Semaphore\\write_log_mutex.log";
    g_hFile = CreateFileW(szLogFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (g_hFile == INVALID_HANDLE_VALUE)
    {
        printf("errCode : %d", GetLastError());
    }
    
    string msg = "Process B\n";
    int iCnt = 0;
    while (iCnt < 300000)
    {
        DWORD dwWaitResult;

        dwWaitResult = WaitForSingleObject(
            g_hProcessMutex,    // handle to mutex
            INFINITE);          // no time-out interval

        switch (dwWaitResult)
        {
            // The thread got ownership of the mutex
            case WAIT_OBJECT_0:

                for (int i = 0; i < 5; i++)
                {
                    SetFilePointer(g_hFile, 0, NULL, FILE_END);
                    WriteFile(g_hFile, msg.c_str(), strlen(msg.c_str()), NULL, NULL);
                }

                // Release ownership of the mutex object
                if (!ReleaseMutex(g_hProcessMutex))
                {
                    // Handle error.
                }
                break;

            // The thread got ownership of an abandoned mutex
            // The database is in an indeterminate state
            case WAIT_ABANDONED:
                break;
        }
        iCnt++;
    }

    if (g_hFile != NULL)
    {
        CloseHandle(g_hFile);
        g_hFile = NULL;
    }
}

int main()
{
    cout << "Wait Process A start: Write Log without synchronized\n";
    GetCharToContinute();
    WriteLog();
    cout << "Write Log without synchronized end\n";

    cout << '\n';

    cout << "Wait Process A CreateMutex then press enter to continue\n";
    GetCharToContinute();
    cout << "Process B start: OpenMutex\n";

    g_hProcessMutex = OpenMutex(
        MUTEX_ALL_ACCESS,           // request full access
        FALSE,                      // handle not inheritable
        TEXT("ProcessMutex"));      // object name

    if (g_hProcessMutex == NULL)
    {
        printf("OpenMutex error: %d\n", GetLastError());
        GetCharToContinute();
        return 0;
    }
    else
    {
        printf("OpenMutex successfully opened the mutex.\n");
        GetCharToContinute();
        printf("Start Process B\n");
        WriteLog_mutex();
        cout << "Write Log with mutex end\n";
        GetCharToContinute();
    }
    
    if (g_hProcessMutex != NULL)
    {
        CloseHandle(g_hProcessMutex);
    }

    return 0;
}
