#pragma once
#include <stdint.h>

namespace W
{
	class Application
	{
	public:
		static Application& Current();

		using AppStartupDelegate = void(*)();
		AppStartupDelegate mStartupCallback = nullptr;

		using AppTickDelegate = void(*)(float deltaTime);
		AppTickDelegate mTickCallback = nullptr;

	public:
		void Run();
		void Exit();

		uintptr_t MainWindow() const { return mMainWindow; }

	private:
		Application() = default;
		Application(const Application&) = delete;
		~Application() = default;

	private:
		bool mShouldExit = false;
		uintptr_t mMainWindow = 0;
	};
} // namespace W
