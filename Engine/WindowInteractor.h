#pragma once
#include "MathHelper.h"

// Redefined from GLFW as to not include it
enum Button
{
	LEFT_BUTTON,
	RIGHT_BUTTON,
	MIDDLE_BUTTON
};

class WindowInteractor
{
public:
	virtual void init(glm::vec2 mousePos, int windowWidth, int windowHeight)
	{
		prevMousePos = mousePos;
		WindowInteractor::mousePos = mousePos;
	}

	virtual void mouseMove(glm::vec2 mousePos)
	{
		// Push back
		prevMousePos = WindowInteractor::mousePos;
		// Update to new one
		WindowInteractor::mousePos = mousePos;
	}

	virtual void mouseDown(int button)
	{
		if (button == LEFT_BUTTON)
			leftButtonDown = true;
		else if (button == RIGHT_BUTTON)
			rightButtonDown = true;
		else if (button == MIDDLE_BUTTON)
			middleButtonDown = true;
	}
	virtual void mouseUp(int button)
	{
		if (button == LEFT_BUTTON)
			leftButtonDown = false;
		else if (button == RIGHT_BUTTON)
			rightButtonDown = false;
		else if (button == MIDDLE_BUTTON)
			middleButtonDown = false;
	}

	// Gives a delta scroll value
	virtual void mouseScroll(GLfloat ds) { }

	virtual void keyDown(int key) { }
	virtual void keyUp(int key) { }

	virtual void windowResize(int width, int height) { }

protected:
	glm::vec2 prevMousePos;
	glm::vec2 mousePos;

	bool leftButtonDown = false;
	bool rightButtonDown = false;
	bool middleButtonDown = false;
};