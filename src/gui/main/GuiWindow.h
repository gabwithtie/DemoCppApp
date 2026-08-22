#pragma once

#include "GuiElement.h"

#include <typeinfo>
#include <string>

namespace app {
	class GuiWindow : public GuiElement {
	protected:
		bool is_open = true;
		bool pointer_here = false;

		inline virtual void pushStyles() {

		}
		inline virtual void popStyles() {

		}
	private:
		bool ext_Begin() override;
		void ext_End() override;

	public:
		inline bool IsPointerHere() {
			return pointer_here;
		}

		inline bool IsOpen() {
			return is_open;
		}
		inline virtual void SetOpen(bool newstate) {
			is_open = newstate;
		}
		virtual std::string GetWindowId() = 0;
	};
}