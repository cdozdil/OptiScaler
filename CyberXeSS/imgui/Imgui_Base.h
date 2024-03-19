#include "../pch.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"

class Imgui_Base
{
private:
	bool _baseInit = false;
	int _width;
	int _height;
	int _top;
	int _left;

protected:

	void RenderMenu();

public:
	bool IsVisible() const;
	void SetVisible(bool visible) const;
	bool IsInited() const { return _baseInit; }
	int Width() { return _width; }
	int Height() { return _height; }
	int Top() { return _top; }
	int Left() { return _left; }

	explicit Imgui_Base(HWND handle);

	~Imgui_Base();
};