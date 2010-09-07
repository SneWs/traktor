#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Color.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashShape.h"
#include "Flash/FlashBitmap.h"
#include "Flash/Acc/AccShape.h"
#include "Flash/Acc/AccTextureCache.h"
#include "Flash/Acc/PathTesselator.h"
#include "Flash/Acc/Triangulator.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const Guid c_guidShaderSolid(L"{4F6F6CCE-97EC-624D-96B7-842F1D99D060}");
const Guid c_guidShaderTextured(L"{049F4B08-1A54-DB4C-86CC-B533BCFFC65D}");
const Guid c_guidShaderSolidMask(L"{D46877B9-0F90-3A42-AB2D-7346AA607233}");
const Guid c_guidShaderTexturedMask(L"{5CDDBEC8-1629-0A4E-ACE5-C8186072D694}");
const Guid c_guidShaderIncrementMask(L"{8DCBCF05-4640-884E-95AC-F090510788F4}");
const Guid c_guidShaderDecrementMask(L"{57F6F4DF-F4EE-6740-907C-027A3A2596D7}");

#pragma pack(1)
struct Vertex
{
	float pos[2];
	uint8_t color[4];
};
#pragma pack()

struct ConstructBatch
{
	AlignedVector< Triangle > triangles;
	AlignedVector< Line > lines;
};

bool s_handleInitialized = false;
render::handle_t s_handleTransform;
render::handle_t s_handleFrameSize;
render::handle_t s_handleViewSize;
render::handle_t s_handleViewOffset;
render::handle_t s_handleCxFormMul;
render::handle_t s_handleCxFormAdd;
render::handle_t s_handleTexture;
render::handle_t s_handleTextureMatrix;

		}

bool AccShape::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	AccTextureCache& textureCache,
	const FlashMovie& movie,
	const FlashShape& shape
)
{
	if (!s_handleInitialized)
	{
		s_handleTransform = render::getParameterHandle(L"Transform");
		s_handleFrameSize = render::getParameterHandle(L"FrameSize");
		s_handleViewSize = render::getParameterHandle(L"ViewSize");
		s_handleViewOffset = render::getParameterHandle(L"ViewOffset");
		s_handleCxFormMul = render::getParameterHandle(L"CxFormMul");
		s_handleCxFormAdd = render::getParameterHandle(L"CxFormAdd");
		s_handleTexture = render::getParameterHandle(L"Texture");
		s_handleTextureMatrix = render::getParameterHandle(L"TextureMatrix");
		s_handleInitialized = true;
	}

	m_shaderSolid = c_guidShaderSolid;
	if (!resourceManager->bind(m_shaderSolid))
		return false;

	m_shaderTextured = c_guidShaderTextured;
	if (!resourceManager->bind(m_shaderTextured))
		return false;

	m_shaderSolidMask = c_guidShaderSolidMask;
	if (!resourceManager->bind(m_shaderSolidMask))
		return false;

	m_shaderTexturedMask = c_guidShaderTexturedMask;
	if (!resourceManager->bind(m_shaderTexturedMask))
		return false;

	m_shaderIncrementMask = c_guidShaderIncrementMask;
	if (!resourceManager->bind(m_shaderIncrementMask))
		return false;

	m_shaderDecrementMask = c_guidShaderDecrementMask;
	if (!resourceManager->bind(m_shaderDecrementMask))
		return false;

	m_bounds.min.x = m_bounds.min.y =  std::numeric_limits< float >::max();
	m_bounds.max.x = m_bounds.max.y = -std::numeric_limits< float >::max();

	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(Vertex, pos)));
	vertexElements.push_back(render::VertexElement(render::DuColor, render::DtByte4N, offsetof(Vertex, color), 0));
	T_ASSERT (render::getVertexSize(vertexElements) == sizeof(Vertex));

	AlignedVector< ConstructBatch > batches;
	uint32_t totalTriangleCount = 0;

	const std::list< Path >& paths = shape.getPaths();
	for (std::list< Path >::const_iterator i = paths.begin(); i != paths.end(); ++i)
	{
		AlignedVector< Segment > segments;
		PathTesselator().tesselate(*i, segments);

		batches.push_back(ConstructBatch());

		Triangulator().triangulate(segments, batches.back().triangles);
		totalTriangleCount += uint32_t(batches.back().triangles.size());

		for (AlignedVector< Segment >::iterator j = segments.begin(); j != segments.end(); ++j)
		{
			if (!j->lineStyle)
				continue;

			Line line;
			line.v[0] = j->v[0];
			line.v[1] = j->v[1];
			line.lineStyle = j->lineStyle;
			batches.back().lines.push_back(line);
		}
		totalTriangleCount += uint32_t(batches.back().lines.size() * 2);
	}	

	if (!totalTriangleCount)
		return true;

	m_vertexBuffer = renderSystem->createVertexBuffer(vertexElements, totalTriangleCount * 3 * sizeof(Vertex), false);
	if (!m_vertexBuffer)
		return false;

	Vertex* vertex = static_cast< Vertex* >(m_vertexBuffer->lock());
	if (!vertex)
		return false;

	uint32_t vertexOffset = 0;

	const AlignedVector< FlashFillStyle >& fillStyles = shape.getFillStyles();
	const AlignedVector< FlashLineStyle >& lineStyles = shape.getLineStyles();

	for (AlignedVector< ConstructBatch >::iterator i = batches.begin(); i != batches.end(); ++i)
	{
		for (AlignedVector< Triangle >::iterator j = i->triangles.begin(); j != i->triangles.end(); ++j)
		{
			Matrix33 textureMatrix = Matrix33::identity();
			Color color(255, 255, 255, 255);
			render::ITexture* texture = 0;

			if (j->fillStyle && j->fillStyle - 1 < uint16_t(fillStyles.size()))
			{
				const FlashFillStyle& style = fillStyles[j->fillStyle - 1];

				const AlignedVector< FlashFillStyle::ColorRecord >& colorRecords = style.getColorRecords();
				if (colorRecords.size() > 1)
				{
					// Create gradient texture.
					texture = textureCache.getGradientTexture(style);
					textureMatrix = translate(0.5f, 0.5f) * scale(1.0f / 32768.0f, 1.0f / 32768.0f) * style.getGradientMatrix().inverse();
				}
				else if (colorRecords.size() == 1)
				{
					// Solid color.
					color.r = colorRecords.front().color.blue;
					color.g = colorRecords.front().color.green;
					color.b = colorRecords.front().color.red;
					color.a = colorRecords.front().color.alpha;
				}

				const FlashBitmap* bitmap = movie.getBitmap(style.getFillBitmap());
				if (bitmap)
				{
					T_ASSERT_M (!texture, L"Cannot combine gradients and bitmaps");
					texture = textureCache.getBitmapTexture(*bitmap);
					textureMatrix = scale(
						1.0f / bitmap->getOriginalWidth(),
						1.0f / bitmap->getOriginalHeight()
					) *
					style.getFillBitmapMatrix().inverse();
				}
			}

			if (m_batches.empty() || m_batches.back().texture != texture)
			{
				m_batches.push_back(Batch());
				m_batches.back().primitives.setNonIndexed(render::PtTriangles, vertexOffset, 0);
				m_batches.back().texture = texture;
				m_batches.back().textureMatrix = textureMatrix;
			}

			for (int k = 0; k < 3; ++k)
			{
				m_bounds.min.x = min(m_bounds.min.x, j->v[k].x);
				m_bounds.min.y = min(m_bounds.min.y, j->v[k].y);
				m_bounds.max.x = max(m_bounds.max.x, j->v[k].x);
				m_bounds.max.y = max(m_bounds.max.y, j->v[k].y);

				vertex->pos[0] = j->v[k].x;
				vertex->pos[1] = j->v[k].y;
				vertex->color[2] = color.r;
				vertex->color[1] = color.g;
				vertex->color[0] = color.b;
				vertex->color[3] = color.a;

				vertex++;
			}

			m_batches.back().primitives.count++;

			vertexOffset += 3;
		}

		for (AlignedVector< Line >::iterator j = i->lines.begin(); j != i->lines.end(); ++j)
		{
			Color color(255, 255, 255, 255);
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

			if (m_batches.empty() || m_batches.back().texture)
			{
				m_batches.push_back(Batch());
				m_batches.back().primitives.setNonIndexed(render::PtTriangles, vertexOffset, 0);
				m_batches.back().texture = 0;
			}

			Vector2 perp = (j->v[1] - j->v[0]).normalized().perpendicular() * width / 2.0f;
			Vector2 v[] = 
			{
				j->v[0] - perp,
				j->v[0] + perp,
				j->v[1] + perp,
				j->v[1] - perp
			};

			const int c_indices[6] = { 0, 1, 2, 0, 2, 3 };
			for (int k = 0; k < 6; ++k)
			{
				m_bounds.min.x = min(m_bounds.min.x, v[c_indices[k]].x);
				m_bounds.min.y = min(m_bounds.min.y, v[c_indices[k]].y);
				m_bounds.max.x = max(m_bounds.max.x, v[c_indices[k]].x);
				m_bounds.max.y = max(m_bounds.max.y, v[c_indices[k]].y);

				vertex->pos[0] = v[c_indices[k]].x;
				vertex->pos[1] = v[c_indices[k]].y;
				vertex->color[2] = color.r;
				vertex->color[1] = color.g;
				vertex->color[0] = color.b;
				vertex->color[3] = color.a;

				vertex++;
			}

			m_batches.back().primitives.count += 2;

			vertexOffset += 6;
		}
	}

	m_vertexBuffer->unlock();

	return true;
}

void AccShape::destroy()
{
	if (m_vertexBuffer)
	{
		m_vertexBuffer->destroy();
		m_vertexBuffer = 0;
	}
}

void AccShape::render(
	render::RenderContext* renderContext,
	const FlashShape& shape,
	const Matrix33& transform,
	const Vector4& frameSize,
	const Vector4& viewSize,
	const Vector4& viewOffset,
	const SwfCxTransform& cxform,
	bool maskWrite,
	bool maskIncrement,
	uint8_t maskReference
)
{
	if (!m_vertexBuffer)
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
			shaderSolid = m_shaderSolid;
			shaderTextured = m_shaderTextured;
		}
		else
		{
			shaderSolid = m_shaderSolidMask;
			shaderTextured = m_shaderTexturedMask;
		}
	}
	else
	{
		if (maskIncrement)
			shaderSolid = m_shaderIncrementMask;
		else
			shaderSolid = m_shaderDecrementMask;

		shaderTextured = 0;
	}

	if (shaderSolid && !shaderSolid->getCurrentProgram())
		shaderSolid = 0;
	if (shaderTextured && !shaderTextured->getCurrentProgram())
		shaderTextured = 0;

	if (!shaderSolid && !shaderTextured)
		return;

	if (shaderSolid)
	{
		render::NullRenderBlock* renderBlockSolid = renderContext->alloc< render::NullRenderBlock >("Flash AccShape; set solid parameters");
		renderBlockSolid->program = shaderSolid->getCurrentProgram();
		renderBlockSolid->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlockSolid->programParams->beginParameters(renderContext);
		renderBlockSolid->programParams->setMatrixParameter(s_handleTransform, m);
		renderBlockSolid->programParams->setVectorParameter(s_handleFrameSize, frameSize);
		renderBlockSolid->programParams->setVectorParameter(s_handleViewSize, viewSize);
		renderBlockSolid->programParams->setVectorParameter(s_handleViewOffset, viewOffset);
		renderBlockSolid->programParams->setVectorParameter(s_handleCxFormMul, Vector4(cxform.red[0], cxform.green[0], cxform.blue[0], cxform.alpha[0]));
		renderBlockSolid->programParams->setVectorParameter(s_handleCxFormAdd, Vector4(cxform.red[1], cxform.green[1], cxform.blue[1], cxform.alpha[1]));
		renderBlockSolid->programParams->setStencilReference(maskReference);
		renderBlockSolid->programParams->endParameters(renderContext);
		renderContext->draw(render::RfOverlay, renderBlockSolid);
	}

	if (shaderTextured)
	{
		render::NullRenderBlock* renderBlockTextured = renderContext->alloc< render::NullRenderBlock >("Flash AccShape; set textured parameters");
		renderBlockTextured->program = shaderTextured->getCurrentProgram();
		renderBlockTextured->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlockTextured->programParams->beginParameters(renderContext);
		renderBlockTextured->programParams->setMatrixParameter(s_handleTransform, m);
		renderBlockTextured->programParams->setVectorParameter(s_handleFrameSize, frameSize);
		renderBlockTextured->programParams->setVectorParameter(s_handleViewSize, viewSize);
		renderBlockTextured->programParams->setVectorParameter(s_handleViewOffset, viewOffset);
		renderBlockTextured->programParams->setVectorParameter(s_handleCxFormMul, Vector4(cxform.red[0], cxform.green[0], cxform.blue[0], cxform.alpha[0]));
		renderBlockTextured->programParams->setVectorParameter(s_handleCxFormAdd, Vector4(cxform.red[1], cxform.green[1], cxform.blue[1], cxform.alpha[1]));
		renderBlockTextured->programParams->setStencilReference(maskReference);
		renderBlockTextured->programParams->endParameters(renderContext);
		renderContext->draw(render::RfOverlay, renderBlockTextured);
	}
	
	for (AlignedVector< Batch >::iterator i = m_batches.begin(); i != m_batches.end(); ++i)
	{
		if (!i->texture)
		{
			if (shaderSolid)
			{
				render::NonIndexedRenderBlock* renderBlock = renderContext->alloc< render::NonIndexedRenderBlock >("Flash AccShape; draw solid batch");
				renderBlock->program = shaderSolid->getCurrentProgram();
				renderBlock->vertexBuffer = m_vertexBuffer;
				renderBlock->primitive = i->primitives.type;
				renderBlock->offset = i->primitives.offset;
				renderBlock->count = i->primitives.count;
				renderContext->draw(render::RfOverlay, renderBlock);
			}
		}
		else if (shaderTextured)
		{
			Matrix44 textureMatrix(
				i->textureMatrix.e11, i->textureMatrix.e12, i->textureMatrix.e13, 0.0f,
				i->textureMatrix.e21, i->textureMatrix.e22, i->textureMatrix.e23, 0.0f,
				i->textureMatrix.e31, i->textureMatrix.e32, i->textureMatrix.e33, 0.0f,
				0.0f, 0.0f, 0.0, 0.0f
			);

			render::NonIndexedRenderBlock* renderBlock = renderContext->alloc< render::NonIndexedRenderBlock >("Flash AccShape; draw textured batch");
			renderBlock->program = shaderTextured->getCurrentProgram();
			renderBlock->vertexBuffer = m_vertexBuffer;
			renderBlock->primitive = i->primitives.type;
			renderBlock->offset = i->primitives.offset;
			renderBlock->count = i->primitives.count;
			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->programParams->beginParameters(renderContext);
			renderBlock->programParams->setTextureParameter(s_handleTexture, i->texture);
			renderBlock->programParams->setMatrixParameter(s_handleTextureMatrix, textureMatrix);
			renderBlock->programParams->endParameters(renderContext);
			renderContext->draw(render::RfOverlay, renderBlock);
		}
	}
}

	}
}
