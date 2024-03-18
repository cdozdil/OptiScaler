#include "../pch.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"

class Imgui_Base
{
private:
	bool _baseInit = false;

protected:

	void RenderMenu();

public:
	bool IsVisible() const;
	void SetVisible(bool visible) const;
	bool IsInited() const { return _baseInit; }

	explicit Imgui_Base(HWND handle);

	~Imgui_Base();
};