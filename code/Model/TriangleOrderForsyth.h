#ifndef traktor_model_TriangleOrderForsyth_H
#define traktor_model_TriangleOrderForsyth_H

#include <vector>
#include "Core/Config.h"

namespace traktor
{
	namespace model
	{

void optimizeFaces(const std::vector< uint32_t >& indexList, uint32_t vertexCount, std::vector< uint32_t >& outNewIndexList, uint32_t lruCacheSize);

	}
}

#endif	// traktor_model_TriangleOrderForsyth_H
