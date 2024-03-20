#include "../pch.h"
#include "imgui/imgui.h"

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
typedef BOOL(WINAPI* PFN_SetCursorPos)(int x, int y);

class Imgui_Base
{
private:
	HWND _handle = nullptr;
	bool _baseInit = false;
	int _width = 400;
	int _height = 400;
	int _top = 10;
	int _left = 10;

protected:
	ImGuiContext* context;

	void RenderMenu();

public:
	bool IsVisible() const;
	void SetVisible(bool visible) const;
	bool IsInited() const { return _baseInit; }
	int Width() { return _width; }
	int Height() { return _height; }
	int Top() { return _top; }
	int Left() { return _left; }
	HWND Handle() { return _handle; }
	bool IsHandleDifferent();

	explicit Imgui_Base(HWND handle);

	~Imgui_Base();
};