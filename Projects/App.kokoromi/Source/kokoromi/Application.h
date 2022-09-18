#pragma once
#include <stdint.h>

class Application
{
public:
	static Application& Current();

public:
	void Run();
	void Shutdown();

	uintptr_t MainWindow() const { return mMainWindow; }

private:
	Application() = default;
	Application(const Application&) = delete;
	~Application() = default;

private:
	bool mShouldExit = false;
	uintptr_t mMainWindow = 0;
};
