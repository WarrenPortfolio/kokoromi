#include "Application.h"

#include <kokoromi/Renderer.h>
#include <kokoromi/Build.h>

#include <Framework/Debug/Debug.hpp>

#include <chrono>

// Including SDKDDKVer.h defines the highest available Windows platform.
// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.
#include <SDKDDKVer.h>

// Windows Header Files
#include <windows.h>

// C RunTime Header Files
#include <tchar.h>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

// Forward declare message handler from imgui_impl_win32.cpp
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
	// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
	// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
	// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

Application& Application::Current()
{
	static Application sApplcation;
	return sApplcation;
}

void Application::Shutdown()
{
	mShouldExit = true;
}

void Application::Run()
{
	// Build Data
	Build::CompileShaders();

	// Register the window class
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = _T("APP_WINDOW_CLASS");
	::RegisterClassEx(&wc);

	// Create the window
	HWND hwnd = CreateWindowEx(
		0,															// Optional window styles.
		wc.lpszClassName,											// Window class
		_T("kokoromi - Playground for Experimentation"),			// Window text
		WS_OVERLAPPEDWINDOW,										// Window style
		CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,	// Size and position
		NULL,														// Parent window    
		NULL,														// Menu
		wc.hInstance,												// Instance handle
		NULL														// Additional application data
	);
	Debug_AssertMsg(hwnd != NULL, "failed to create window");

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	mMainWindow = (uint64_t)hwnd;

	// create graphics
	Renderer* renderer = new Renderer();
	renderer->Startup();

	// On Windows, steady_clock is now based on QueryPerformanceCounter()
	// https://docs.microsoft.com/en-us/cpp/standard-library/chrono
	using ChronoClock = std::chrono::steady_clock;
	using TimePoint = ChronoClock::time_point;
	const TimePoint startTime = ChronoClock::now();
	TimePoint previosFrameTime = startTime;
	TimePoint currentFrameTime = startTime;

	// application loop
	while (mShouldExit == false)
	{
		// Poll and handle messages (inputs, window resize, etc.)
		{
			MSG msg;
			while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);

				// check application exit
				if (msg.message == WM_QUIT)
				{
					mShouldExit = true;
				}
			}

			// exit application loop
			if (mShouldExit)
			{
				continue; 
			}
		}

		// get current and previous frame time
		previosFrameTime = currentFrameTime;
		currentFrameTime = ChronoClock::now();

		// calculate delta time in seconds
		float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentFrameTime - previosFrameTime).count();

		renderer->FrameUpdate(deltaTime);
		renderer->FrameRender();
		renderer->FramePresent();
	}

	// destroy graphics
	renderer->Shutdown();
	delete renderer;

	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);
}