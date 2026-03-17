#pragma once

#include "Math/gbe_math.h"
#include <functional>
#include <vector>

namespace gbe {
	template<typename T>
	class TrackedVariable {
	private:
		T variable;
		std::vector<std::function<void(T, T)>> doOnChange;
		std::vector<std::function<void(T, T)>> doAlways;
	public:
		TrackedVariable(std::function<void(T, T)> func) {
			doOnChange.push_back(func);
		}

		void AddCallback(std::function<void(T, T)> func) {
			doOnChange.push_back(func);
		}

		T& Get() {
			return variable;
		}
		void Set(T value) {
			T oldvalue = this->variable;
			this->variable = value;

			for (const auto& func : doOnChange)
			{
				func(oldvalue, value);
			}
		}
	};
}