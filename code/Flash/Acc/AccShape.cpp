#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Color4ub.h"
#include "Core/Timer/Timer.h"
#include "Flash/FlashCanvas.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashShape.h"
#include "Flash/FlashBitmap.h"
#include "Flash/Acc/AccShape.h"
#include "Flash/Acc/AccShapeResources.h"
#include "Flash/Acc/AccTextureCache.h"
#include "Flash/Acc/PathTesselator.h"
#include "Flash/Acc/Triangulator.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/Context/RenderContext.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

class ScopeTime
{
public:
	ScopeTime(const wchar_t* name)
	:	m_name(name)
	{
		m_timer.start();
	}

	~ScopeTime()
	{
		double T1 = m_timer.getElapsedTime();
		log::debug << L"Scope \"" << m_name << L"\" " << float(T1 * 1000.0f) << L" ms" << Endl;
	}

private:
	const wchar_t* m_name;
	Timer m_timer;
};

enum BatchFlags
{
	BfHaveSolid = 1,
	BfHaveTextured = 2
};

#pragma pack(1)
struct Vertex
{
	float pos[2];
	uint8_t color[4];
};
#pragma pack()

		}

AccShape::AccShape(AccShapeResources* shapeResources)
:	m_shapeResources(shapeResources)
,	m_vertexPool(0)
,	m_tesselationTriangleCount(0)
,	m_batchFlags(0)
,	m_needUpdate(true)
{
}

bool AccShape::createTesselation(const FlashShape& shape)
{
	AlignedVector< Segment > segments;
	PathTesselator tesselator;
	Triangulator triangulator;

	const std::list< Path >& paths = shape.getPaths();
	
	m_tesselationBatches.resize(0);
	m_tesselationBatches.reserve(paths.size());

	m_tesselationTriangleCount = 0;

	// Create triangles and lines through tesselation.
	for (std::list< Path >::const_iterator i = paths.begin(); i != paths.end(); ++i)
	{
		segments.resize(0);
		tesselator.tesselate(*i, segments);

		m_tesselationBatches.push_back(TesselationBatch());

		triangulator.triangulate(segments, m_tesselationBatches.back().triangles);
		m_tesselationTriangleCount += uint32_t(m_tesselationBatches.back().triangles.size());

		for (AlignedVector< Segment >::iterator j = segments.begin(); j != segments.end(); ++j)
		{
			if (!j->lineStyle)
				continue;

			Line line;
			line.v[0] = j->v[0];
			line.v[1] = j->v[1];
			line.lineStyle = j->lineStyle;
			m_tesselationBatches.back().lines.push_back(line);
		}
		m_tesselationTriangleCount += uint32_t(m_tesselationBatches.back().lines.size() * 2);
	}

	// Calculate bounding box.
	m_bounds.min.x = m_bounds.min.y =  std::numeric_limits< float >::max();
	m_bounds.max.x = m_bounds.max.y = -std::numeric_limits< float >::max();

	for (AlignedVector< TesselationBatch >::const_iterator i = m_tesselationBatches.begin(); i != m_tesselationBatches.end(); ++i)
	{
		for (AlignedVector< Triangle >::const_iterator j = i->triangles.begin(); j != i->triangles.end(); ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				m_bounds.min.x = min(m_bounds.min.x, j->v[k].x);
				m_bounds.min.y = min(m_bounds.min.y, j->v[k].y);
				m_bounds.max.x = max(m_bounds.max.x, j->v[k].x);
				m_bounds.max.y = max(m_bounds.max.y, j->v[k].y);
			}
		}
		for (AlignedVector< Line >::const_iterator j = i->lines.begin(); j != i->lines.end(); ++j)
		{
			for (int k = 0; k < 2; ++k)
			{
				m_bounds.min.x = min(m_bounds.min.x, j->v[k].x);
				m_bounds.min.y = min(m_bounds.min.y, j->v[k].y);
				m_bounds.max.x = max(m_bounds.max.x, j->v[k].x);
				m_bounds.max.y = max(m_bounds.max.y, j->v[k].y);
			}
		}
	}

	m_needUpdate = true;
	return true;
}

bool AccShape::createTesselation(const FlashCanvas& canvas)
{
	AlignedVector< Segment > segments;
	PathTesselator tesselator;
	Triangulator triangulator;

	const std::list< Path >& paths = canvas.getPaths();

	m_tesselationBatches.resize(0);
	m_tesselationBatches.reserve(paths.size());

	m_tesselationTriangleCount = 0;

	// Create triangles and lines through tesselation.
	for (std::list< Path >::const_iterator i = paths.begin(); i != paths.end(); ++i)
	{
		segments.resize(0);
		tesselator.tesselate(*i, segments);

		m_tesselationBatches.push_back(TesselationBatch());

		triangulator.triangulate(segments, m_tesselationBatches.back().triangles);
		m_tesselationTriangleCount += uint32_t(m_tesselationBatches.back().triangles.size());

		for (AlignedVector< Segment >::iterator j = segments.begin(); j != segments.end(); ++j)
		{
			if (!j->lineStyle)
				continue;

			Line line;
			line.v[0] = j->v[0];
			line.v[1] = j->v[1];
			line.lineStyle = j->lineStyle;
			m_tesselationBatches.back().lines.push_back(line);
		}
		m_tesselationTriangleCount += uint32_t(m_tesselationBatches.back().lines.size() * 2);
	}

	// Calculate bounding box.
	m_bounds.min.x = m_bounds.min.y =  std::numeric_limits< float >::max();
	m_bounds.max.x = m_bounds.max.y = -std::numeric_limits< float >::max();

	for (AlignedVector< TesselationBatch >::const_iterator i = m_tesselationBatches.begin(); i != m_tesselationBatches.end(); ++i)
	{
		for (AlignedVector< Triangle >::const_iterator j = i->triangles.begin(); j != i->triangles.end(); ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				m_bounds.min.x = min(m_bounds.min.x, j->v[k].x);
				m_bounds.min.y = min(m_bounds.min.y, j->v[k].y);
				m_bounds.max.x = max(m_bounds.max.x, j->v[k].x);
				m_bounds.max.y = max(m_bounds.max.y, j->v[k].y);
			}
		}
		for (AlignedVector< Line >::const_iterator j = i->lines.begin(); j != i->lines.end(); ++j)
		{
			for (int k = 0; k < 2; ++k)
			{
				m_bounds.min.x = min(m_bounds.min.x, j->v[k].x);
				m_bounds.min.y = min(m_bounds.min.y, j->v[k].y);
				m_bounds.max.x = max(m_bounds.max.x, j->v[k].x);
				m_bounds.max.y = max(m_bounds.max.y, j->v[k].y);
			}
		}
	}

	m_needUpdate = true;
	return true;
}

bool AccShape::updateRenderable(
	AccShapeVertexPool* vertexPool,
	AccTextureCache& textureCache,
	const FlashMovie& movie,
	const AlignedVector< FlashFillStyle >& fillStyles,
	const AlignedVector< FlashLineStyle >& lineStyles
)
{
	if (!m_tesselationTriangleCount)
		return false;

	// Do we need to update?
	if (!m_needUpdate)
		return true;

	// Free previous vertex range if any.
	if (m_vertexRange.vertexBuffer)
	{
		m_vertexPool->releaseRange(m_vertexRange);
		m_vertexPool = 0;
		m_vertexRange.vertexBuffer = 0;
	}

	// Allocate vertex range.
	if (!vertexPool->acquireRange(m_tesselationTriangleCount * 3, m_vertexRange))
		return false;

	T_ASSERT (m_vertexRange.vertexBuffer);
	m_vertexPool = vertexPool;

	const static Matrix33 textureTS = translate(0.5f, 0.5f) * scale(1.0f / 32768.0f, 1.0f / 32768.0f);

	uint32_t vertexOffset = m_vertexRange.offset;
	Matrix33 textureMatrix;

	m_renderBatches.reserve(m_tesselationBatches.size());
	m_batchFlags = 0;

	Vertex* vertex = static_cast< Vertex* >(m_vertexRange.vertexBuffer->lock(m_vertexRange.offset, m_tesselationTriangleCount * 3));
	if (!vertex)
		return false;

	for (AlignedVector< TesselationBatch >::const_iterator i = m_tesselationBatches.begin(); i != m_tesselationBatches.end(); ++i)
	{
		for (AlignedVector< Triangle >::const_iterator j = i->triangles.begin(); j != i->triangles.end(); ++j)
		{
			Color4ub color(255, 255, 255, 255);
			render::ITexture* texture = 0;

			if (j->fillStyle && j->fillStyle - 1 < uint16_t(fillStyles.size()))
			{
				const FlashFillStyle& style = fillStyles[j->fillStyle - 1];

				const AlignedVector< FlashFillStyle::ColorRecord >& colorRecords = style.getColorRecords();
				if (colorRecords.size() > 1)
				{
					// Create gradient texture.
					texture = textureCache.getGradientTexture(style);
					textureMatrix = textureTS * style.getGradientMatrix().inverse();
					m_batchFlags |= BfHaveTextured;
				}
				else if (colorRecords.size() == 1)
				{
					// Solid color.
					color.r = colorRecords.front().color.red;
					color.g = colorRecords.front().color.green;
					color.b = colorRecords.front().color.blue;
					color.a = colorRecords.front().color.alpha;
					m_batchFlags |= BfHaveSolid;
				}

				const FlashBitmap* bitmap = movie.getBitmap(style.getFillBitmap());
				if (bitmap)
				{
					T_ASSERT_M (!texture, L"Cannot combine gradients and bitmaps");
					texture = textureCache.getBitmapTexture(*bitmap);
					textureMatrix =
						scale(
							1.0f / bitmap->getWidth(),
							1.0f / bitmap->getHeight()
						) *
						style.getFillBitmapMatrix().inverse();
					m_batchFlags |= BfHaveTextured;
				}
			}

			if (m_renderBatches.empty() || m_renderBatches.back().texture != texture)
			{
				m_renderBatches.push_back(RenderBatch());
				m_renderBatches.back().primitives.setNonIndexed(render::PtTriangles, vertexOffset, 0);
				m_renderBatches.back().texture = texture;
				m_renderBatches.back().textureMatrix = textureMatrix;
			}

			for (int k = 0; k < 3; ++k)
			{
				vertex->pos[0] = j->v[k].x;
				vertex->pos[1] = j->v[k].y;
				vertex->color[0] = color.r;
				vertex->color[1] = color.g;
				vertex->color[2] = color.b;
				vertex->color[3] = color.a;
				vertex++;
			}

			m_renderBatches.back().primitives.count++;

			vertexOffset += 3;
		}

		for (AlignedVector< Line >::const_iterator j = i->lines.begin(); j != i->lines.end(); ++j)
		{
			Color4ub color(255, 255, 255, 255);
			float width = 0.0f;

			if (j->lineStyle && j->lineStyle - 1 < uint16_t(lineStyles.size()))
			{
				const FlashLineStyle& style = lineStyles[j->lineStyle - 1];

				color.r = style.getLineColor().blue;
				color.g = style.getLineColor().green;
				color.b = style.getLineColor().red;
				color.a = style.getLineColor().alpha;

				width = style.getLineWidth();
			}

			if (width <= 0.0f)
				continue;

			m_batchFlags |= BfHaveSolid;

			if (m_renderBatches.empty() || m_renderBatches.back().texture)
			{
				m_renderBatches.push_back(RenderBatch());
				m_renderBatches.back().primitives.setNonIndexed(render::PtTriangles, vertexOffset, 0);
				m_renderBatches.back().texture = 0;
			}

			Vector2 perp = (j->v[1] - j->v[0]).normalized().perpendicular() * width / 2.0f;
			Vector2 v[] = 
			{
				j->v[0] - perp,
				j->v[0] + perp,
				j->v[1] + perp,
				j->v[1] - perp
			};

			const static int c_indices[6] = { 0, 1, 2, 0, 2, 3 };
			for (int k = 0; k < 6; ++k)
			{
				const Vector2& vr = v[c_indices[k]];

				vertex->pos[0] = vr.x;
				vertex->pos[1] = vr.y;
				vertex->color[2] = color.r;
				vertex->color[1] = color.g;
				vertex->color[0] = color.b;
				vertex->color[3] = color.a;

				vertex++;
			}

			m_renderBatches.back().primitives.count += 2;

			vertexOffset += 6;
		}
	}

	m_vertexRange.vertexBuffer->unlock();

	// Some shapes doesn't expose styles, such as glyphs, but are
	// assumed to be solids.
	if (!m_batchFlags)
		m_batchFlags |= BfHaveSolid;

	m_needUpdate = false;
	return true;
}

AccShape::~AccShape()
{
	destroy();
}

void AccShape::destroy()
{
	m_shapeResources = 0;

	if (m_vertexPool)
	{
		m_vertexPool->releaseRange(m_vertexRange);
		m_vertexPool = 0;
		m_vertexRange = AccShapeVertexPool::Range();
	}

	m_tesselationBatches.clear();
	m_renderBatches.clear();
}

void AccShape::render(
	render::RenderContext* renderContext,
	const Matrix33& transform,
	const Vector4& frameSize,
	const Vector4& viewSize,
	const Vector4& viewOffset,
	float screenOffsetScale,
	const SwfCxTransform& cxform,
	bool maskWrite,
	bool maskIncrement,
	uint8_t maskReference
)
{
	if (!m_vertexRange.vertexBuffer)
		return;

	Matrix44 m(
		transform.e11, transform.e12, transform.e13, 0.0f,
		transform.e21, transform.e22, transform.e23, 0.0f,
		transform.e31, transform.e32, transform.e33, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	Ref< render::Shader > shaderSolid, shaderTextured;
	if (!maskWrite)
	{
		if (maskReference == 0)
		{
			shaderSolid = m_shapeResources->m_shaderSolid;
			shaderTextured = m_shapeResources->m_shaderTextured;
		}
		else
		{
			shaderSolid = m_shapeResources->m_shaderSolidMask;
			shaderTextured = m_shapeResources->m_shaderTexturedMask;
		}
	}
	else
	{
		if (maskIncrement)
			shaderSolid = m_shapeResources->m_shaderIncrementMask;
		else
			shaderSolid = m_shapeResources->m_shaderDecrementMask;
		shaderTextured = 0;
	}

	if (shaderSolid && !shaderSolid->getCurrentProgram())
		shaderSolid = 0;
	if (shaderTextured && !shaderTextured->getCurrentProgram())
		shaderTextured = 0;

	if (!shaderSolid && !shaderTextured)
		return;

	if (shaderSolid && (m_batchFlags & BfHaveSolid) != 0)
	{
		render::NullRenderBlock* renderBlockSolid = renderContext->alloc< render::NullRenderBlock >("Flash AccShape; set solid parameters");
		renderBlockSolid->program = shaderSolid->getCurrentProgram();
		renderBlockSolid->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlockSolid->programParams->beginParameters(renderContext);
		renderBlockSolid->programParams->setMatrixParameter(m_shapeResources->m_handleTransform, m);
		renderBlockSolid->programParams->setVectorParameter(m_shapeResources->m_handleFrameSize, frameSize);
		renderBlockSolid->programParams->setVectorParameter(m_shapeResources->m_handleViewSize, viewSize);
		renderBlockSolid->programParams->setVectorParameter(m_shapeResources->m_handleViewOffset, viewOffset);
		renderBlockSolid->programParams->setFloatParameter(m_shapeResources->m_handleScreenOffsetScale, screenOffsetScale);
		renderBlockSolid->programParams->setVectorParameter(m_shapeResources->m_handleCxFormMul, Vector4(cxform.red[0], cxform.green[0], cxform.blue[0], cxform.alpha[0]));
		renderBlockSolid->programParams->setVectorParameter(m_shapeResources->m_handleCxFormAdd, Vector4(cxform.red[1], cxform.green[1], cxform.blue[1], cxform.alpha[1]));
		renderBlockSolid->programParams->setStencilReference(maskReference);
		renderBlockSolid->programParams->endParameters(renderContext);
		renderContext->draw(render::RfOverlay, renderBlockSolid);
	}

	if (shaderTextured && (m_batchFlags & BfHaveTextured) != 0)
	{
		render::NullRenderBlock* renderBlockTextured = renderContext->alloc< render::NullRenderBlock >("Flash AccShape; set textured parameters");
		renderBlockTextured->program = shaderTextured->getCurrentProgram();
		renderBlockTextured->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlockTextured->programParams->beginParameters(renderContext);
		renderBlockTextured->programParams->setMatrixParameter(m_shapeResources->m_handleTransform, m);
		renderBlockTextured->programParams->setVectorParameter(m_shapeResources->m_handleFrameSize, frameSize);
		renderBlockTextured->programParams->setVectorParameter(m_shapeResources->m_handleViewSize, viewSize);
		renderBlockTextured->programParams->setVectorParameter(m_shapeResources->m_handleViewOffset, viewOffset);
		renderBlockTextured->programParams->setFloatParameter(m_shapeResources->m_handleScreenOffsetScale, screenOffsetScale);
		renderBlockTextured->programParams->setVectorParameter(m_shapeResources->m_handleCxFormMul, Vector4(cxform.red[0], cxform.green[0], cxform.blue[0], cxform.alpha[0]));
		renderBlockTextured->programParams->setVectorParameter(m_shapeResources->m_handleCxFormAdd, Vector4(cxform.red[1], cxform.green[1], cxform.blue[1], cxform.alpha[1]));
		renderBlockTextured->programParams->setStencilReference(maskReference);
		renderBlockTextured->programParams->endParameters(renderContext);
		renderContext->draw(render::RfOverlay, renderBlockTextured);
	}
	
	for (AlignedVector< RenderBatch >::iterator i = m_renderBatches.begin(); i != m_renderBatches.end(); ++i)
	{
		if (!i->texture)
		{
			if (shaderSolid)
			{
				render::NonIndexedRenderBlock* renderBlock = renderContext->alloc< render::NonIndexedRenderBlock >("Flash AccShape; draw solid batch");
				renderBlock->program = shaderSolid->getCurrentProgram();
				renderBlock->vertexBuffer = m_vertexRange.vertexBuffer;
				renderBlock->primitive = i->primitives.type;
				renderBlock->offset = i->primitives.offset;
				renderBlock->count = i->primitives.count;
				renderContext->draw(render::RfOverlay, renderBlock);
			}
		}
		else
		{
			if (shaderTextured)
			{
				Matrix44 textureMatrix(
					i->textureMatrix.e11, i->textureMatrix.e12, i->textureMatrix.e13, 0.0f,
					i->textureMatrix.e21, i->textureMatrix.e22, i->textureMatrix.e23, 0.0f,
					i->textureMatrix.e31, i->textureMatrix.e32, i->textureMatrix.e33, 0.0f,
					0.0f, 0.0f, 0.0, 0.0f
				);

				render::NonIndexedRenderBlock* renderBlock = renderContext->alloc< render::NonIndexedRenderBlock >("Flash AccShape; draw textured batch");
				renderBlock->program = shaderTextured->getCurrentProgram();
				renderBlock->vertexBuffer = m_vertexRange.vertexBuffer;
				renderBlock->primitive = i->primitives.type;
				renderBlock->offset = i->primitives.offset;
				renderBlock->count = i->primitives.count;
				renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
				renderBlock->programParams->beginParameters(renderContext);
				renderBlock->programParams->setTextureParameter(m_shapeResources->m_handleTexture, i->texture);
				renderBlock->programParams->setMatrixParameter(m_shapeResources->m_handleTextureMatrix, textureMatrix);
				renderBlock->programParams->endParameters(renderContext);
				renderContext->draw(render::RfOverlay, renderBlock);
			}
		}
	}
}

void AccShape::preBuild()
{
	if (m_vertexRange.vertexBuffer)
		m_needUpdate = !m_vertexRange.vertexBuffer->isContentValid();
	else
		m_needUpdate = false;
}

	}
}
