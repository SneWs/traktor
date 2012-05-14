#ifndef traktor_render_RenderViewSw_H
#define traktor_render_RenderViewSw_H

#include <list>
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Core/Misc/AutoPtr.h"
#include "Graphics/ISurface.h"
#include "Render/IRenderView.h"
#include "Render/Sw/VaryingUtils.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_SW_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace graphics
	{

class IGraphicsSystem;

	}

	namespace render
	{

class RenderSystemSw;
class VertexBufferSw;
class IndexBufferSw;
class ProgramSw;
class Processor;

/*!
 * \ingroup SW
 */
class T_DLLCLASS RenderViewSw : public IRenderView
{
	T_RTTI_CLASS;

public:
	RenderViewSw(RenderSystemSw* renderSystem, graphics::IGraphicsSystem* graphicsSystem, Processor* processor);

	virtual ~RenderViewSw();

	virtual bool nextEvent(RenderEvent& outEvent);

	virtual void close();

	virtual bool reset(const RenderViewDefaultDesc& desc);

	virtual bool reset(int32_t width, int32_t height);

	virtual int getWidth() const;

	virtual int getHeight() const;

	virtual bool isActive() const;

	virtual bool isFullScreen() const;

	virtual bool setGamma(float gamma);

	virtual void setViewport(const Viewport& viewport);

	virtual Viewport getViewport();

	virtual SystemWindow getSystemWindow();

	virtual bool begin(EyeType eye);

	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget);

	virtual void clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil);

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives);

	virtual void end();

	virtual void present();

	virtual void pushMarker(const char* const marker);

	virtual void popMarker();

	virtual void getStatistics(RenderViewStatistics& outStatistics) const;

private:
	struct RenderState
	{
		Viewport viewPort;
		uint32_t width;
		uint32_t height;
		uint16_t* colorTarget;
		uint32_t colorTargetPitch;	//< Color target pitch in bytes.
		uint16_t* depthTarget;
		uint32_t depthTargetPitch;	//< Depth target pitch in bytes.
	};

	struct FragmentContext
	{
		T_ALIGN16 varying_data_t vertexVaryings[3];			//< From vertex stream for each triangle
		T_ALIGN16 varying_data_t interpolatorVaryings[3];	//< Post vertex program
		T_ALIGN16 varying_data_t clippedVaryings[8];		//< Post view plane clip
		uint32_t clippedCount;
		Vector2 screen[8];
		const Vector2* triangle;
		uint32_t indices[3];
		float baryDenom[3];
		float baryOffset[3];
		bool depthEnable;
		bool depthWriteEnable;
		bool blendEnable;

		FragmentContext()
		:	clippedCount(0)
		,	triangle(0)
		,	depthEnable(false)
		,	depthWriteEnable(false)
		,	blendEnable(false)
		{
			std::memset(vertexVaryings, 0, sizeof(vertexVaryings));
			std::memset(interpolatorVaryings, 0, sizeof(interpolatorVaryings));
			std::memset(clippedVaryings, 0, sizeof(clippedVaryings));
		}
	};

	Ref< RenderSystemSw > m_renderSystem;
	Ref< graphics::IGraphicsSystem > m_graphicsSystem;
	Ref< Processor > m_processor;

	/*! \name Primary render target surfaces. */
	//@{
	Ref< graphics::ISurface > m_frameBufferSurface;
	graphics::SurfaceDesc m_frameBufferSurfaceDesc;
	AutoArrayPtr< uint16_t > m_depthBuffer;
	//@}

	std::list< RenderState > m_renderStateStack;
	Ref< VertexBufferSw > m_currentVertexBuffer;
	Ref< IndexBufferSw > m_currentIndexBuffer;
	Ref< ProgramSw > m_currentProgram;
	Viewport m_viewPort;
	Vector4 m_targetSize;

	void fetchVertex(uint32_t index, varying_data_t& outVertexVarying) const;

	void executeVertexShader(const varying_data_t& vertexVarying, varying_data_t& outInterpolatorVarying) /*const*/;

	void clipPlanes(FragmentContext& context, uint32_t vertexCount) const;

	void projectScreen(FragmentContext& context) const;

	void drawIndexed(const Primitives& primitives);

	void drawNonIndexed(const Primitives& primitives);

	void triangleShadeOpaque(const FragmentContext& context, int x1, int x2, int y);

	void triangleShadeBlend(const FragmentContext& context, int x1, int x2, int y);
};

	}
}

#endif	// traktor_render_RenderViewSw_H
