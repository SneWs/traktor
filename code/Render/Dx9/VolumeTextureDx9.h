#ifndef traktor_render_VolumeTextureDx9_H
#define traktor_render_VolumeTextureDx9_H

#include "Render/VolumeTexture.h"
#include "Render/Dx9/TextureBaseDx9.h"
#include "Core/Heap/Ref.h"
#include "Core/Misc/ComRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX9_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextDx9;
struct VolumeTextureCreateDesc;
		
/*!
 * \ingroup DX9 Xbox360
 */
class T_DLLCLASS VolumeTextureDx9
:	public VolumeTexture
,	public TextureBaseDx9
{
	T_RTTI_CLASS(VolumeTextureDx9)
	
public:
	VolumeTextureDx9(ContextDx9* context);

	virtual ~VolumeTextureDx9();

	bool create(IDirect3DDevice9* d3dDevice, const VolumeTextureCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	virtual IDirect3DBaseTexture9* getD3DBaseTexture() const;
	
private:
	Ref< ContextDx9 > m_context;
	ComRef< IDirect3DVolumeTexture9 > m_d3dVolumeTexture;
	TextureFormat m_format;
	int m_width;
	int m_height;
	int m_depth;
};
		
	}
}

#endif	// traktor_render_VolumeTextureDx9_H
