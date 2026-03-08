#include "GuiElement.h"

namespace app {
	void GuiElement::Draw()
	{
		if (!this->ext_Begin())
		{
			this->ext_End();
			return;
		}
		this->DrawSelf();
		this->ext_End();
	}
}