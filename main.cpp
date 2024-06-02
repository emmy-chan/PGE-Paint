#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <olectl.h>
#include <atlbase.h>
#include <atlconv.h>

class Example : public olc::PixelGameEngine
{
public:
	Example()
	{
		sAppName = "Paint";
	}

public:
	olc::Sprite* sprTools[4];
	olc::Sprite* sprTrash;

	/*struct sPixel
	{
		int x;
		int y;
		olc::Pixel col;
	};

	std::list<sPixel> listPixel;*/

	POINT GetMousePosSystem() {
		static POINT ptMouse;
		GetCursorPos(&ptMouse);
		//ScreenToClient(olc::windowHandle, &ptMouse);

		//ptMouse.x = clamped(ptMouse.x, 1, GetSystemMetrics(SM_CXSCREEN));
		//ptMouse.y = clamped(ptMouse.y, 1, GetSystemMetrics(SM_CYSCREEN));
		static POINT finMouse;
		finMouse.x = ptMouse.x;
		finMouse.y = ptMouse.y;

		return finMouse;
	}

	bool Holding(int iXStart, int iYStart, int iWidth, int iHeight, DWORD key) {
		if (GetAsyncKeyState(key)) // && hwWindow == GetActiveWindow()
			if (Hovering(iXStart, iYStart, iWidth, iHeight))
				return true;

		return false;
	}

	bool Hovering(int iXStart, int iYStart, int iWidth, int iHeight) {
		int mouseX = GetMouseX();
		int mouseY = GetMouseY();

		if (mouseX >= iXStart && iWidth - 1 >= mouseX)
			if (mouseY >= iYStart && iHeight - 1 >= mouseY)
				return true;

		return false;
	}

	bool Clicked(int iXStart, int iYStart, int iWidth, int iHeight, int mouseButton) {
		if (GetMouse(mouseButton).bPressed)
			if (Hovering(iXStart, iYStart, iWidth, iHeight))
				return true;

		return false;
	}

	bool bRenderedSlider;
	bool bDrawingSlider;
	bool bSetArrows;

	void drawHUESlider(float x, float y, float w, float h) {
		const olc::vd2d ptMouse = GetMousePos();

		DrawRect(int(x) - 1, int(y) - 1, int(w) + 1, int(h) + 1, olc::BLACK);

		if (!bRenderedSlider) {
			redrawColorPicker();

			for (int i = 0; i < int(w) - 1; i++) {
				const olc::Pixel currentCol = olc::FromHsv(i * 360 / w, 1, 1);
				DrawRect(int(x) + i, int(y), 1, int(h) - 1, currentCol);
			}

			bRenderedSlider = true;

			if (!bSetArrows) {
				for (int i = 0; i < 10; i++) {
					sliderColValue[i] = olc::RED;
					sliderValue[i] = int(x);
				}

				bSetArrows = true;
			}
		}

		for (int i = 0; i < w - 1; i++) {
			const olc::Pixel currentCol = olc::FromHsv(olc::clamped(i * 360 / w, 0, 360), 1, 1);

			if (GetAsyncKeyState(VK_LBUTTON) && Hovering(int(x), int(y), int(x + w), int(y + h))) {
				redrawColorPicker();

				const float hueResult = (float((ptMouse.x)) - x) * 360 / w - 1;
				sliderColValue[selColIndex] = olc::FromHsv(hueResult, 1, 1);
				selHue[selColIndex] = hueResult; //map slider to 0-360 (hue range)
				sliderValue[selColIndex] = int(ptMouse.x);
			}
		}

		//let's overlay this smooth decal, it looks nice
		DrawDecal(x, y, gfxHue.Decal(), { 0.2128f, 0.216f });

		//DrawString(100, 188, std::to_string(sliderColValue[selColIndex].r) + "-" + std::to_string(sliderColValue[selColIndex].g) + "-" + std::to_string(sliderColValue[selColIndex].b), olc::BLACK);
		//DrawString(100, 205, std::to_string(sliderValue[selColIndex]), olc::BLACK);
		//DrawString(100, 205, std::to_string(selHue), olc::BLACK);

		DrawRotatedDecal({ float(sliderValue[selColIndex]), float(y + h) + 2 }, gfxMouse.Decal(), 0.75f, { 0, 0 }, { 0.125, 0.125 }, sliderColValue[selColIndex]);
		//FillCircle(sliderValue[selColIndex], y + h / 2, 1, olc::BLACK);
		//FillTriangle(sliderValue[selColIndex], y + h + 2, sliderValue[selColIndex] - 3, y + h + 7, sliderValue[selColIndex] + 3, y + h + 7, olc::BLACK); //sliderCol
	}

	olc::Pixel myCols[10];
	olc::vd2d colSelectionPoint[10];
	olc::Pixel sliderColValue[10];
	int sliderValue[10];
	int selColIndex;
	float selHue[10];

	void drawMyColors(int x, int y, int size) {
		for (int i = 0; i < 10; i++) {
			//create a mapped white / black color based on our picker y selection
			const int invertedCol = int((colSelectionPoint[i].y - picker.y) * 255 / pickerSize.y);

			if (Hovering(x + 102, y, x + 102 + size, y + size)) {
				if (GetMouse(0).bPressed) {
					redrawColorPicker();
					selColIndex = i;
				}
				else if (GetMouse(1).bPressed) {
					redrawColorPicker();

					// select black and white with right click
					if (invertedCol > 250)
						colSelectionPoint[i] = { picker.x + 1, picker.y + 1 };
					else
						colSelectionPoint[i] = { picker.x + pickerSize.x - 1, picker.y + pickerSize.y - 1 };

					pickerCircleSize[i] = 1;
					selColIndex = i;
				}
			}

			DrawRect(x + 102, y + 1, size, size, olc::BLACK);
			FillRect(x + 103, y + 2, size - 1, size - 1, myCols[i]);

			//selected palette marker
			if (selColIndex == i)
				FillCircle(x + 105, y + 4, 1, olc::Pixel(invertedCol, invertedCol, invertedCol, 255));

			y += 10;
		}
	}

	olc::Pixel darker(uint8_t& r, uint8_t& g, uint8_t& b, int& a) {
		const float FACTOR = .7f;

		return olc::Pixel(std::max((int)(r * FACTOR), 0),
			std::max((int)(g * FACTOR), 0),
			std::max((int)(b * FACTOR), 0),
			a);
	}

	olc::Pixel brighter(uint8_t& r, uint8_t& g, uint8_t& b, int& a) {
		const float FACTOR = 4.7f;

		int i = (int)(1.0 / (1.0 - FACTOR));
		if (r == 0 && g == 0 && b == 0) {
			return olc::Pixel(i, i, i, a);
		}
		if (r > 0 && r < i) r = i;
		if (g > 0 && g < i) g = i;
		if (b > 0 && b < i) b = i;

		return olc::Pixel(std::min((int)(r / FACTOR), 255),
			std::min((int)(g / FACTOR), 255),
			std::min((int)(b / FACTOR), 255),
			a);
	}

	bool bPickerOpen;
	bool bSetDefaults;
	int pickerCircleSize[10];
	olc::vd2d picker = { int(ScreenWidth() / 2) - 50, int(ScreenHeight() / 2) - 50 };
	olc::vd2d pickerSize = { 100, 100 };
	olc::Pixel fontCol = olc::Pixel(255, 15, 125);

	bool bRenderedIcon = false;

	void redrawColorPicker() {
		const olc::vd2d ptMouse = GetMousePos();

		//render our outline
		DrawRect(int(picker.x - 1), int(picker.y - 1), 101, 101, olc::BLACK);

		for (int i = 0; i < pickerSize.x; i++) {
			//generate a color based on our hue selection, increment the saturation over time
			olc::Pixel currentCol = olc::FromHsv(selHue[selColIndex], i * 1.f / pickerSize.x, 1);

			for (int j = 0; j < pickerSize.y; j++) {
				currentCol.a = 255 - int(j * 2.55); //j * 255 / pickerSize.y

				if (ptMouse.x == picker.x + i && ptMouse.y == picker.y + j) {
					colSelectionPoint[selColIndex] = ptMouse;
					myCols[selColIndex] = currentCol;

					//shrink cursor at edges
					if (ptMouse.x < picker.x + 3 || ptMouse.x > picker.x + 96 || ptMouse.y < picker.y + 3 || ptMouse.y > picker.y + 96)
						pickerCircleSize[selColIndex] = 1;
					else
						pickerCircleSize[selColIndex] = 2;
				}
				else {
					if (colSelectionPoint[selColIndex].x == picker.x + i && colSelectionPoint[selColIndex].y == picker.y + j)
						myCols[selColIndex] = currentCol;
				}

				Draw(int(picker.x + i), int(picker.y + j), currentCol);
			}
		}
	}

	void colorPicker(int x, int y, int w, int h) {
		bool bHovering = Hovering(x, y, x + w, y + h);

		if (bHovering && GetMouse(0).bPressed)
			bPickerOpen = !bPickerOpen;
		else if (bHovering && GetMouse(1).bPressed)
			myCols[selColIndex] = olc::BLACK;

		if (bPickerOpen) {
			drawMyColors(int(picker.x), int(picker.y), 8);
			DrawStringShadow({ int(picker.x), int(picker.y - 12) }, "pick a color", fontCol);
			
			drawHUESlider(float(picker.x), float(picker.y) + 103, 100, 10);
			//DrawDecal(GetMouseX(), GetMouseY(), gfxMouse.Decal(), { 0.075, 0.075 }); //cursor
			static olc::Pixel circleCol = olc::BLACK;
			olc::Pixel currentCol = int(selHue[selColIndex]);

			bHovering = Hovering(int(picker.x), int(picker.y), int(picker.x + pickerSize.x), int(picker.y + pickerSize.y));

			if (bHovering && GetAsyncKeyState(VK_LBUTTON))
				redrawColorPicker();

			//change selection circle col based on pos
			//int circleCol = (colSelectionPoint[selColIndex].y - picker.y) * 255 / h; // if we want mapped to height, only useful when picker size is not static

			circleCol.r = int(colSelectionPoint[selColIndex].y - picker.y * 2.55);
			circleCol.g = int(colSelectionPoint[selColIndex].y - picker.y * 2.55);
			circleCol.b = int(colSelectionPoint[selColIndex].y - picker.y * 2.55);

			//MAKE CIRCLE A DECAL SO IT SHOWS OVER GRADIENT
			DrawCircle(colSelectionPoint[selColIndex], pickerCircleSize[selColIndex], circleCol);
			//DrawDecal(x - 66, y - 152, gfxGradient.Decal(), { 0.2, 0.2 }, olc::FromHsv(selHue, 1, 1));
		}
		else {
			//setup the picker icon and some default stuff, also draw our gradient once (updates in hue slider)
			if (!bRenderedIcon) {
				for (int i = 0; i < w; i++) {
					for (int j = 0; j < h; j++) {
						const olc::Pixel currentCol = olc::Pixel(int(i * 18.215), int(255 - j * 18.215), int(j * 18.215), 255);
						Draw(x + i, y + j, currentCol);
					}
				}

				//set our preset color palette positions on the picker
				if (!bSetDefaults) {
					for (int i = 2; i < 10; i++) {
						colSelectionPoint[i] = { picker.x + pickerSize.x - 1, picker.y };
						pickerCircleSize[i] = 1;
					}
				}

				bSetDefaults = true;
				bRenderedIcon = true;
			}

			bRenderedSlider = false;
			//DrawDecal(GetMouseX(), GetMouseY() - 3, dclCursor, { 0.25, 0.25 });
		}
	}

	int paintToolSelected;
	int oldSelection;
	//olc::Decal* dclCursor;

	void drawButton(int x, int y, int value) { //olc::Sprite* spr
		static int size = 15;
		int w = x + size;
		int h = y + size;
		bool bHovering = Hovering(x, y, w, h);

		DrawSprite(x, y, sprTools[value]);

		if (GetAsyncKeyState(VK_LBUTTON) && bHovering) { //GetAsyncKeyState(VK_LBUTTON)
			//dclCursor = new olc::Decal(sprTools[value]);
			oldSelection = paintToolSelected;
			paintToolSelected = value;
			LeftClickUp();
		}

		if (paintToolSelected == value)
			DrawRect(x, y, size, size, olc::WHITE);
		else
			DrawRect(x, y, size, size, olc::BLACK);
	}

	bool yesNo(int x, int y) {
		bool bHoveringYes = Hovering(x - 30, y, x - 6, y + 10);
		bool bHoveringNo = Hovering(x + 24, y, x + 40, y + 10);
		bPickerOpen = false;

		if (bHoveringYes) {
			DrawStringShadow({ x - 30, y }, "yes", olc::GREEN);

			if (GetMouse(0).bPressed) {
				for (int ny = 0; ny < height; ny++)
					for (int nx = 0; nx < width; nx++)
						tiles[nx][ny] = olc::WHITE;

				return true;
			}
		}
		else
			DrawStringShadow({ x - 30, y }, "yes", fontCol);

		if (bHoveringNo) {
			DrawStringShadow({ x + 24, y }, "no", olc::GREEN);

			if (GetMouse(0).bPressed)
				return true;
		}
		else
			DrawStringShadow({ x + 24, y }, "no", fontCol);

		return false;
	}

	bool bTrashDialogue;

	void trash() {
		static int size = 15;
		static int x = ScreenWidth() / 2 + 33;
		static int y = ScreenHeight() - 19;
		static int w = x + size;
		static int h = y + size;
		bool bHovering = Hovering(x, y, w, h);

		if (GetMouse(0).bPressed && bHovering)
			bTrashDialogue = !bTrashDialogue;

		if (bTrashDialogue) {
			DrawStringShadow({ ScreenWidth() / 2 - 58, ScreenHeight() / 2 - 12 }, "delete project?", fontCol);
			
			if (yesNo(ScreenWidth() / 2, ScreenHeight() / 2)) {
				bTrashDialogue = false;
				LeftClickUp();
			}
		}
	}

	int gridSize = 120;
	olc::Pixel tiles[64][64];
	int width = 56;
	int height = 65;

	void floodFill(int x, int y, olc::Pixel curCol, olc::Pixel col)
	{
		if (x < width && y < height && y > 0 && x > 0 && tiles[x][y] == curCol && curCol != col)
		{
			tiles[x][y] = col;
			floodFill(x + 1, y, curCol, col);
			floodFill(x, y + 1, curCol, col);
			floodFill(x - 1, y, curCol, col);
			floodFill(x, y - 1, curCol, col);
		}
	}

	olc::Pixel curCol;

	void editor(int start_y) {
		int x = -gridSizeX;
		for (int ny = 0; ny < height; ny++) {
			int y = -gridSizeY + start_y;

			for (int nx = 0; nx < width; nx++) {
				bool bHovering = Hovering(x, y, x + gridSizeX, y + gridSizeY);

				if (!bPickerOpen && !bTrashDialogue && bHovering) {
					RECT rect = { NULL };
					GetWindowRect(hw, &rect);

					switch (paintToolSelected) {
					case 0:
						//brush
						if (GetAsyncKeyState(VK_LBUTTON)) {
							if (GetMouseX() > 0 && GetMouseX() < ScreenWidth() - 1 && GetMouseY() > 0 && GetMouseY() < ScreenHeight() - 1)
								tiles[nx][ny] = myCols[selColIndex];
						}
						else if (GetAsyncKeyState(VK_RBUTTON)) {
							if (GetMouseX() > 0 && GetMouseX() < ScreenWidth() - 1 && GetMouseY() > 0 && GetMouseY() < ScreenHeight() - 1)
								tiles[nx][ny] = olc::WHITE;
						}

						FillRect(x, y, gridSizeX, gridSizeY, myCols[selColIndex]);
						break;
					case 1:
						//paint bucket
						curCol = tiles[nx][ny];

						if (GetMouse(0).bPressed)
							floodFill(nx, ny, curCol, myCols[selColIndex]);

						FillRect(x, y, gridSizeX, gridSizeY, myCols[selColIndex]);
						break;
					case 2:
						//eraser
						if (GetAsyncKeyState(VK_LBUTTON))
							if (GetMouseX() > 0 && GetMouseX() < ScreenWidth() - 1 && GetMouseY() > 0 && GetMouseY() < ScreenHeight() - 1)
								tiles[nx][ny] = olc::WHITE;

						DrawRect(x, y, gridSizeX - 1, gridSizeY - 1, olc::BLACK);
						FillRect(x + 1, y + 1, gridSizeX - 2, gridSizeY - 2, olc::WHITE);
						break;
					case 3:
						//dropper
						curCol = tiles[nx][ny];

						if (GetMouse(0).bPressed) {
							paintToolSelected = oldSelection;
							//dclCursor = new olc::Decal(sprTools[paintToolSelected]);
							myCols[selColIndex] = tiles[nx][ny];
							LeftClickUp();
						}

						DrawRect(x, y, gridSizeX - 1, gridSizeY - 1, olc::BLACK);
						FillRect(x + 1, y + 1, gridSizeX - 2, gridSizeY - 2, curCol);

						break;
					}
				}
				else if (!bHovering)
					FillRect(x, y, gridSizeX, gridSizeY, tiles[nx][ny]);

				y += gridSizeY;
			}

			x += gridSizeX;
		}
	}

	olc::Renderable gfxGradient;
	olc::Renderable gfxMouse;
	olc::Renderable gfxHue;

	int gridSizeX;
	int gridSizeY;

	bool OnUserCreate() override
	{
		gridSizeX = ScreenWidth() / (gridSize / 2);
		gridSizeY = ScreenHeight() / (gridSize / 2);

		sprTools[0] = new olc::Sprite("gfx/brush.png");
		sprTools[1] = new olc::Sprite("gfx/bucket.png");
		sprTools[2] = new olc::Sprite("gfx/eraser.png");
		sprTools[3] = new olc::Sprite("gfx/dropper.png");
		sprTrash = new olc::Sprite("gfx/trash.png");
		gfxMouse.Load("gfx/cursor.png");
		gfxHue.Load("gfx/hue.png");
		gfxGradient.Load("gfx/gradient.png");
		//dclCursor = new olc::Decal(sprTools[0]);

		Clear(olc::BLACK);

		//clear grid to white
		for (int ny = 0; ny < height; ny++)
			for (int nx = 0; nx < width; nx++)
				tiles[nx][ny] = olc::WHITE;

		//set default colors
		for (int i = 0; i < 10; i++) {
			pickerCircleSize[i] = 2;
			myCols[i] = olc::RED;

			switch (i) {
			case 0:
				myCols[i].a = 0;

				colSelectionPoint[selColIndex] = { picker.x + pickerSize.x - 1, picker.y + pickerSize.y - 1 };
				pickerCircleSize[selColIndex] = 1;
				break;
			case 1:
				colSelectionPoint[i] = { picker.x, picker.y };
				pickerCircleSize[i] = 1;
				myCols[i] = olc::WHITE;
				break;
			/*case 2:
				sliderValue[i] = 85;
				myCols[i] = olc::Pixel(255, 122, 8);
				sliderColValue[i] = myCols[i];
				break;
			case 3:
				break;
			case 4:
				break;
			case 5:
				break;
			case 6:
				break;
			case 7:
				break;
			case 8:
				break;
			case 9:
				break;*/
			}
		}

		FillRect(0, ScreenHeight() - 23, ScreenWidth(), 23, olc::BLACK);

		//trash bucket
		SetPixelMode(olc::Pixel::MASK);
		DrawSprite(ScreenWidth() / 2 + 33, ScreenHeight() - 19, sprTrash);
		SetPixelMode(olc::Pixel::NORMAL);

		return true;
	}

	inline void LeftClickUp()
	{
		INPUT    Input = { 0 };
		//// left down 
		//Input.type = INPUT_MOUSE;
		//Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		//::SendInput(1, &Input, sizeof(INPUT));

		// left up
		::ZeroMemory(&Input, sizeof(INPUT));
		Input.type = INPUT_MOUSE;
		Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
		::SendInput(1, &Input, sizeof(INPUT));

		// right up
		::ZeroMemory(&Input, sizeof(INPUT));
		Input.type = INPUT_MOUSE;
		Input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
		::SendInput(1, &Input, sizeof(INPUT));
	}

	HWND hw = FindWindowA("OLC_PIXEL_GAME_ENGINE", NULL);

	POINT getMousePos() {
		static POINT mouse;
		GetCursorPos(&mouse);
		ScreenToClient(hw, &mouse);
		return mouse;
	}

	bool saveBitmap(LPCSTR filename, HBITMAP bmp, HPALETTE pal)
	{
		bool result = false;
		PICTDESC pd;

		pd.cbSizeofstruct = sizeof(PICTDESC);
		pd.picType = PICTYPE_BITMAP;
		pd.bmp.hbitmap = bmp;
		pd.bmp.hpal = pal;

		LPPICTURE picture;
		HRESULT res = OleCreatePictureIndirect(&pd, IID_IPicture, false,
			reinterpret_cast<void**>(&picture));

		if (!SUCCEEDED(res))
			return false;

		LPSTREAM stream;
		res = CreateStreamOnHGlobal(0, true, &stream);

		if (!SUCCEEDED(res))
		{
			picture->Release();
			return false;
		}

		LONG bytes_streamed;
		res = picture->SaveAsFile(stream, true, &bytes_streamed);

		HANDLE file = CreateFile(ATL::CA2W(filename), GENERIC_WRITE, FILE_SHARE_READ, 0,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

		if (!SUCCEEDED(res) || !file)
		{
			stream->Release();
			picture->Release();
			return false;
		}

		HGLOBAL mem = 0;
		GetHGlobalFromStream(stream, &mem);
		LPVOID data = GlobalLock(mem);

		DWORD bytes_written;

		result = !!WriteFile(file, data, bytes_streamed, &bytes_written, 0);
		result &= (bytes_written == static_cast<DWORD>(bytes_streamed));

		GlobalUnlock(mem);
		CloseHandle(file);

		stream->Release();
		picture->Release();

		return result;
	}

	//void CaptureScreen()
	//{
	//	//int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	//	//int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	//	HWND hw = FindWindowA("OLC_PIXEL_GAME_ENGINE", NULL);
	//	RECT rect = { NULL };
	//	GetWindowRect(hw, &rect);
	//	int hh = rect.bottom - height;
	//	int w = rect.right - rect.left - 60;
	//	int h = rect.bottom - rect.top - 170;
	//	HWND hDesktopWnd = hw; //GetDesktopWindow();
	//	HDC hDesktopDC = GetDC(hDesktopWnd);
	//	HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);
	//	HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDesktopDC,
	//		w, h);
	//	SelectObject(hCaptureDC, hCaptureBitmap);
	//	BitBlt(hCaptureDC, 0, 0, w, h,
	//		hDesktopDC, rect.left + 10, rect.top, SRCCOPY | CAPTUREBLT);

	//	LPCSTR fname = "image.png";
	//	HPALETTE hpal = NULL;
	//	saveBitmap(fname, hCaptureBitmap, hpal);

	//	ReleaseDC(hDesktopWnd, hDesktopDC);
	//	DeleteDC(hCaptureDC);
	//	DeleteObject(hCaptureBitmap);
	//}

	void LockFPS()
	{
		if (GetElapsedTime() < 0.066f)
			Sleep(1);

		/*int deltaTime = std::abs(GetElapsedTime() - 0.02f) * 1000;
		std::this_thread::sleep_for(std::chrono::milliseconds(16 - deltaTime));*/
	}

	bool bClickedBtn[60];

	bool drawButton(int x, int y, float w, float h, const char* title, olc::Pixel color, int index) {
		bool bHovering = Hovering(x, y, x + w, y + h);
		bool holding = bHovering && GetMouse(0).bHeld;
		olc::Pixel hoverColor = olc::Pixel(int(color.r * 1.25), int(color.g * 1.25), int(color.b * 1.25), 255);
		olc::Pixel clickedColor = olc::Pixel(color.r / 3, color.g / 3, color.b / 3, color.a / 3);
		static float lerpCol = hoverColor.a;

		if (bHovering && lerpCol <= hoverColor.a && !holding)
			color = hoverColor;
		else if (!bHovering && lerpCol <= hoverColor.a && !holding) {
			color = olc::Pixel(color.r, color.g, color.b, 255);
			hoverColor = color;
		}

		/*if (holding)
			color = olc::Pixel(color.r / 3, color.g / 3, color.b / 3, 255);
		else*/

		if (bHovering && GetMouse(0).bPressed) {
			if (!bClickedBtn[index]) {
				bClickedBtn[index] = true;
				return true;
			}
		}

		if (bClickedBtn[index]) {
			static bool bFlip;

			if (!bFlip) {
				lerpCol = olc::lerp(lerpCol, clickedColor.a, 24 * GetElapsedTime());
				color = olc::Pixel(color.r, color.g, color.b, lerpCol);

				if ((int)lerpCol == clickedColor.a)
					bFlip = true;
			}
			else {
				lerpCol = olc::lerp(lerpCol, hoverColor.a, 12 * GetElapsedTime());
				color = olc::Pixel(color.r, color.g, color.b, lerpCol);

				if (hoverColor.a - lerpCol < 1.f) {
					bClickedBtn[index] = false;
					bFlip = false;
				}
			}
		}

		FillRect(int(x), int(y), int(w), int(h), color);
		DrawString(x + w / 2 - 4, y + 1, title, olc::GREY);
		return false;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		LockFPS();

		//if (GetKey(olc::Key::HOME).bPressed) {
			//CaptureScreen();
		//}

		if (GetKey(olc::Key::C).bPressed)
			bPickerOpen = !bPickerOpen;
		else if (GetKey(olc::Key::A).bPressed) {
			paintToolSelected--;

			if (paintToolSelected < 0)
				paintToolSelected = 3;

			//dclCursor = new olc::Decal(sprTools[paintToolSelected]);
		}
		else if (GetKey(olc::Key::D).bPressed) {
			paintToolSelected++;

			if (paintToolSelected > 3)
				paintToolSelected = 0;

			//dclCursor = new olc::Decal(sprTools[paintToolSelected]);
		}

		//DrawLine(0, ScreenHeight() - 23, ScreenWidth(), ScreenHeight() - 23);

		//if (mouse.x < 0 && mouse.x > ScreenWidth())
			//LeftClickUp();

		if (IsFocused()) {
			//window dragging
			POINT ptMouse = GetMousePosSystem();
			bool bHovering = Hovering(0, 0, ScreenWidth(), 15);
			static int bWasntHolding = false;
			static int bGrabbing = false;
			static int iOffsetX = 0, iOffsetY = 0;

			if (bHovering && GetMouse(0).bHeld && !bGrabbing) {
				static RECT rect = { NULL };
				GetWindowRect(olc::windowHandle, &rect);
				bGrabbing = true;

				iOffsetX = rect.left - ptMouse.x;
				iOffsetY = rect.top - ptMouse.y;
			}
			else if (bGrabbing && !GetAsyncKeyState(VK_LBUTTON)) {
				bGrabbing = false;
				iOffsetX = 0;
				iOffsetY = 0;
			}
			else if (bGrabbing)
				SetWindowPos(olc::windowHandle, nullptr, ptMouse.x + iOffsetX, olc::clamped(ptMouse.y + iOffsetY, 0, GetSystemMetrics(SM_CYSCREEN) - 70), 0, 0, SWP_NOSIZE | SWP_NOZORDER); //clamped(ptMouse.x + iOffsetX, 0, GetSystemMetrics(SM_CXSCREEN) + (rect.left - rect.right)

			bWasntHolding = bHovering && !GetAsyncKeyState(VK_LBUTTON);

			if (!bPickerOpen)
				editor(15);

			trash();
			colorPicker(ScreenWidth() / 2 + 17, ScreenHeight() - 18, 14, 14);
			SetPixelMode(olc::Pixel::MASK);
			drawButton(ScreenWidth() / 2 - 48, ScreenHeight() - 19, 0);
			drawButton(ScreenWidth() / 2 - 32, ScreenHeight() - 19, 1);
			drawButton(ScreenWidth() / 2 - 16, ScreenHeight() - 19, 2);
			drawButton(ScreenWidth() / 2, ScreenHeight() - 19, 3);
			SetPixelMode(olc::Pixel::NORMAL);
		}

		//DrawStringShadow({ 100, 100 }, std::to_string(mouse.x) + " " + std::to_string(mouse.y), olc::WHITE);
		/*static HWND hw = FindWindowA("OLC_PIXEL_GAME_ENGINE", NULL);
		RECT rect = { NULL };
		GetWindowRect(hw, &rect);
		int w = rect.right - rect.left;
		int h = rect.bottom - rect.top;
		olc::vd2d delta = { float(rect.right) - (ScreenWidth() - olc::viewPos.x) , olc::viewPos.y - ScreenHeight() };

		if (delta.x < 0)
			delta.x = 0;

		if (delta.y < 0)
			delta.y = 0;*/
		
		//DrawStringShadow({ 100, 100 }, std::to_string(delta.x) + " " + std::to_string(delta.y), olc::WHITE);
		//DrawString(150, 175, std::to_string(myCols[selColIndex].r) + "-" + std::to_string(myCols[selColIndex].g) + "-" + std::to_string(myCols[selColIndex].b), olc::BLACK);

		//exit button
		if (drawButton(ScreenWidth() - 16, 0, 16, 11, "x", olc::Pixel(75, 75, 75, 255), 0))
			ExitProcess(0);

		return true;
	}
};


int main()
{
	FreeConsole();

	Example demo;
	if (demo.Construct(255, 255, 5, 5, false)) //426, 240, 5, 5 //255, 255, 5, 5
		demo.Start();

	return 0;
}