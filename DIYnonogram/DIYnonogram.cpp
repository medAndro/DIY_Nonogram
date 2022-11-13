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
LPCTSTR lpszClass = _T("DIY���׷� 0.1");
void DrawObject(HDC, RECT&, COLORREF, int);
void DrawObject(HDC, RECT&, COLORREF, COLORREF, int);
static COLORREF black_Color = RGB(0, 0, 0);
static COLORREF white_Color = RGB(255, 255, 255);
static RECT cell;
static RECT crt;
int initPoint = 0;//�׷����� �����ϴ� ��ġ
int cellSize = 25; //��ũ��
int x_Cell_Length = 10; //���� �� ����
int y_Cell_Length = 10; //���� �� ����
HWND createButton;
HWND playButton;
HWND returnMainButton;
HWND genCode;
HWND edit_In_Draw;
HWND edit_In_Main;
static int gameFlag = 1; //1�̸� �ʱ�ȭ��, 2�̸� �����, 3�̸� �÷���


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
	hWndMain = hWnd; // hWnd ������ ���������� ����!

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
			//MessageBox(hWnd, _T("����ȭ������ Ŭ����"), _T("��ư1"), MB_OK);
			gameFlag = 1;
			InitGame();
			break;
		case 2:
			//MessageBox(hWnd, _T("����� Ŭ����"), _T("��ư1"), MB_OK);
			gameFlag = 2;
			InitGame();
			break;
		case 3:
			//MessageBox(hWnd, _T("�÷��� Ŭ����"), _T("��ư1"), MB_OK);
			TCHAR editword[256];
			TCHAR cellRowData[256];
			TCHAR* token;
			int x, y, cnt;
			if (GetWindowTextW(edit_In_Main, editword, GetWindowTextLengthW(edit_In_Main) + 1) != NULL) {
				if(_tcsrchr(editword, _T('|'))==NULL)
					MessageBox(hWnd, _T("�����ڵ尡 �ùٸ��� �ʽ��ϴ�."), _T("�˸�"), MB_OK);
				else {
					gameFlag = 3;
					token = _tcstok(editword, _T("|"));
					x_Cell_Length = _ttoi(token); //���� �� ����
					token = _tcstok(NULL, _T("|")); // ���� ��ū
					y_Cell_Length = _ttoi(token); //���� �� ����
					token = _tcstok(NULL, _T("|")); // ���� ��ū
					_tcscpy(cellRowData, token);
					MessageBox(hWnd, cellRowData, _T("�˸�"), MB_OK);
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
				MessageBox(hWnd, _T("�����ڵ带 �Է����ּ���."), _T("�˸�"), MB_OK);
			}

			break;
		case 101:
			//�ڵ������ư
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
			//����׿� �ڵ�
			//_stprintf_s(str, _T("x��ǥ: %d, y��ǥ: %d, x�迭: %d, y�迭:%d"), LOWORD(lParam) - initPoint, HIWORD(lParam) - initPoint, (LOWORD(lParam) - initPoint)/ cellSize, (HIWORD(lParam) - initPoint) / cellSize);
			//MessageBox(hWnd, str, _T("�˸�"), MB_OK);

			//�� �ȿ����� �۵��ϵ��� ����
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
				MessageBox(hWnd, _T("�����մϴ� Ŭ���� �ϼ̽��ϴ�!\nó������ ���ư��ϴ�"), _T("�˸�"), MB_OK);
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
					MessageBox(hWnd, _T("�����մϴ�. �ٽ� �����մϴ�."), _T("�˸�"), MB_OK);
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
	case 1: //����ȭ��
		initPoint = 20;
		SetRect(&crt, 0, 0, 64 * 5 + 230, 64 * 6);
		AdjustWindowRect(&crt, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
		SetWindowPos(hWndMain, NULL, 0, 0, crt.right - crt.left, crt.bottom - crt.top, SWP_NOMOVE | SWP_NOZORDER);
		createButton = CreateWindow(_T("button"), _T("�����"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			40, 40, 100, 25, hWndMain, (HMENU)2, g_hInst, NULL);
		playButton = CreateWindow(_T("button"), _T("�÷���"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			150, 40, 100, 25, hWndMain, (HMENU)3, g_hInst, NULL);
		edit_In_Main = CreateWindow(_T("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
			20, 300, 200, 25, hWndMain, (HMENU)100, g_hInst, NULL);
		DestroyWindow(edit_In_Draw);
		DestroyWindow(genCode);
		DestroyWindow(returnMainButton);
		break;
	case 2: //�����ȭ��
		initPoint = 20;
		SetRect(&crt, 0, 0, 64 * 4 + 250, 350);
		AdjustWindowRect(&crt, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
		SetWindowPos(hWndMain, NULL, 0, 0, crt.right - crt.left, crt.bottom - crt.top, SWP_NOMOVE | SWP_NOZORDER);
		edit_In_Draw = CreateWindow(_T("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
			20, 300, 200, 25, hWndMain, (HMENU)100, g_hInst, NULL);
		genCode = CreateWindow(_T("button"), _T("�ڵ����"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			220, 300, 80, 25, hWndMain, (HMENU)101, g_hInst, NULL);

		returnMainButton = CreateWindow(_T("button"), _T("����ȭ������"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			300, 300, 120, 25, hWndMain, (HMENU)1, g_hInst, NULL);
		DestroyWindow(createButton);
		DestroyWindow(playButton);
		DestroyWindow(edit_In_Main);
		break;
	case 3: //�÷���ȭ��
		initPoint = 80;
		SetRect(&crt, 0, 0, 64 * 5 + 250, 400);
		AdjustWindowRect(&crt, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
		SetWindowPos(hWndMain, NULL, 0, 0, crt.right - crt.left, crt.bottom - crt.top, SWP_NOMOVE | SWP_NOZORDER);
		returnMainButton = CreateWindow(_T("button"), _T("����ȭ������"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
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
	//_stprintf_s(str, _T("x��ǥ: %d, y��ǥ: %d, x�迭: %d, y�迭:%d"), LOWORD(lParam) - initPoint, HIWORD(lParam) - initPoint, (LOWORD(lParam) - initPoint) / cellSize, (HIWORD(lParam) - initPoint) / cellSize);
		//MessageBox(hWnd, str, _T("�˸�"), MB_OK);
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
		lstrcpy(Mes, _T("DIY ���׷�"));
		TextOut(hdc, 50, 10, Mes, _tcslen(Mes));
		break;
	case 2:
		Rectangle(hdc, initPoint-1, initPoint-1, initPoint + cellSize*10 + 1, initPoint + cellSize*10 + 1); //�ٱ� �׵θ�
		for (y = 0; y < 10; y++) {
			for (x = 0; x < 10; x++) {
				if (arCell[x][y].St == WHITE)
					DrawCell(hdc, x * cellSize + initPoint, y * cellSize + initPoint, cellSize, white_Color);
				if (arCell[x][y].St == BLACK)
					DrawCell(hdc, x * cellSize + initPoint, y * cellSize + initPoint, cellSize, black_Color);
			}
		}

		lstrcpy(Mes, _T("�����"));//����
		TextOut(hdc, 300, 10, Mes, _tcslen(Mes));//tcslen����
		break;
	case 3:
		Rectangle(hdc, initPoint - 1, initPoint - 1, initPoint + cellSize * 10 + 1, initPoint + cellSize * 10 + 1); //�ٱ� �׵θ�
		for (y = 0; y < 10; y++) {
			for (x = 0; x < 10; x++) {
				if (arCell[x][y].St == WHITE)
					DrawCell(hdc, x * cellSize + initPoint, y * cellSize + initPoint, cellSize, white_Color);
				if (arCell[x][y].St == BLACK)
					DrawCell(hdc, x * cellSize + initPoint, y * cellSize + initPoint, cellSize, black_Color);
			}
		}
		DrawNums(hdc);

		lstrcpy(Mes, _T("�÷���"));//����
		TextOut(hdc, 400, 10, Mes, _tcslen(Mes));//tcslen����
		break;

	}
}
void GenNums(){
	TCHAR str[256];
	int x, y, num;
	//�ʱ�ȭ
	for (y = 0; y < 10; y++) {
		for (x = 0; x < 10; x++) {
			xNums[x][y] = 0;
			yNums[x][y] = 0;
		}
	}
	//9�ù��� ���ڵ� ����
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

	//12�ù��� ���ڵ� ����
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
	MessageBox(hWndMain, str, _T("�����"), MB_OK);
	_stprintf_s(str, _T("%d|%d|%d|%d"), yNums[0][0], yNums[0][1], yNums[0][2], yNums[0][3]);
	MessageBox(hWndMain, str, _T("�����"), MB_OK);

}

void DrawNums(HDC hdc) {
	TCHAR str[100];
	int x, y, num;
	//9�ù��� ���ڵ� �׸���
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
	//12�ù��� ���ڵ� �׸���
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
				return 0; //������ ���� ĥ�����ִµ� ���� �Ͼ���̸� 0 ��ȯ
			}
			if (arCell[x][y].Answer == 0 && arCell[x][y].St == BLACK) {
				return 0; //������ ���� ��ĥ�����ִµ� ���� �������̸� 0 ��ȯ
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

	switch (type) {		// type�� 0: �簢��, 1: ��
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


//temp ���ڿ� ���� ���ϱ�
// num = _tcslen(temp);


// ���ڿ��� ���� 1
// #include <stdio.h> ������ �ʿ���
// _stprintf_s(temp, _T("Cown Down: %2d"), Count);


// ���ڿ��� ���� 2
// _tcscpy_s(temp, _T("ȣȯ�� ����"));