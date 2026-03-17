#define _USE_MATH_DEFINES
#include <math.h>

namespace gbe {
	inline double toRad(double deg) {
		return deg * (M_PI / 180.0);
	}
}