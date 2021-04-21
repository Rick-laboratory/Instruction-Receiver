#pragma comment(lib, "Ws2_32.lib")
#include<WinSock2.h>
#include<iostream>
#include<conio.h>
#include<chrono>
#include "stdio.h"
#pragma warning(disable : 4996)
using namespace std;
#define PORT 9009

WSADATA WSA;
SOCKET Socket;
char Broadcast = '1';
int Errorcode =0;
struct sockaddr_in Addr_recv;
struct sockaddr_in Addr_sender;
char Buffer_recv[50];
int Buffer_len = 50;

HCURSOR createBlankCursor()
{
	int cw = GetSystemMetrics(SM_CXCURSOR);
	int ch = GetSystemMetrics(SM_CYCURSOR);
	UINT8* cursorAND = new UINT8[ch * ((cw + 31) >> 2)];
	UINT8* cursorXOR = new UINT8[ch * ((cw + 31) >> 2)];
	memset(cursorAND, 0xff, ch * ((cw + 31) >> 2));
	memset(cursorXOR, 0x00, ch * ((cw + 31) >> 2));
	HCURSOR c = CreateCursor(GetModuleHandle(NULL),
		0, 0, cw, ch, cursorAND, cursorXOR);
	delete[] cursorXOR;
	delete[] cursorAND;
	return c;
}
struct ChronoTimer
{
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<float> duration;
	ChronoTimer()
	{
		start = std::chrono::high_resolution_clock::now();
		
	}
	~ChronoTimer()
	{
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;
		float ms = duration.count() * 1000.0f;
		std::cout << "Timer took " << ms << "ms" << '\n';
		float fps = 1000 / ms;
		std::cout << "FPS:  " << fps << '\n';
	}

};

int main(void) {
	BOOL Success = SetProcessAffinityMask(GetCurrentProcess(), 0xF);
	Errorcode = WSAStartup(MAKEWORD(2, 2), &WSA);
	if (Errorcode != 0) {
		cout << "WSAStartup failed with Errorcode: " << Errorcode << "\n";
	}

	Socket = socket(AF_INET, SOCK_DGRAM, 0);

	if(setsockopt(Socket,
		SOL_SOCKET,
		SO_BROADCAST, &Broadcast, sizeof(Broadcast)) < 0) {
		cout << "Error in setting Broadcast option\n";
		closesocket(Socket);
	}

	int len = sizeof(struct sockaddr_in);

	char sendMessage[] = "Broadcast message from READER";

	Addr_recv.sin_family		= AF_INET;
	Addr_recv.sin_port			= htons(PORT);
	Addr_recv.sin_addr.s_addr	= INADDR_ANY;
	int instr[7];
	INPUT   Input = { 0 };
	bool control = false;
	if (bind(Socket,
		(sockaddr*)&Addr_recv, sizeof(Addr_recv)) < 0) {
		cout << "Error in binding" << WSAGetLastError();
		//system("PAUSE");
		closesocket(Socket);
	}
	while (true)
	{
		//ChronoTimer timer;
		recvfrom(Socket,
			Buffer_recv, Buffer_len, 0,
			(sockaddr*)&Addr_sender, &len);
		
		cout << "\n\n\n\tReceived Message is: " << Buffer_recv << "\n";
	    sscanf(Buffer_recv, "%d %d %d %d %d %d %d", &instr[0], &instr[1], &instr[2], &instr[3], &instr[4], &instr[5], &instr[6]);
		if (instr[5] == 1 ) {
			SetSystemCursor(createBlankCursor(), 32512);
			SetSystemCursor(createBlankCursor(), 32650);
			SetSystemCursor(createBlankCursor(), 32515);
			SetSystemCursor(createBlankCursor(), 32649);
			SetSystemCursor(createBlankCursor(), 32651);
			SetSystemCursor(createBlankCursor(), 32513);
			SetSystemCursor(createBlankCursor(), 32648);
			SetSystemCursor(createBlankCursor(), 32646);
			SetSystemCursor(createBlankCursor(), 32643);
			SetSystemCursor(createBlankCursor(), 32645);
			SetSystemCursor(createBlankCursor(), 32642);
			SetSystemCursor(createBlankCursor(), 32644);
			SetSystemCursor(createBlankCursor(), 32516);
			SetSystemCursor(createBlankCursor(), 32514);
			DestroyCursor(createBlankCursor());
			control = !control;
		}
		if (instr[5] == 2) {
			SystemParametersInfo(SPI_SETCURSORS, 0, NULL, 0);
		}
		if (instr[0] == 2) {
			SetCursorPos(instr[1], instr[2]);
		}		
		if (instr[0] == 0) {
			ZeroMemory(&Input, sizeof(INPUT));
			Input.type = INPUT_MOUSE;
			Input.mi.dwFlags = instr[3];
			::SendInput(1, &Input, sizeof(INPUT));
		}
		if (instr[0] == 1) {
			ZeroMemory(&Input, sizeof(INPUT));
			Input.type = INPUT_KEYBOARD;
			Input.ki.wVk = instr[3];
			if(instr[4]==1)
				Input.ki.dwFlags = KEYEVENTF_KEYUP;
			
			::SendInput(1, &Input, sizeof(INPUT));
		}
		if (instr[6] == 120) {
			ZeroMemory(&Input, sizeof(INPUT));
			Input.type = INPUT_MOUSE;
			Input.mi.dwFlags = MOUSEEVENTF_WHEEL;
			Input.mi.mouseData = 5*120;
			Input.mi.time = 0;
			SendInput(1, &Input, sizeof(INPUT));
			std::cout << "wheelup\n";
		}
		if (instr[6] == -120) {
			ZeroMemory(&Input, sizeof(INPUT));
			Input.type = INPUT_MOUSE;
			Input.mi.dwFlags = MOUSEEVENTF_WHEEL;
			Input.mi.mouseData = -120;
			Input.mi.time = 0;
			SendInput(1, &Input, sizeof(INPUT));
			std::cout << "wheeldown\n";
		}
	}



	//cout << "\n\n\n\tPress Any key to send message";
	//system("PAUSE");

	//if (sendto(Socket,
	//	sendMessage, strlen(sendMessage) + 1, 0,
	//	(sockaddr*)&Addr_sender, sizeof(Addr_sender)) < 0) {
	//	cout << "Error in Sending." << WSAGetLastError() << "\n";
	//	system("PAUSE");
	//	closesocket(Socket);
	//}

	//cout << "\n\n\n\tREADER send the Broadcast message Successfully \n";
	//system("PAUSE");

	closesocket(Socket);
	WSACleanup();
	return 0;
}