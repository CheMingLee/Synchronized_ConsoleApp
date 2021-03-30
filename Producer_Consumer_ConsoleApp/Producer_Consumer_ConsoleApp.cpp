#include <windows.h>
#include <iostream>
#include <thread>

#define QUEUE_LENGTH 10 // buffer length

using namespace std;

HANDLE g_full_sem; // full semaphore
HANDLE g_empty_sem; // null semaphore

void GetCharToContinute()
{
	cout << "Press Enter to continute...\n";
	char ch;
	ch = getchar();
	cout << ch;
}

struct Msg
{
	int i;
}; // Message Structure

Msg g_MsgQueue[QUEUE_LENGTH]; // Buffer
int g_ihead = 0; // queue head
int g_itail = 0; // end of the queue

void CONSUMER()
{
	for (int i = 0; i < 1000; i++)
	{
		WaitForSingleObject(g_full_sem, INFINITE);
		int val = g_MsgQueue[g_ihead].i;
		g_ihead = (g_ihead + 1) % QUEUE_LENGTH;
		printf("CONSUMER : %d\n", val);
		ReleaseSemaphore(g_empty_sem, 1, NULL);
	}
}

void PRODUCER()
{
	for (int i = 0; i < 1000; i++)
	{
		WaitForSingleObject(g_empty_sem, INFINITE);
		g_MsgQueue[g_itail].i = i;
		g_itail = (g_itail + 1) % QUEUE_LENGTH;
		printf("PRODUCER : %d\n", i);
		ReleaseSemaphore(g_full_sem, 1, NULL);
	}
}

int main()
{
	g_full_sem = CreateSemaphore(NULL, 0, QUEUE_LENGTH, NULL);
	g_empty_sem = CreateSemaphore(NULL, QUEUE_LENGTH, QUEUE_LENGTH, NULL);
	
	thread tProducer(PRODUCER);
	thread tConsumer(CONSUMER);
	tProducer.join();
	tConsumer.join();

	GetCharToContinute();
	return 0;
}
