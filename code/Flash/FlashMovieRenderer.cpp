#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/Split.h"
#include "Flash/FlashDictionary.h"
#include "Flash/FlashButton.h"
#include "Flash/FlashButtonInstance.h"
#include "Flash/FlashEdit.h"
#include "Flash/FlashEditInstance.h"
#include "Flash/FlashFont.h"
#include "Flash/FlashMorphShapeInstance.h"
#include "Flash/FlashMovieRenderer.h"
#include "Flash/FlashShape.h"
#include "Flash/FlashShapeInstance.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashText.h"
#include "Flash/FlashTextInstance.h"
#include "Flash/IDisplayRenderer.h"
#include "Flash/TextLayout.h"
#include "Flash/Action/ActionContext.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

SwfCxTransform concateCxTransform(const SwfCxTransform& cxt1, const SwfCxTransform& cxt2)
{
	SwfCxTransform cxtr = 
	{
		{ cxt1.red[0]   * cxt2.red[0]  , clamp(cxt1.red[1]   + cxt1.red[0]   * cxt2.red[1],   0.0f, 1.0f) },
		{ cxt1.green[0] * cxt2.green[0], clamp(cxt1.green[1] + cxt1.green[0] * cxt2.green[1], 0.0f, 1.0f) },
		{ cxt1.blue[0]  * cxt2.blue[0] , clamp(cxt1.blue[1]  + cxt1.blue[0]  * cxt2.blue[1],  0.0f, 1.0f) },
		{ cxt1.alpha[0] * cxt2.alpha[0], clamp(cxt1.alpha[1] + cxt1.alpha[0] * cxt2.alpha[1], 0.0f, 1.0f) }
	};
	return cxtr;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashMovieRenderer", FlashMovieRenderer, Object)

FlashMovieRenderer::FlashMovieRenderer(IDisplayRenderer* displayRenderer)
:	m_displayRenderer(displayRenderer)
{
}

void FlashMovieRenderer::renderFrame(
	FlashSpriteInstance* movieInstance,
	const Aabb2& frameBounds,
	float viewWidth,
	float viewHeight,
	const Vector4& viewOffset
)
{
	const SwfColor& backgroundColor = movieInstance->getDisplayList().getBackgroundColor();
	m_displayRenderer->begin(
		*movieInstance->getContext()->getDictionary(),
		backgroundColor,
		frameBounds,
		viewWidth,
		viewHeight,
		viewOffset
	);

	renderSprite(
		movieInstance->getContext()->getDictionary(),
		movieInstance,
		Matrix33::identity(),
		movieInstance->getColorTransform(),
		false
	);

	m_displayRenderer->end();
}

void FlashMovieRenderer::renderSprite(
	FlashDictionary* dictionary,
	FlashSpriteInstance* spriteInstance,
	const Matrix33& transform,
	const SwfCxTransform& cxTransform,
	bool renderAsMask
)
{
	if (!spriteInstance->isVisible() && !renderAsMask)
		return;

	const FlashDisplayList& displayList = spriteInstance->getDisplayList();
	const FlashDisplayList::layer_map_t& layers = displayList.getLayers();

	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); )
	{
		const FlashDisplayList::Layer& layer = i->second;
		if (!layer.instance)
		{
			++i;
			continue;
		}

		if (!layer.clipDepth)
		{
			renderCharacter(
				dictionary,
				layer.instance,
				transform,
				cxTransform
			);
			++i;
		}
		else
		{
			m_displayRenderer->beginMask(true);

			renderCharacter(
				dictionary,
				layer.instance,
				transform,
				cxTransform
			);

			m_displayRenderer->endMask();

			for (++i; i != layers.end(); ++i)
			{
				if (layer.clipDepth > 0 && i->first > layer.clipDepth)
					break;

				const FlashDisplayList::Layer& clippedLayer = i->second;
				if (!clippedLayer.instance)
					continue;

				renderCharacter(
					dictionary,
					clippedLayer.instance,
					transform,
					cxTransform
				);
			}

			m_displayRenderer->beginMask(false);

			renderCharacter(
				dictionary,
				layer.instance,
				transform,
				cxTransform
			);

			m_displayRenderer->endMask();
		}
	}

	FlashCanvas* canvas = spriteInstance->getCanvas();
	if (canvas)
		m_displayRenderer->renderCanvas(
			*dictionary,
			transform,
			*canvas,
			cxTransform
		);
}

void FlashMovieRenderer::renderCharacter(
	FlashDictionary* dictionary,
	FlashCharacterInstance* characterInstance,
	const Matrix33& transform,
	const SwfCxTransform& cxTransform
)
{
	// Don't render completely transparent shapes.
	if (cxTransform.alpha[0] + cxTransform.alpha[1] <= FUZZY_EPSILON)
		return;

	// Render basic shapes.
	FlashShapeInstance* shapeInstance = dynamic_type_cast< FlashShapeInstance* >(characterInstance);
	if (shapeInstance)
	{
		m_displayRenderer->renderShape(
			*dictionary,
			transform * shapeInstance->getTransform(),
			*shapeInstance->getShape(),
			concateCxTransform(cxTransform, characterInstance->getColorTransform())
		);
		return;
	}

	// Render morph shapes.
	FlashMorphShapeInstance* morphInstance = dynamic_type_cast< FlashMorphShapeInstance* >(characterInstance);
	if (morphInstance)
	{
		m_displayRenderer->renderMorphShape(
			*dictionary,
			transform * morphInstance->getTransform(),
			*morphInstance->getShape(),
			concateCxTransform(cxTransform, characterInstance->getColorTransform())
		);
		return;
	}

	// Render static texts.
	FlashTextInstance* textInstance = dynamic_type_cast< FlashTextInstance* >(characterInstance);
	if (textInstance)
	{
		if (!textInstance->isVisible())
			return;

		const FlashText* text = textInstance->getText();

		Matrix33 textTransform = transform * textInstance->getTransform() * text->getTextMatrix();

		const AlignedVector< FlashText::Character >& characters = text->getCharacters();
		for (AlignedVector< FlashText::Character >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		{
			const FlashFont* font = dictionary->getFont(i->fontId);
			if (!font)
				continue;

			const FlashShape* shape = font->getShape(i->glyphIndex);
			if (!shape)
				continue;

			float scaleHeight = 
				font->getCoordinateType() == FlashFont::CtTwips ? 
				1.0f / 1000.0f :
				1.0f / (20.0f * 1000.0f);

			float scaleOffset = i->height * scaleHeight;

			m_displayRenderer->renderGlyph(
				*dictionary,
				textTransform * translate(i->offsetX, i->offsetY) * scale(scaleOffset, scaleOffset),
				font->getMaxDimension(),
				*shape,
				i->color,
				concateCxTransform(cxTransform, characterInstance->getColorTransform()),
				textInstance->getFilter(),
				textInstance->getFilterColor()
			);
		}

		return;
	}

	// Render dynamic texts.
	FlashEditInstance* editInstance = dynamic_type_cast< FlashEditInstance* >(characterInstance);
	if (editInstance)
	{
		if (!editInstance->isVisible())
			return;

		Matrix33 editTransform = transform * editInstance->getTransform();

		const TextLayout* layout = editInstance->getTextLayout();
		T_ASSERT (layout);

		const AlignedVector< TextLayout::Line >& lines = layout->getLines();
		const AlignedVector< TextLayout::Attribute >& attribs = layout->getAttributes();

		for (AlignedVector< TextLayout::Line >::const_iterator i = lines.begin(); i != lines.end(); ++i)
		{
			for (AlignedVector< TextLayout::Word >::const_iterator j = i->words.begin(); j != i->words.end(); ++j)
			{
				const TextLayout::Attribute& attrib = attribs[j->attrib];
				const AlignedVector< TextLayout::Character >& chars = j->chars;

				float coordScale = attrib.font->getCoordinateType() == FlashFont::CtTwips ? 1.0f / 1000.0f : 1.0f / (20.0f * 1000.0f);
				float fontScale = coordScale * layout->getFontHeight();

				for (uint32_t k = 0; k < chars.size(); ++k)
				{
					uint16_t glyphIndex = attrib.font->lookupIndex(chars[k].ch);

					const FlashShape* glyphShape = attrib.font->getShape(glyphIndex);
					if (!glyphShape)
						continue;

					m_displayRenderer->renderGlyph(
						*dictionary,
						editTransform * translate(chars[k].x, i->y) * scale(fontScale, fontScale),
						attrib.font->getMaxDimension(),
						*glyphShape,
						attrib.color,
						concateCxTransform(cxTransform, characterInstance->getColorTransform()),
						editInstance->getFilter(),
						editInstance->getFilterColor()
					);
				}
			}
		}

		return;
	}

	// Render buttons.
	FlashButtonInstance* buttonInstance = dynamic_type_cast< FlashButtonInstance* >(characterInstance);
	if (buttonInstance)
	{
		const FlashButton* button = buttonInstance->getButton();

		Matrix33 buttonTransform = transform * buttonInstance->getTransform();
		uint8_t buttonState = buttonInstance->getState();

		const FlashButton::button_layers_t& layers = button->getButtonLayers();
		for (int32_t j = int32_t(layers.size() - 1); j >= 0; --j)
		{
			const FlashButton::ButtonLayer& layer = layers[j];
			if ((layer.state & buttonState) == 0)
				continue;

			FlashCharacterInstance* referenceInstance = buttonInstance->getCharacterInstance(layer.characterId);
			if (!referenceInstance)
				continue;

			renderCharacter(
				dictionary,
				referenceInstance,
				buttonTransform * layer.placeMatrix,
				concateCxTransform(cxTransform, buttonInstance->getColorTransform())
			);
		}

		return;
	}

	// Render sprites.
	FlashSpriteInstance* spriteInstance = dynamic_type_cast< FlashSpriteInstance* >(characterInstance);
	if (spriteInstance)
	{
		FlashSpriteInstance* maskInstance = spriteInstance->getMask();
		if (maskInstance)
		{
			m_displayRenderer->beginMask(true);

			renderSprite(
				dictionary,
				maskInstance,
				transform * maskInstance->getTransform(),
				maskInstance->getColorTransform(),
				true
			);

			m_displayRenderer->endMask();
		}

		renderSprite(
			dictionary,
			spriteInstance,
			transform * spriteInstance->getTransform(),
			concateCxTransform(cxTransform, spriteInstance->getColorTransform()),
			false
		);

		if (maskInstance)
		{
			m_displayRenderer->beginMask(false);

			renderSprite(
				dictionary,
				maskInstance,
				transform * maskInstance->getTransform(),
				maskInstance->getColorTransform(),
				true
			);

			m_displayRenderer->endMask();
		}

		return;
	}
}

	}
}
