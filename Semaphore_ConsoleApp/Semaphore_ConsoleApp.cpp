#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <thread>

#define MAX_SEM_COUNT 5

using namespace std;

HANDLE ghSemaphore;

void ThreadProducer()
{
    DWORD dwWaitResult;
    BOOL bContinue = TRUE;

    while (bContinue)
    {
        // Try to enter the semaphore gate.
        dwWaitResult = WaitForSingleObject(
            ghSemaphore,   // handle to semaphore
            0L);           // zero-second time-out interval

        switch (dwWaitResult)
        {
            // The semaphore object was signaled.
            case WAIT_OBJECT_0:
                // TODO: Perform task
                printf("Thread %d: wait succeeded\n", GetCurrentThreadId());
                bContinue = FALSE;
                // Simulate thread spending time on task
                Sleep(2);
                // Release the semaphore when task is finished
                ReleaseSemaphore(ghSemaphore, 1, NULL);
                break;

            // The semaphore was nonsignaled, so a time-out occurred.
            case WAIT_TIMEOUT:
                printf("Thread %d: wait timed out\n", GetCurrentThreadId());
                break;
        }
    }
}

void GetCharToContinute()
{
    cout << "Press Enter to continute...\n";
    char ch;
    ch = getchar();
    cout << ch;
}

int main(void)
{
    // Create a semaphore with initial and max counts of MAX_SEM_COUNT
    ghSemaphore = CreateSemaphore(
        NULL,           // default security attributes
        MAX_SEM_COUNT,  // initial count
        MAX_SEM_COUNT,  // maximum count
        NULL);          // unnamed semaphore

    if (ghSemaphore == NULL)
    {
        printf("CreateSemaphore error: %d\n", GetLastError());
        return 1;
    }

    // Create worker threads
    thread t1(ThreadProducer);
    thread t2(ThreadProducer);
    thread t3(ThreadProducer);
    thread t4(ThreadProducer);
    thread t5(ThreadProducer);
    thread t6(ThreadProducer);
    thread t7(ThreadProducer);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    t7.join();
    
    CloseHandle(ghSemaphore);
    GetCharToContinute();
    return 0;
}
