#include "../pch.h"
#include "imgui_common.h"

class ImguiDxBase
{
private:
	HWND _handle = nullptr;
	bool _baseInit = false;
	int _width = 400;
	int _height = 400;
	int _top = 10;
	int _left = 10;

protected:
	long frameCounter = 0;
	void RenderMenu();

public:
	bool IsVisible() const;
	bool IsInited() const { return _baseInit; }
	int Width() { return _width; }
	int Height() { return _height; }
	int Top() { return _top; }
	int Left() { return _left; }
	HWND Handle() { return _handle; }
	bool IsHandleDifferent();

	explicit ImguiDxBase(HWND handle);

	~ImguiDxBase();
};