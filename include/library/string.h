#ifndef INC_8INF259_TP3_STRING_H
#define INC_8INF259_TP3_STRING_H
#include <sstream>
#include <string>
#include <vector>

namespace library::string
{
	template <typename T>
	std::string join(const std::vector<T>& v, const std::string& delimiter)
	{
		std::ostringstream oss;

		for (auto it = v.begin(); it != v.end(); ++it)
		{
			oss << *it;

			if (std::next(it) != v.end())
				oss << delimiter;
		}

		return oss.str();
	}
}

#endif //INC_8INF259_TP3_STRING_H
