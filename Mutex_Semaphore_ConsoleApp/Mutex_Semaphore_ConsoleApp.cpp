#include <iostream>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <windows.h>

#define MAX_SEM_COUNT 10

using namespace std;

int g_count = 0;
mutex g_mutex;

CRITICAL_SECTION cs;
HANDLE ghMutex;
HANDLE ghSemaphore;

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
    // critical section (exclusive access to std::cout signaled by locking mtx):
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
    // critical section (exclusive access to std::cout signaled by locking mtx):
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
            __try {
                for (int i = 0; i < n; ++i)
                {
                    cout << c;
                    g_count++;
                }
                cout << '\n';

                cout << "count=" << g_count << endl;
            }

            __finally {
                // Release ownership of the mutex object
                if (!ReleaseMutex(ghMutex))
                {
                    // Handle error.
                }
            }
            break;

        // The thread got ownership of an abandoned mutex
        // The database is in an indeterminate state
        case WAIT_ABANDONED:
            break;
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
            ghSemaphore,   // handle to semaphore
            INFINITE);     // no time-out interval

        switch (dwWaitResult)
        {
            // The semaphore object was signaled.
            case WAIT_OBJECT_0:
                // TODO: Perform task
                bContinue = FALSE;
                for (int i = 0; i < n; ++i)
                {
                    cout << c;
                    g_count++;
                }
                cout << '\n';

                cout << "count=" << g_count << endl;

                // Release the semaphore when task is finished
                if (!ReleaseSemaphore(
                    ghSemaphore,  // handle to semaphore
                    1,            // increase count by one
                    NULL))       // not interested in previous count
                {
                    printf("ReleaseSemaphore error: %d\n", GetLastError());
                }
                break;

            // The semaphore was nonsignaled, so a time-out occurred.
            case WAIT_TIMEOUT:
                printf("Thread: wait timed out\n");
                break;
        }
    }
}

int main()
{
    cout << "No Synchronized:\n";
    thread t1(print, 10, 'A');
    thread t2(print, 5, 'B');
    t1.join();
    t2.join();

    cout << '\n';

    cout << "With std::mutex:\n";
    thread t3(print_mutex, 10, 'A');
    thread t4(print_mutex, 5, 'B');
    t3.join();
    t4.join();

    cout << '\n';

    cout << "With std::mutex (lock_guard):\n";
    thread t5(print_mutex_lock_guard, 10, 'A');
    thread t6(print_mutex_lock_guard, 5, 'B');
    t5.join();
    t6.join();

    cout << '\n';

    cout << "With Win32 API: CRITICAL_SECTION:\n";
    InitializeCriticalSection(&cs);
    thread t7(print_critical_section, 10, 'A');
    thread t8(print_critical_section, 5, 'B');
    t7.join();
    t8.join();
    DeleteCriticalSection(&cs);

    cout << '\n';

    cout << "With Win32 API: CreateMutex:\n";
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
    // Create a semaphore with initial and max counts of MAX_SEM_COUNT
    ghSemaphore = CreateSemaphore(
        NULL,           // default security attributes
        1L,  // initial count
        MAX_SEM_COUNT,  // maximum count
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
    
    cout << "Press Enter to continute...\n";
    char ch;
    ch = getchar();
    cout << ch;

    return 0;
}
