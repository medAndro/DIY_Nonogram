#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <tchar.h>
#include <string.h>
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

void DrawCell(HDC hdc, int x, int y, int cellSize, COLORREF inner_Color);

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
			TCHAR editword[256];
			TCHAR cellRowData[256];
			TCHAR* token;
			int x, y, cnt;
			if (GetWindowTextW(edit_In_Main, editword, GetWindowTextLengthW(edit_In_Main) + 1) != NULL) {
				if(_tcsrchr(editword, _T('|'))==NULL)
					MessageBox(hWnd, _T("게임코드가 올바르지 않습니다."), _T("알림"), MB_OK);
				else {
					gameFlag = 3;
					token = _tcstok(editword, _T("|"));
					x_Cell_Length = _ttoi(token); //가로 셀 길이
					token = _tcstok(NULL, _T("|")); // 다음 토큰
					y_Cell_Length = _ttoi(token); //세로 셀 길이
					token = _tcstok(NULL, _T("|")); // 다음 토큰
					_tcscpy(cellRowData, token);
					MessageBox(hWnd, cellRowData, _T("알림"), MB_OK);
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
				else
					arCell[(LOWORD(lParam) - initPoint) / cellSize][(HIWORD(lParam) - initPoint) / cellSize].St = WHITE;
				InvalidateRect(hWnd, NULL, FALSE);
			}
			if (CheckClear()) {
				MessageBox(hWnd, _T("축하합니다 클리어 하셨습니다!\n처음으로 돌아갑니다"), _T("알림"), MB_OK);
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
	case WM_TIMER:
		/*
		switch (wParam) {
		case 0:
			KillTimer(hWnd, 0);
			GameStatus = RUN;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case 1:
			KillTimer(hWnd, 1);
			GameStatus = RUN;
			for (i = 0; i < 4; i++) {
				for (j = 0; j < 4; j++) {
					if (arCell[i][j].St == TEMPFLIP)
						arCell[i][j].St = HIDDEN;
				}
			}
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		}*/
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
	TCHAR str[256];
	_stprintf_s(str, _T("%d|%d|"), x_Cell_Length, y_Cell_Length);
	int x, y, num= _tcslen(str);
	for (y = 0; y < 10; y++) {
		for (x = 0; x < 10; x++) {
			if (arCell[x][y].St == WHITE)
				str[num] = '0';
			if (arCell[x][y].St == BLACK)
				str[num] = '1';
			num++;
		}
	}
	str[num] = 0;
	//_stprintf_s(str, _T("x좌표: %d, y좌표: %d, x배열: %d, y배열:%d"), LOWORD(lParam) - initPoint, HIWORD(lParam) - initPoint, (LOWORD(lParam) - initPoint) / cellSize, (HIWORD(lParam) - initPoint) / cellSize);
		//MessageBox(hWnd, str, _T("알림"), MB_OK);
	DestroyWindow(edit_In_Draw);
	edit_In_Draw = CreateWindow(_T("edit"), str, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
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
		Rectangle(hdc, initPoint-1, initPoint-1, initPoint + cellSize*10 + 1, initPoint + cellSize*10 + 1); //바깥 테두리
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
		Rectangle(hdc, initPoint - 1, initPoint - 1, initPoint + cellSize * 10 + 1, initPoint + cellSize * 10 + 1); //바깥 테두리
		for (y = 0; y < 10; y++) {
			for (x = 0; x < 10; x++) {
				if (arCell[x][y].St == WHITE)
					DrawCell(hdc, x * cellSize + initPoint, y * cellSize + initPoint, cellSize, white_Color);
				if (arCell[x][y].St == BLACK)
					DrawCell(hdc, x * cellSize + initPoint, y * cellSize + initPoint, cellSize, black_Color);
			}
		}
		DrawNums(hdc);

		lstrcpy(Mes, _T("플레이"));//수정
		TextOut(hdc, 400, 10, Mes, _tcslen(Mes));//tcslen수정
		break;

	}
}
void GenNums(){
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

			if (arCell[x][y].Answer == 1 ) {
				if (xNums[y][num] == 0) {
					xNums[y][num] = 1;
				}
				else if(xNums[y][num] > 0) {
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
	_stprintf_s(str, _T("%d|%d|%d|%d"), xNums[0][0], xNums[0][1], xNums[0][2], xNums[0][3]);
	MessageBox(hWndMain, str, _T("디버그"), MB_OK);
	_stprintf_s(str, _T("%d|%d|%d|%d"), yNums[0][0], yNums[0][1], yNums[0][2], yNums[0][3]);
	MessageBox(hWndMain, str, _T("디버그"), MB_OK);

}

void DrawNums(HDC hdc) {
	TCHAR str[100];
	int x, y, num;
	//9시방향 숫자들 그리기
	for (y = 0; y < 10; y++) {
		num = 0;
		for (x = 0; x < 10; x++) {
			if (xNums[y][x] >= 1) {
				_stprintf_s(str, _T("%d"), xNums[y][x]);
				TextOut(hdc, (initPoint-12)-(num*9), (initPoint+3)+ y* cellSize, str, 1);
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
				_stprintf_s(str, _T("%d"), yNums[y][x]);
				TextOut(hdc, (initPoint + 8) + y * cellSize, (initPoint - 18) - (num * 16), str, 1);
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

	HBRUSH myBrush = (HBRUSH)CreateSolidBrush(inner_Color);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, myBrush);

	Rectangle(hdc, x, y, x + cellSize, y + cellSize);


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


//temp 문자열 길이 구하기
// num = _tcslen(temp);


// 문자열에 쓰기 1
// #include <stdio.h> 파일이 필요함
// _stprintf_s(temp, _T("Cown Down: %2d"), Count);


// 문자열에 쓰기 2
// _tcscpy_s(temp, _T("호환성 지원"));