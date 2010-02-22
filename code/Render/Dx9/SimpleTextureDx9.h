#ifndef traktor_render_SimpleTextureDx9_H
#define traktor_render_SimpleTextureDx9_H

#include "Core/Misc/ComRef.h"
#include "Render/ISimpleTexture.h"
#include "Render/Dx9/IResourceDx9.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX9_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ResourceManagerDx9;
struct SimpleTextureCreateDesc;

/*!
 * \ingroup DX9 Xbox360
 */
class T_DLLCLASS SimpleTextureDx9
:	public ISimpleTexture
,	public IResourceDx9
{
	T_RTTI_CLASS;

public:
	SimpleTextureDx9(ResourceManagerDx9* resourceManager);

	virtual ~SimpleTextureDx9();

	bool create(IDirect3DDevice9* d3dDevice, const SimpleTextureCreateDesc& desc);

	// \name ISimpleTexture
	// \{

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	virtual bool lock(int level, Lock& lock);

	virtual void unlock(int level);

	// \}

	// \name IResourceDx9
	// \{

	virtual HRESULT lostDevice();

	virtual HRESULT resetDevice(IDirect3DDevice9* d3dDevice);

	// \}

	IDirect3DBaseTexture9* getD3DBaseTexture() const { return m_d3dTexture; }

private:
	Ref< ResourceManagerDx9 > m_resourceManager;
	ComRef< IDirect3DTexture9 > m_d3dTexture;
	TextureFormat m_format;
	int m_width;
	int m_height;
	void* m_lock;
};
		
	}
}

#endif	// traktor_render_SimpleTextureDx9_H
