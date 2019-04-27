#pragma once

struct MouseEvent {
	float posx, posy;
	enum class Button { None = 0, Left = 1, Middle = 2, Right = 4 };
	Button button;
};