
#include "gui/grid/GridEditor.h"
#include "grid/GridManager.h"

namespace Trainsim {
	struct Trainsim {
		GridManager gridManager;
		GridEditor gridEditor = { gridManager };
		Trainsim();
	};
}