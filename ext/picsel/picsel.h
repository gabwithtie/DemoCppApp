#include "project/ImageManager.h"
#include "project/ProjectManager.h"

#include "viewport/Viewport.h"

namespace picsel {
	class Picsel {
		ImageManager image_manager;
		ProjectManager project_manager;

		Viewport viewport;
	};
}