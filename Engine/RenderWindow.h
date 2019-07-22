#pragma once
#include <vector>
#include <string>

class WindowInteractor;
class GLFWwindow;
class Renderer;

class RenderWindow
{
public:
	// Negative defaults tell it to use defaults based on screen
	RenderWindow(std::string windowName,
		int x = -1, int y = -1,
		int width = -1, int height = -1,
		bool fullscreen = false);
	~RenderWindow() { stop(); }

public:
	void start();
	void stop();
	void render();
	bool isActive();

	void setRenderer(Renderer* ren);
	void setWindowName(std::string name);
	void setInteractor(WindowInteractor* interactor);
	WindowInteractor* getInteractor() { return interactor; }

	void glfwWindowResize(int width, int height);

	int getWindowWidth();
	int getWindowHeight();
	int getFramebufferWidth();
	int getFramebufferHeight();

protected:
	void createWindow(std::string windowName, int x, int y, int windowWidth, int windowHeight, bool fullscreen = false);

	// GLFW callback
	static void glfwErrorCallback(int error, const char * description)
	{
		printf(("GLFW Error (" + std::to_string(error) + ") " + description + "\n").c_str());
	}
	static void glfwWindowResize(GLFWwindow* window, int width, int height);
	static void glfwMouseMove(GLFWwindow* window, double posX, double posY);
	static void glfwMouseButton(GLFWwindow* window, int button, int action, int mods);
	static void glfwScroll(GLFWwindow* window, double xoffset, double yoffset);
	static void glfwKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods);

protected:
	GLFWwindow* window = nullptr;
	Renderer* ren = nullptr;
	WindowInteractor* interactor = nullptr;
	std::string windowName = "GLFW Window";
};