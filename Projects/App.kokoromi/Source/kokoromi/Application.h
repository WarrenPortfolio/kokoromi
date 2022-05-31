#pragma once
#include <stdint.h>

class Application
{
public:
	static Application& Current();

public:
	void Run();
	void Shutdown();

	uint64_t MainWindow() const { return mMainWindow; }

private:
	Application() = default;
	Application(const Application&) = delete;
	~Application() = default;

private:
	bool mShouldExit = false;
	uint64_t mMainWindow = 0;
};
