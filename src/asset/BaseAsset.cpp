#include "BaseAsset.h"

namespace app{
	namespace internal{
		AssetType BaseAsset_base::Get_assettype()
		{
			return this->assettype;
		}
	}
}