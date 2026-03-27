#ifndef INC_8INF259_TP3_STRING_H
#define INC_8INF259_TP3_STRING_H
#include <string>
#include <vector>

namespace library::string
{
	// Modified from: https://www.oreilly.com/library/view/c-cookbook/0596007612/ch04s09.html
	inline std::string join(
		const std::vector<std::string>& v,
		const std::string& delimiter
	)
	{
		std::string s;

		for (auto p = v.begin(); p != v.end(); ++p)
		{
			s += *p;

			if (p != v.end() - 1)
				s += delimiter;
		}

		return s;
	}
}

#endif //INC_8INF259_TP3_STRING_H
