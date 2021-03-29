#include <iostream>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <windows.h>

using namespace std;

int g_count = 0;
mutex g_mutex;

CRITICAL_SECTION cs;
HANDLE ghMutex;
HANDLE ghSemaphore;

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

    string msg = "Process A\n";
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
    
    string msg = "Process A\n";
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

void print(int n, char c)
{
    for (int i = 0; i < n; ++i)
    {
        cout << c;
        g_count++;
    }
    cout << '\n';

    cout << "count=" << g_count << endl;
}

void print_mutex(int n, char c)
{
    g_mutex.lock();
    for (int i = 0; i < n; ++i)
    {
        cout << c;
        g_count++;
    }
    cout << '\n';

    cout << "count=" << g_count << endl;
    g_mutex.unlock();
}

void print_mutex_lock_guard(int n, char c)
{
    lock_guard<mutex> lock(g_mutex);
    for (int i = 0; i < n; ++i)
    {
        cout << c;
        g_count++;
    }
    cout << '\n';

    cout << "count=" << g_count << endl;
}

void print_critical_section(int n, char c)
{
    // critical section
    EnterCriticalSection(&cs);
    for (int i = 0; i < n; ++i)
    {
        cout << c;
        g_count++;
    }
    cout << '\n';

    cout << "count=" << g_count << endl;
    LeaveCriticalSection(&cs);
}

void print_win32_mutex(int n, char c)
{
    // Request ownership of mutex.
    DWORD dwWaitResult;
    dwWaitResult = WaitForSingleObject(
        ghMutex,    // handle to mutex
        INFINITE);  // no time-out interval

    switch (dwWaitResult)
    {
        // The thread got ownership of the mutex
        case WAIT_OBJECT_0:
        {
            for (int i = 0; i < n; ++i)
            {
                cout << c;
                g_count++;
            }
            cout << '\n';
            cout << "count=" << g_count << endl;
            ReleaseMutex(ghMutex);
            break;
        }
        case WAIT_ABANDONED:
        {
            break;
        }
    }
}

void print_win32_semaphore(int n, char c)
{
    DWORD dwWaitResult;
    BOOL bContinue = TRUE;
    while (bContinue)
    {
        // Try to enter the semaphore gate.
        dwWaitResult = WaitForSingleObject(
            ghSemaphore,    // handle to semaphore
            0L);            // zero-second time-out interval

        switch (dwWaitResult)
        {
            // The semaphore object was signaled.
            case WAIT_OBJECT_0:
            {
                bContinue = FALSE;
                for (int i = 0; i < n; ++i)
                {
                    cout << c;
                    g_count++;
                }
                cout << '\n';
                cout << "count=" << g_count << endl;

                ReleaseSemaphore(
                    ghSemaphore,    // handle to semaphore
                    1,              // increase count by one
                    NULL);          // not interested in previous count
                break;
            }
            // The semaphore was nonsignaled, so a time-out occurred.
            case WAIT_TIMEOUT:
            {
                printf("Thread: wait timed out\n");
                break;
            } 
        }
    }
}

int main()
{
    cout << "No Synchronized:\n";
    g_count = 0;
    thread t1(print, 10, 'A');
    thread t2(print, 5, 'B');
    t1.join();
    t2.join();

    cout << '\n';

    cout << "With std::mutex:\n";
    g_count = 0;
    thread t3(print_mutex, 10, 'A');
    thread t4(print_mutex, 5, 'B');
    t3.join();
    t4.join();

    cout << '\n';

    cout << "With std::mutex (lock_guard):\n";
    g_count = 0;
    thread t5(print_mutex_lock_guard, 10, 'A');
    thread t6(print_mutex_lock_guard, 5, 'B');
    t5.join();
    t6.join();

    cout << '\n';

    cout << "With Win32 API: CRITICAL_SECTION:\n";
    g_count = 0;
    InitializeCriticalSection(&cs);
    thread t7(print_critical_section, 10, 'A');
    thread t8(print_critical_section, 5, 'B');
    t7.join();
    t8.join();
    DeleteCriticalSection(&cs);

    cout << '\n';

    cout << "With Win32 API: CreateMutex:\n";
    g_count = 0;
    // Create a mutex with no initial owner
    ghMutex = CreateMutex(
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex

    if (ghMutex == NULL)
    {
        printf("CreateMutex error: %d\n", GetLastError());
        return 1;
    }

    thread t9(print_win32_mutex, 10, 'A');
    thread t10(print_win32_mutex, 5, 'B');
    t9.join();
    t10.join();

    CloseHandle(ghMutex);

    cout << '\n';

    cout << "With Win32 API: CreateSemaphore:\n";
    g_count = 0;
    // Create a semaphore with initial and max counts of 1
    ghSemaphore = CreateSemaphore(
        NULL,           // default security attributes
        1L,             // initial count
        1L,             // maximum count
        NULL);          // unnamed semaphore

    if (ghSemaphore == NULL)
    {
        printf("CreateSemaphore error: %d\n", GetLastError());
        return 1;
    }

    thread t11(print_win32_semaphore, 10, 'A');
    thread t12(print_win32_semaphore, 5, 'B');
    t11.join();
    t12.join();

    CloseHandle(ghSemaphore);

    cout << '\n';
    
    cout << "Start the multi-process example\n";
    GetCharToContinute();

    cout << '\n';

    cout << "Write Log without synchronized start\n";
    cout << "Please start Process B then\n";
    GetCharToContinute();
    Sleep(1000);
    WriteLog();
    cout << "Write Log without synchronized end\n";

    cout << '\n';

    cout << "Write Log with mutex start\n";
    GetCharToContinute();

    g_hProcessMutex = CreateMutex(
        NULL,                       // request full access
        FALSE,                      // handle not inheritable
        TEXT("ProcessMutex"));      // object name

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        printf("CreateMutex opened an existing mutex\n");
        GetCharToContinute();
        return 0;
    }  
    else
    {
        printf("CreateMutex created a new mutex.\n");
        printf("Wait Process B OpenMutex\n");
        GetCharToContinute();
        printf("Start Process A\n");
        Sleep(1000);
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
