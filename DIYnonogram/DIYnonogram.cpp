#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <tchar.h>
#include <string.h>
#include <stdio.h>
#include <atlconv.h>
#include<memory.h>
#define random(n) (rand()%n)

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass = _T("DIY노노그램 0.1");
void DrawObject(HDC, RECT&, COLORREF, int);
void DrawObject(HDC, RECT&, COLORREF, COLORREF, int);
static COLORREF black_Color = RGB(0, 0, 0);
static COLORREF white_Color = RGB(255, 255, 255);
static RECT cell;
static RECT crt;
int initPoint = 0;//그려지기 시작하는 위치
int cellSize = 25; //셀크기
int x_Cell_Length = 10; //가로 셀 길이
int y_Cell_Length = 10; //세로 셀 길이
HWND createButton;
HWND playButton;
HWND returnMainButton;
HWND genCode;
HWND edit_In_Draw;
HWND edit_In_Main;
static int currentPlayTime;
static TCHAR gameTimerText[128] = _T("");
static int gameFlag = 1; //1이면 초기화면, 2이면 만들기, 3이면 플레이


int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpszCmdParam, _In_ int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	hWndMain = hWnd; // hWnd 정보도 전역변수에 저장!

	while (GetMessage(&Message, NULL, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return (int)Message.wParam;
}

enum Status { WHITE, BLACK, X };
struct tag_Cell {
	int Answer;
	Status St;
};
tag_Cell arCell[10][10];
int xNums[10][10] = { 0, };
int yNums[10][10] = { 0, };
int count;
enum { INTRO, MAKE, PLAY } GameStatus;

void InitGame();
void GenNums();
void DrawNums(HDC hdc);
void DrawScreen(HDC hdc, int gameFlag);
void GetTempFlip(int* tx, int* ty);
int CheckClear();
void GenCode();
int base64_encode(char* text, int numBytes, char** encoded);
int base64_decode(char* text, char* dst, int numBytes);

void DrawCell(HDC hdc, int x, int y, int cellSize, COLORREF inner_Color);
void DrawCell(HDC hdc, int x, int y, int cellSize, COLORREF inner_Color, bool XdrawFlag);

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage,
	WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	int nx, ny, i, j, tx, ty;



	switch (iMessage) {
	case WM_CREATE:
		hWndMain = hWnd;
		InitGame();
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case 1:
			//MessageBox(hWnd, _T("메인화면으로 클릭됨"), _T("버튼1"), MB_OK);
			gameFlag = 1;
			InitGame();
			break;
		case 2:
			//MessageBox(hWnd, _T("만들기 클릭됨"), _T("버튼1"), MB_OK);
			gameFlag = 2;
			InitGame();
			break;
		case 3:
			//MessageBox(hWnd, _T("플레이 클릭됨"), _T("버튼1"), MB_OK);
			char editword[256];
			TCHAR cellRowData[256];
			TCHAR* token;
			int x, y, cnt;
			char gamecode[256];
			char str[256];
			char* strBase64;


			if (GetWindowTextA(edit_In_Main, editword, GetWindowTextLengthA(edit_In_Main) + 1) != NULL) {
				editword[GetWindowTextLengthA(edit_In_Main) + 1] = '\0';
				base64_decode(editword, gamecode, strlen(editword));

				TCHAR* gamecodeTstr = new TCHAR[strlen(gamecode) + 1];
				memset(gamecodeTstr, '\0', sizeof(TCHAR) * (strlen(gamecode) + 1));

				MultiByteToWideChar(CP_ACP, 0, gamecode, strlen(gamecode) + 1, gamecodeTstr, strlen(gamecode));
				MessageBox(hWnd, gamecodeTstr, _T("알림"), MB_OK);
				if (_tcsrchr(gamecodeTstr, _T('|')) == NULL)
					MessageBox(hWnd, _T("게임코드가 올바르지 않습니다."), _T("알림"), MB_OK);
				else {
					gameFlag = 3;
					token = _tcstok(gamecodeTstr, _T("|"));
					x_Cell_Length = _ttoi(token); //가로 셀 길이
					token = _tcstok(NULL, _T("|")); // 다음 토큰
					y_Cell_Length = _ttoi(token); //세로 셀 길이
					token = _tcstok(NULL, _T("|")); // 다음 토큰
					_tcscpy(cellRowData, token);
					//MessageBox(hWnd, cellRowData, _T("알림"), MB_OK);
					cnt = 0;
					for (y = 0; y < x_Cell_Length; y++) {
						for (x = 0; x < y_Cell_Length; x++) {
							if (cellRowData[cnt] == '1') {
								arCell[x][y].Answer = 1;
							}
							else
								arCell[x][y].Answer = 0;
							cnt++;
						}
					}
					InitGame();
				}
			}
			else {
				MessageBox(hWnd, _T("게임코드를 입력해주세요."), _T("알림"), MB_OK);
			}

			break;
		case 101:
			//코드생성버튼
			GenCode();
			break;
		}
		return 0;
	case WM_LBUTTONDOWN:
		TCHAR str[100];

		switch (gameFlag) {
		case 1:
			break;
		case 2:
			//디버그용 코드
			//_stprintf_s(str, _T("x좌표: %d, y좌표: %d, x배열: %d, y배열:%d"), LOWORD(lParam) - initPoint, HIWORD(lParam) - initPoint, (LOWORD(lParam) - initPoint)/ cellSize, (HIWORD(lParam) - initPoint) / cellSize);
			//MessageBox(hWnd, str, _T("알림"), MB_OK);

			//셀 안에서만 작동하도록 제한
			if ((LOWORD(lParam) - initPoint) > 0 && (LOWORD(lParam) - initPoint) < cellSize * 10 && (HIWORD(lParam) - initPoint) > 0 && (HIWORD(lParam) - initPoint) < cellSize * 10) {
				if (arCell[(LOWORD(lParam) - initPoint) / cellSize][(HIWORD(lParam) - initPoint) / cellSize].St == WHITE)
					arCell[(LOWORD(lParam) - initPoint) / cellSize][(HIWORD(lParam) - initPoint) / cellSize].St = BLACK;
				else
					arCell[(LOWORD(lParam) - initPoint) / cellSize][(HIWORD(lParam) - initPoint) / cellSize].St = WHITE;
				InvalidateRect(hWnd, NULL, FALSE);
				GenCode();
			}
			break;
		case 3:
			if ((LOWORD(lParam) - initPoint) > 0 && (LOWORD(lParam) - initPoint) < cellSize * 10 && (HIWORD(lParam) - initPoint) > 0 && (HIWORD(lParam) - initPoint) < cellSize * 10) {
				if (arCell[(LOWORD(lParam) - initPoint) / cellSize][(HIWORD(lParam) - initPoint) / cellSize].St == WHITE)
					arCell[(LOWORD(lParam) - initPoint) / cellSize][(HIWORD(lParam) - initPoint) / cellSize].St = BLACK;
				else if (arCell[(LOWORD(lParam) - initPoint) / cellSize][(HIWORD(lParam) - initPoint) / cellSize].St == BLACK)
					arCell[(LOWORD(lParam) - initPoint) / cellSize][(HIWORD(lParam) - initPoint) / cellSize].St = WHITE;
				InvalidateRect(hWnd, NULL, FALSE);
			}
			if (CheckClear()) {
				KillTimer(hWndMain, 31);
				TCHAR clearText[128] = _T("");
				_stprintf_s(clearText, _T("축하합니다 클리어 하셨습니다!\n처음으로 돌아갑니다\n클리어 시간 : %02d:%02d:%02d"),
					int(currentPlayTime / 3600), int(currentPlayTime / 60) % 60, currentPlayTime % 60);
				MessageBox(hWnd, clearText, _T("알림"), MB_OK);
				gameFlag = 1;
				InitGame();
			}
			break;
		}

		/*
		nx = LOWORD(lParam) / 64;
		ny = HIWORD(lParam) / 64;
		if (GameStatus != RUN || nx > 3 || ny > 3 || arCell[nx][ny].St != HIDDEN) {
			return 0;
		}
		GetTempFlip(&tx, &ty);
		if (tx == -1) {
			arCell[nx][ny].St = TEMPFLIP;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else {
			count++;
			if (arCell[tx][ty].Num == arCell[nx][ny].Num) {
				MessageBeep(0);
				arCell[tx][ty].St = FLIP;
				arCell[nx][ny].St = FLIP;
				InvalidateRect(hWnd, NULL, FALSE);
				if (GetRemain() == 0) {
					MessageBox(hWnd, _T("축하합니다. 다시 시작합니다."), _T("알림"), MB_OK);
					InitGame();
				}
			}
			else {
				arCell[nx][ny].St = TEMPFLIP;
				InvalidateRect(hWnd, NULL, FALSE);
				GameStatus = VIEW;
				SetTimer(hWnd, 1, 100, NULL);
			}
		}*/
		return 0;
	case WM_RBUTTONDOWN:
		switch (gameFlag) {
		case 1:
			break;
		case 2:
			break;
		case 3:
			if ((LOWORD(lParam) - initPoint) > 0 && (LOWORD(lParam) - initPoint) < cellSize * 10 && (HIWORD(lParam) - initPoint) > 0 && (HIWORD(lParam) - initPoint) < cellSize * 10) {
				if (arCell[(LOWORD(lParam) - initPoint) / cellSize][(HIWORD(lParam) - initPoint) / cellSize].St != X)
					arCell[(LOWORD(lParam) - initPoint) / cellSize][(HIWORD(lParam) - initPoint) / cellSize].St = X;
				else
					arCell[(LOWORD(lParam) - initPoint) / cellSize][(HIWORD(lParam) - initPoint) / cellSize].St = WHITE;
				InvalidateRect(hWnd, NULL, FALSE);
			}
			break;
		}
		return 0;
	case WM_TIMER:
		switch (wParam) {
		case 31:
			currentPlayTime += 1;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		}
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWndMain, &ps);
		DrawScreen(hdc, gameFlag);
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}
void InitGame() {
	int i, j, x, y;
	count = 0;
	for (y = 0; y < 10; y++) {
		for (x = 0; x < 10; x++) {
			arCell[x][y].St = WHITE;
		}
	}
	switch (gameFlag) {
	case 1: //메인화면
		initPoint = 20;
		SetRect(&crt, 0, 0, 64 * 5 + 230, 64 * 6);
		AdjustWindowRect(&crt, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
		SetWindowPos(hWndMain, NULL, 0, 0, crt.right - crt.left, crt.bottom - crt.top, SWP_NOMOVE | SWP_NOZORDER);
		createButton = CreateWindow(_T("button"), _T("만들기"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			40, 40, 100, 25, hWndMain, (HMENU)2, g_hInst, NULL);
		playButton = CreateWindow(_T("button"), _T("플레이"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			150, 40, 100, 25, hWndMain, (HMENU)3, g_hInst, NULL);
		edit_In_Main = CreateWindow(_T("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
			20, 300, 200, 25, hWndMain, (HMENU)100, g_hInst, NULL);
		DestroyWindow(edit_In_Draw);
		DestroyWindow(genCode);
		DestroyWindow(returnMainButton);
		break;
	case 2: //만들기화면
		initPoint = 20;
		SetRect(&crt, 0, 0, 64 * 4 + 250, 350);
		AdjustWindowRect(&crt, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
		SetWindowPos(hWndMain, NULL, 0, 0, crt.right - crt.left, crt.bottom - crt.top, SWP_NOMOVE | SWP_NOZORDER);
		edit_In_Draw = CreateWindow(_T("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
			20, 300, 200, 25, hWndMain, (HMENU)100, g_hInst, NULL);
		genCode = CreateWindow(_T("button"), _T("코드생성"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			220, 300, 80, 25, hWndMain, (HMENU)101, g_hInst, NULL);

		returnMainButton = CreateWindow(_T("button"), _T("메인화면으로"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			300, 300, 120, 25, hWndMain, (HMENU)1, g_hInst, NULL);
		DestroyWindow(createButton);
		DestroyWindow(playButton);
		DestroyWindow(edit_In_Main);
		break;
	case 3: //플레이화면
		initPoint = 80;
		currentPlayTime = 0;
		SetTimer(hWndMain, 31, 1000, NULL);
		SetRect(&crt, 0, 0, 64 * 5 + 250, 400);
		AdjustWindowRect(&crt, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
		SetWindowPos(hWndMain, NULL, 0, 0, crt.right - crt.left, crt.bottom - crt.top, SWP_NOMOVE | SWP_NOZORDER);
		returnMainButton = CreateWindow(_T("button"), _T("메인화면으로"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			400, 300, 120, 25, hWndMain, (HMENU)1, g_hInst, NULL);
		DestroyWindow(createButton);
		DestroyWindow(playButton);
		DestroyWindow(edit_In_Main);
		GenNums();
		break;
	}
	InvalidateRect(hWndMain, NULL, FALSE);
}

void GenCode() {
	char str[256];
	char* strBase64;
	int strLen = 256;
	sprintf(str, "%d|%d|\0", x_Cell_Length, y_Cell_Length);
	//strcpy(str, "%d|%d|");
	//_stprintf_s(str, _T(), x_Cell_Length, y_Cell_Length);
	int x, y, num = strlen(str);;
	for (y = 0; y < 10; y++) {
		for (x = 0; x < 10; x++) {
			if (arCell[x][y].St == WHITE)
				str[num] = '0';
			if (arCell[x][y].St == BLACK)
				str[num] = '1';
			num++;
		}
	}
	str[num] = '\0';
	//https://dhna.tistory.com/22

	//WideCharToMultiByte(CP_ACP, 0, str, _tcslen(str), strChar, _tcslen(str), NULL, NULL);
	base64_encode(str, strlen(str), &strBase64);
	TCHAR* base64Tstr = new TCHAR[strlen(strBase64) + 1];
	memset(base64Tstr, '\0', sizeof(TCHAR) * (strlen(strBase64) + 1));

	MultiByteToWideChar(CP_ACP, 0, strBase64, strlen(strBase64) + 1, base64Tstr, strlen(strBase64));
	//_stprintf_s(str, _T("x좌표: %d, y좌표: %d, x배열: %d, y배열:%d"), LOWORD(lParam) - initPoint, HIWORD(lParam) - initPoint, (LOWORD(lParam) - initPoint) / cellSize, (HIWORD(lParam) - initPoint) / cellSize);
		//MessageBox(hWnd, str, _T("알림"), MB_OK);
	DestroyWindow(edit_In_Draw);
	edit_In_Draw = CreateWindow(_T("edit"), base64Tstr, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		20, 300, 200, 25, hWndMain, (HMENU)100, g_hInst, NULL);
	InvalidateRect(hWndMain, NULL, FALSE);
}




void DrawScreen(HDC hdc, int gameFlag) {
	int x, y, image;
	TCHAR Mes[128];
	switch (gameFlag) {
	case 1:
		lstrcpy(Mes, _T("DIY 노노그램"));
		TextOut(hdc, 50, 10, Mes, _tcslen(Mes));
		break;
	case 2:
		Rectangle(hdc, initPoint - 1, initPoint - 1, initPoint + cellSize * 10 + 1, initPoint + cellSize * 10 + 1); //바깥 테두리
		for (y = 0; y < 10; y++) {
			for (x = 0; x < 10; x++) {
				if (arCell[x][y].St == WHITE)
					DrawCell(hdc, x * cellSize + initPoint, y * cellSize + initPoint, cellSize, white_Color);
				if (arCell[x][y].St == BLACK)
					DrawCell(hdc, x * cellSize + initPoint, y * cellSize + initPoint, cellSize, black_Color);
			}
		}

		lstrcpy(Mes, _T("만들기"));//수정
		TextOut(hdc, 300, 10, Mes, _tcslen(Mes));//tcslen수정
		break;
	case 3:

		_stprintf_s(gameTimerText, _T("경과시간 %02d:%02d:%02d"),
			int(currentPlayTime / 3600), int(currentPlayTime / 60) % 60, currentPlayTime % 60);
		TextOut(hdc, 400, 350, gameTimerText, _tcslen(gameTimerText));

		Rectangle(hdc, initPoint - 1, initPoint - 1, initPoint + cellSize * 10 + 1, initPoint + cellSize * 10 + 1); //바깥 테두리
		for (y = 0; y < 10; y++) {
			for (x = 0; x < 10; x++) {
				if (arCell[x][y].St == WHITE)
					DrawCell(hdc, x * cellSize + initPoint, y * cellSize + initPoint, cellSize, white_Color);
				if (arCell[x][y].St == BLACK)
					DrawCell(hdc, x * cellSize + initPoint, y * cellSize + initPoint, cellSize, black_Color);
				if (arCell[x][y].St == X)
					DrawCell(hdc, x * cellSize + initPoint, y * cellSize + initPoint, cellSize, white_Color, true);
			}
		}
		DrawNums(hdc);

		lstrcpy(Mes, _T("플레이"));//수정
		TextOut(hdc, 400, 10, Mes, _tcslen(Mes));//tcslen수정
		break;

	}
}
void GenNums() {
	TCHAR str[256];
	int x, y, num;
	//초기화
	for (y = 0; y < 10; y++) {
		for (x = 0; x < 10; x++) {
			xNums[x][y] = 0;
			yNums[x][y] = 0;
		}
	}
	//9시방향 숫자들 생성
	for (y = 0; y < 10; y++) {
		num = 0;
		for (x = 9; x >= 0; x--) {

			if (arCell[x][y].Answer == 1) {
				if (xNums[y][num] == 0) {
					xNums[y][num] = 1;
				}
				else if (xNums[y][num] > 0) {
					xNums[y][num] += 1;
				}
			}
			else if (arCell[x][y].Answer == 0) {
				if (xNums[y][num] > 0) {
					num++;
				}
			}
		}
	}

	//12시방향 숫자들 생성
	for (y = 0; y < 10; y++) {
		num = 0;
		for (x = 9; x >= 0; x--) {
			if (arCell[y][x].Answer == 1) {
				if (yNums[y][num] == 0) {
					yNums[y][num] = 1;
				}
				else if (yNums[y][num] > 0) {
					yNums[y][num] += 1;
				}
			}
			else if (arCell[y][x].Answer == 0) {
				if (yNums[y][num] > 0) {
					num++;
				}
			}
		}
	}
	//_stprintf_s(str, _T("%d|%d|%d|%d"), xNums[0][0], xNums[0][1], xNums[0][2], xNums[0][3]);
	//MessageBox(hWndMain, str, _T("디버그"), MB_OK);
	//_stprintf_s(str, _T("%d|%d|%d|%d"), yNums[0][0], yNums[0][1], yNums[0][2], yNums[0][3]);
	//MessageBox(hWndMain, str, _T("디버그"), MB_OK);

}

void DrawNums(HDC hdc) {
	TCHAR str[10];
	int x, y, num;
	//9시방향 숫자들 그리기
	for (y = 0; y < 10; y++) {
		num = 0;
		for (x = 0; x < 10; x++) {
			if (xNums[y][x] >= 1) {
				_itow(xNums[y][x], str, 10);
				TextOut(hdc, (initPoint - 12) - (num * 16) - ((_tcslen(str) - 1) * 9), (initPoint + 3) + y * cellSize, str, _tcslen(str));
				num++;
			}
		}
		if (num == 0) {
			TextOut(hdc, (initPoint - 12) - (num * 9), (initPoint + 3) + y * cellSize, _T("0"), 1);
		}
	}

	//12시방향 숫자들 그리기
	for (y = 0; y < 10; y++) {
		num = 0;
		for (x = 0; x < 10; x++) {
			if (yNums[y][x] >= 1) {
				_itow(yNums[y][x], str, 10);
				TextOut(hdc, (initPoint + 8) - ((_tcslen(str) - 1) * 4) + y * cellSize, (initPoint - 18) - (num * 16), str, _tcslen(str));
				num++;
			}
		}
		if (num == 0) {
			TextOut(hdc, (initPoint + 8) + y * cellSize, (initPoint - 18) - (num * 16), _T("0"), 1);
		}
	}
}

int CheckClear() {
	int x, y;

	for (y = 0; y < 10; y++) {
		for (x = 0; x < 10; x++) {
			if (arCell[x][y].Answer == 1 && arCell[x][y].St == WHITE) {
				return 0; //정답의 셀이 칠해져있는데 셀이 하얀색이면 0 반환
			}
			if (arCell[x][y].Answer == 0 && arCell[x][y].St == BLACK) {
				return 0; //정답의 셀이 안칠해져있는데 셀이 검은색이면 0 반환
			}
		}
	}
	return 1;
}


void DrawCell(HDC hdc, int x, int y, int cellSize, COLORREF inner_Color) {
	DrawCell(hdc, x, y, cellSize, inner_Color, false);
}


void DrawCell(HDC hdc, int x, int y, int cellSize, COLORREF inner_Color, bool XdrawFlag) {

	HBRUSH myBrush = (HBRUSH)CreateSolidBrush(inner_Color);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, myBrush);

	Rectangle(hdc, x, y, x + cellSize, y + cellSize);

	if (XdrawFlag) {
		MoveToEx(hdc, x, y, NULL);
		LineTo(hdc, x + cellSize, y + cellSize);
		MoveToEx(hdc, x + cellSize, y, NULL);
		LineTo(hdc, x, y + cellSize);
	}
	SelectObject(hdc, oldBrush);
	DeleteObject(myBrush);

}



void DrawObject(HDC hdc, RECT& r, COLORREF color, int type)
{
	DrawObject(hdc, r, color, color, type);
}


void DrawObject(HDC hdc, RECT& r, COLORREF penC, COLORREF brushC, int type) {
	HPEN hPen, hOldPen;
	HBRUSH hBrush, hOldBrush;
	hPen = CreatePen(PS_SOLID, 1, penC);
	hOldPen = (HPEN)SelectObject(hdc, hPen);
	hBrush = CreateSolidBrush(brushC);
	hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

	switch (type) {		// type이 0: 사각형, 1: 원
	case 0:
		Rectangle(hdc, r.left, r.top, r.right, r.bottom);
		break;
	case 1:
		Ellipse(hdc, r.left, r.top, r.right, r.bottom);
		break;
	}

	SelectObject(hdc, hOldPen);
	SelectObject(hdc, hOldBrush);
	DeleteObject(hPen);
	DeleteObject(hBrush);
}

/*------Base64 Encoding Table------*/
static const char MimeBase64[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};

int base64_encode(char* text, int numBytes, char** encodedText)
{
	unsigned char input[3] = { 0,0,0 };
	unsigned char output[4] = { 0,0,0,0 };
	int   index, i, j, size;
	char* p, * plen;
	plen = text + numBytes - 1;
	size = (4 * (numBytes / 3)) + (numBytes % 3 ? 4 : 0) + 1;
	(*encodedText) = (char*)malloc(size);
	j = 0;
	for (i = 0, p = text; p <= plen; i++, p++) {
		index = i % 3;
		input[index] = *p;
		if (index == 2 || p == plen) {
			output[0] = ((input[0] & 0xFC) >> 2);
			output[1] = ((input[0] & 0x3) << 4) | ((input[1] & 0xF0) >> 4);
			output[2] = ((input[1] & 0xF) << 2) | ((input[2] & 0xC0) >> 6);
			output[3] = (input[2] & 0x3F);
			(*encodedText)[j++] = MimeBase64[output[0]];
			(*encodedText)[j++] = MimeBase64[output[1]];
			(*encodedText)[j++] = index == 0 ? '=' : MimeBase64[output[2]];
			(*encodedText)[j++] = index < 2 ? '=' : MimeBase64[output[3]];
			input[0] = input[1] = input[2] = 0;
		}
	}
	(*encodedText)[j] = '\0';
	return size;
}

/*------ Base64 Decoding Table ------*/
static int DecodeMimeBase64[256] = {
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
	52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
	-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
	15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
	-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
	41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
};


int base64_decode(char* text, char* dst, int numBytes)
{
	const char* cp;
	int space_idx = 0, phase;
	int d, prev_d = 0;
	unsigned char c;
	space_idx = 0;
	phase = 0;
	for (cp = text; *cp != '\0'; ++cp) {
		d = DecodeMimeBase64[(int)*cp];
		if (d != -1) {
			switch (phase) {
			case 0:
				++phase;
				break;
			case 1:
				c = ((prev_d << 2) | ((d & 0x30) >> 4));
				if (space_idx < numBytes)
					dst[space_idx++] = c;
				++phase;
				break;
			case 2:
				c = (((prev_d & 0xf) << 4) | ((d & 0x3c) >> 2));
				if (space_idx < numBytes)
					dst[space_idx++] = c;
				++phase;
				break;
			case 3:
				c = (((prev_d & 0x03) << 6) | d);
				if (space_idx < numBytes)
					dst[space_idx++] = c;
				phase = 0;
				break;
			}
			prev_d = d;
		}
	}
	dst[space_idx] = '\0';
	return space_idx;
}
//https://rangsub.tistory.com/8


//temp 문자열 길이 구하기
// num = _tcslen(temp);


// 문자열에 쓰기 1
// #include <stdio.h> 파일이 필요함
// _stprintf_s(temp, _T("Cown Down: %2d"), Count);


// 문자열에 쓰기 2
// _tcscpy_s(temp, _T("호환성 지원"));