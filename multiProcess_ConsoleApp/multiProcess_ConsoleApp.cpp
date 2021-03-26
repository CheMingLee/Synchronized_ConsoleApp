#include <iostream>
#include <thread>
#include <windows.h>
#include <string>

using namespace std;

int main()
{
    HANDLE hMutex;

    hMutex = OpenMutex(
        MUTEX_ALL_ACCESS,            // request full access
        FALSE,                       // handle not inheritable
        TEXT("NameOfMutexObject"));  // object name

    if (hMutex == NULL)
        printf("OpenMutex error: %d\n", GetLastError());
    else printf("OpenMutex successfully opened the mutex.\n");

    cout << '\n';

    cout << "Press Enter to continute...\n";
    char ch;
    ch = getchar();
    cout << ch;

    HANDLE hFile;
    LPCTSTR szLogFilePath = L"D:\\Mutex_Semaphore\\Logger.log";
    
    hFile = CreateFileW(szLogFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        printf("errCode : %d", GetLastError());

    string msg = "Process B\n";

    while (true)
    {
        DWORD dwWaitResult;

        dwWaitResult = WaitForSingleObject(
            hMutex,    // handle to mutex
            INFINITE);  // no time-out interval

        switch (dwWaitResult)
        {
            // The thread got ownership of the mutex
        case WAIT_OBJECT_0:
            
            for (int i = 0; i < 5; i++)
            {
                SetFilePointer(hFile, 0, NULL, FILE_END);
                WriteFile(hFile, msg.c_str(), strlen(msg.c_str()), NULL, NULL);
            }

            // Release ownership of the mutex object
            if (!ReleaseMutex(hMutex))
            {
                // Handle error.
            }
            break;

            // The thread got ownership of an abandoned mutex
            // The database is in an indeterminate state
        case WAIT_ABANDONED:
            break;
        }
    }
    
    if (hFile != NULL)
    {
        CloseHandle(hFile);
        hFile = NULL;
    }
    
    if (hMutex != NULL)
    {
        CloseHandle(hMutex);
    }

    return 0;
}
