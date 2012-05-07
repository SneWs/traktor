#ifndef traktor_render_TypesDx10_H
#define traktor_render_TypesDx10_H

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup DX10
 */
const DXGI_FORMAT c_dxgiTextureFormats[] =
{
	DXGI_FORMAT_UNKNOWN,
	DXGI_FORMAT_R8_UNORM,
	DXGI_FORMAT_R8G8B8A8_UNORM,
	DXGI_FORMAT_UNKNOWN,
	DXGI_FORMAT_UNKNOWN,
	DXGI_FORMAT_UNKNOWN,
	DXGI_FORMAT_R16G16B16A16_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R16G16_FLOAT,
	DXGI_FORMAT_R32G32_FLOAT,
	DXGI_FORMAT_R16_FLOAT,
	DXGI_FORMAT_R32_FLOAT,
	DXGI_FORMAT_BC1_UNORM,	// DXT1
	DXGI_FORMAT_BC2_UNORM,	// DXT2
	DXGI_FORMAT_BC2_UNORM,	// DXT3
	DXGI_FORMAT_BC3_UNORM,	// DXT4
	DXGI_FORMAT_BC3_UNORM,	// DXT5
	DXGI_FORMAT_UNKNOWN,
	DXGI_FORMAT_UNKNOWN,
	DXGI_FORMAT_UNKNOWN,
	DXGI_FORMAT_UNKNOWN
};

/*!
 * \ingroup DX10
 */
const LPCSTR c_dxgiInputSemantic[] =
{
	"POSITION",
	"NORMAL",
	"TANGENT",
	"BINORMAL",
	"COLOR",
	"TEXCOORD"
};

/*!
 * \ingroup DX10
 */
const DXGI_FORMAT c_dxgiInputType[] =
{
	DXGI_FORMAT_R32_FLOAT,
	DXGI_FORMAT_R32G32_FLOAT,
	DXGI_FORMAT_R32G32B32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R8G8B8A8_UINT,
	DXGI_FORMAT_R8G8B8A8_UNORM,
	DXGI_FORMAT_R16G16_UINT,
	DXGI_FORMAT_R16G16B16A16_UINT,
	DXGI_FORMAT_R16G16_SNORM,
	DXGI_FORMAT_R16G16B16A16_SNORM,
	DXGI_FORMAT_R16G16_FLOAT,
	DXGI_FORMAT_R16G16B16A16_FLOAT
};

	}
}

#endif	// traktor_render_TypesDx10_H
