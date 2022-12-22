#include "Application.hpp"

namespace W
{
	Application& Application::Current()
	{
		static Application sApplcation;
		return sApplcation;
	}

	void Application::Exit()
	{
		mShouldExit = true;
	}
} // namespace W
