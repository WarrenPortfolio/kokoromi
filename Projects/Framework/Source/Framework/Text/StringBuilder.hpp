#pragma once

#include <vector>

namespace W
{
	class StringBuilder
	{
	private:
		std::vector<char> mData;

	public:
		StringBuilder();
		~StringBuilder() = default;

	public:
		const char* Text();

		void Clear();
		void Append(const char* text);
		void AppendFormat(const char* format, ...);
	};
} // namespace W