#include "BaseAsset.h"

namespace app{
	namespace internal{
		AssetType IBaseAsset::Get_assettype()
		{
			return this->assettype;
		}
	}
}