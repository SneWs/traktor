#include <sstream>
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Spark/Editor/Shape/Gradient.h"
#include "Spark/Editor/Shape/Path.h"
#include "Spark/Editor/Shape/PathShape.h"
#include "Spark/Editor/Shape/Shape.h"
#include "Spark/Editor/Shape/Style.h"
#include "Spark/Editor/Shape/SvgParser.h"
#include "Xml/Document.h"
#include "Xml/Element.h"
#include "Xml/Attribute.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{
		
const struct { const wchar_t* name; Color4ub color; } c_colorTable[] =
{
	L"black",	Color4ub(0, 0, 0, 255),
	L"red",		Color4ub(255, 0, 0, 255),
	L"green",	Color4ub(0, 255, 0, 255),
	L"blue",	Color4ub(0, 0, 255, 255),
	L"yellow",	Color4ub(255, 255, 0, 255),
	L"white",	Color4ub(255, 255, 255, 255),
	L"lime",	Color4ub(0x40, 0x80, 0xff, 255)
};

bool parseColor(const std::wstring& color, Color4ub& outColor)
{
	if (startsWith< std::wstring >(color, L"#"))
	{
		int red, green, blue;
		swscanf(color.c_str(), L"#%02x%02x%02x", &red, &green, &blue);
		outColor = Color4ub(red, green, blue, 255);
		return true;
	}
	else if (startsWith< std::wstring >(color, L"rgb"))
	{
		int red, green, blue;
		swscanf(color.c_str(), L"rgb(%d,%d,%d)", &red, &green, &blue);
		outColor = Color4ub(red, green, blue, 255);
		return true;
	}
	else if (toLower(color) == L"none")
		return false;

	for (int i = 0; i < sizeof(c_colorTable) / sizeof(c_colorTable[0]); ++i)
	{
		if (toLower(color) == c_colorTable[i].name)
		{
			outColor = c_colorTable[i].color;
			return true;
		}
	}

	log::warning << L"Unknown color \"" << color << L"\"" << Endl;
	return false;
}

bool isWhiteSpace(wchar_t ch)
{
	return bool(ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n' || ch == L',');
}

bool isNumeric(wchar_t ch)
{
	return bool(std::wstring(L"+-0123456789.").find(ch) != std::wstring::npos);
}

void skipUntil(std::wstring::iterator& i, std::wstring::iterator end, bool (*isProc)(wchar_t))
{
	while (i != end && !isProc(*i))
		++i;
}
void skipUntilNot(std::wstring::iterator& i, std::wstring::iterator end, bool (*isProc)(wchar_t))
{
	while (i != end && isProc(*i))
		++i;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SvgParser", SvgParser, Object)

Ref< Shape > SvgParser::parse(xml::Document* doc)
{
	return traverse(doc->getDocumentElement());
}

Ref< Shape > SvgParser::traverse(xml::Element* elm)
{
	Ref< Shape > shape;

	std::wstring name = elm->getName();

	// Shapes
	if (name == L"svg" || name == L"g")
	{
		shape = parseGroup(elm);
	}
	else if (name == L"rect")
	{
		shape = parseRect(elm);
	}
	else if (name == L"polygon")
	{
		shape = parsePolygon(elm);
	}
	else if (name == L"polyline")
	{
		shape = parsePolyLine(elm);
	}
	else if (name == L"path")
	{
		shape = parsePath(elm);
	}
	
	// Other
	else if (name == L"defs")
	{
		parseDefs(elm);
	}
	else
		log::warning << L"Unknown SVG element \"" << name << L"\"" << Endl;

	if (shape)
	{
		shape->setStyle(
			parseStyle(elm)
		);
		
		shape->setTransform(
			parseTransform(elm)
		);
	
		for (xml::Node* child = elm->getFirstChild(); child; child = child->getNextSibling())
		{
			if (!is_a< xml::Element >(child))
				continue;

			Ref< Shape > childShape = traverse(static_cast< xml::Element* >(child));
			if (childShape)
				shape->addChild(childShape);
		}
	}

	return shape;
}

Ref< Shape > SvgParser::parseGroup(xml::Element* elm)
{
	return new Shape();
}

Ref< Shape > SvgParser::parseRect(xml::Element* elm)
{
	float x = parseAttr(elm, L"x");
	float y = parseAttr(elm, L"y");
	float width = parseAttr(elm, L"width");
	float height = parseAttr(elm, L"height");
	float round = parseAttr(elm, L"ry");

	Path path;

	if (round <= 0.0f)
	{
		path.moveTo(x, y);
		path.lineTo(x + width, y);
		path.lineTo(x + width, y + height);
		path.lineTo(x, y + height);
		path.close ();
	}
	else
	{
		path.moveTo (x + round, y);
		path.lineTo (x + width - round, y);
		path.cubicTo(x + width, y, x + width, y, x + width, y + round);
		path.lineTo (x + width, y + height - round);
		path.cubicTo(x + width, y + height, x + width, y + height, x + width - round, y + height);
		path.lineTo (x + round, y + height);
		path.cubicTo(x, y + height, x, y + height, x, y + height - round);
		path.lineTo (x, y + round);
		path.cubicTo(x, y, x, y, x + round, y);
		path.close  ();
	}

	return new PathShape(path);
}

Ref< Shape > SvgParser::parsePolygon(xml::Element* elm)
{
	Path path;
	bool first = true;

	std::wstring points = elm->getAttribute(L"points")->getValue();
	std::wstring::iterator i = points.begin();
	while (i != points.end())
	{
		skipUntilNot(i, points.end(), isWhiteSpace);

		std::wstring::iterator j = i;
		skipUntilNot(i, points.end(), isNumeric);

		float x = 0.0f;
		std::wstringstream(std::wstring(j, i)) >> x;

		skipUntil(i, points.end(), isNumeric);

		j = i;
		skipUntilNot(i, points.end(), isNumeric);

		float y = 0.0f;
		std::wstringstream(std::wstring(j, i)) >> y;

		if (first)
			path.moveTo(x, y);
		else
			path.lineTo(x, y);

		first = false;
	}

	path.close();

	return new PathShape(path);
}

Ref< Shape > SvgParser::parsePolyLine(xml::Element* elm)
{
	if (!elm || !elm->hasAttribute(L"points"))
		return 0;

	Path path;
	bool first = true;

	std::wstring points = elm->getAttribute(L"points")->getValue();
	std::wstring::iterator i = points.begin();
	while (i != points.end())
	{
		skipUntilNot(i, points.end(), isWhiteSpace);

		std::wstring::iterator j = i;
		skipUntilNot(i, points.end(), isNumeric);

		float x = 0.0f;
		std::wstringstream(std::wstring(j, i)) >> x;

		skipUntil(i, points.end(), isNumeric);

		j = i;
		skipUntilNot(i, points.end(), isNumeric);

		float y = 0.0f;
		std::wstringstream(std::wstring(j, i)) >> y;

		if (first)
			path.moveTo(x, y);
		else
			path.lineTo(x, y);

		first = false;
	}

	return new PathShape(path);
}

namespace
{

	float parsePathNumber(std::wstring::iterator& i, std::wstring::iterator end)
	{
		skipUntilNot(i, end, isWhiteSpace);

		std::wstring::iterator j = i;
		skipUntilNot(i, end, isNumeric);

		float number = 0.0f;
		std::wstringstream(std::wstring(j, i)) >> number;

		return number;
	}

}

Ref< Shape > SvgParser::parsePath(xml::Element* elm)
{
	if (!elm || !elm->hasAttribute(L"d"))
		return 0;

	std::wstring def = elm->getAttribute(L"d")->getValue();
	std::wstring::iterator i = def.begin();

	Path path;
	while (i != def.end())
	{
		skipUntilNot(i, def.end(), isWhiteSpace);
		if (i == def.end())
			break;

		wchar_t cmd = *i++;
		bool relative = (cmd != toupper(cmd));

		switch (toupper(cmd))
		{
		case L'M':	// Move to
			{
				float x = parsePathNumber(i, def.end());
				float y = parsePathNumber(i, def.end());
				path.moveTo(x, y, relative);
			}
			break;

		case L'L':	// Line to
			{
				float x = parsePathNumber(i, def.end());
				float y = parsePathNumber(i, def.end());
				path.lineTo(x, y, relative);
			}
			break;

		case L'V':	// Vertical line to
			{
				float x = parsePathNumber(i, def.end());
				path.lineTo(x, path.getCursor().y, relative);
			}
			break;

		case L'H':	// Horizontal line to
			{
				float y = parsePathNumber(i, def.end());
				path.lineTo(path.getCursor().x, y, relative);
			}
			break;

		case L'Q':	// Quadric to
			{
				float x1 = parsePathNumber(i, def.end());
				float y1 = parsePathNumber(i, def.end());
				float x2 = parsePathNumber(i, def.end());
				float y2 = parsePathNumber(i, def.end());
				path.quadricTo(x1, y1, x2, y2, relative);
			}
			break;

		case L'T':	// Quadric to (shorthand/smooth)
			{
				float x = parsePathNumber(i, def.end());
				float y = parsePathNumber(i, def.end());
				path.quadricTo(x, y, relative);
			}
			break;

		case L'C':	// Cubic to
			{
				float x1 = parsePathNumber(i, def.end());
				float y1 = parsePathNumber(i, def.end());
				float x2 = parsePathNumber(i, def.end());
				float y2 = parsePathNumber(i, def.end());
				float x  = parsePathNumber(i, def.end());
				float y  = parsePathNumber(i, def.end());
				path.cubicTo(x1, y1, x2, y2, x, y, relative);
			}
			break;

		case L'S':	// Cubic to (shorthand/smooth)
			{
				float x1 = parsePathNumber(i, def.end());
				float y1 = parsePathNumber(i, def.end());
				float x2 = parsePathNumber(i, def.end());
				float y2 = parsePathNumber(i, def.end());
				path.cubicTo(x1, y1, x2, y2, relative);
			}
			break;

		case L'A':	// Unknown
			break;

		case L'Z':	// Close sub path
			path.close();
			break;

		default:
			log::error << L"Unknown path command character \"" << *i << L"\"" << Endl;
		}
	}

	return new PathShape(path);
}

void SvgParser::parseDefs(xml::Element* elm)
{
	for (xml::Node* child = elm->getFirstChild(); child; child = child->getNextSibling())
	{
		if (!is_a< xml::Element >(child))
			continue;
			
		xml::Element* ch = static_cast< xml::Element* >(child);
		if (!ch->hasAttribute(L"id"))
		{
			log::warning << L"Invalid definition, no \"id\" attribute" << Endl;
			continue;
		}
		
		std::wstring name = ch->getName();
		std::wstring id = ch->getAttribute(L"id")->getValue();
		
		if (name == L"linearGradient")
		{
			RefArray< xml::Element > stops;
			elm->get(L"stop", stops);
			
			if (!stops.empty())
			{
				Ref< Gradient > gradient = new Gradient(Gradient::GtLinear);
				for (RefArray< xml::Element >::iterator i = stops.begin(); i != stops.end(); ++i)
				{
					xml::Element* stop = *i;
					if (!stop->hasAttribute(L"offset") || !stop->hasAttribute(L"stop-color"))
						continue;
				
					float offset;
					std::wstringstream(stop->getAttribute(L"offset")->getValue()) >> offset;
					
					Color4ub color;
					parseColor(stop->getAttribute(L"stop-color")->getValue(), color);
					
					gradient->addStop(offset, color);
				}
				m_gradients[id] = gradient;
			}
		}
		else if (name == L"radialGradient")
		{
			RefArray< xml::Element > stops;
			elm->get(L"stop", stops);
			
			if (!stops.empty())
			{
				Ref< Gradient > gradient = new Gradient(Gradient::GtRadial);
				for (RefArray< xml::Element >::iterator i = stops.begin(); i != stops.end(); ++i)
				{
					xml::Element* stop = *i;
					if (!stop->hasAttribute(L"offset") || !stop->hasAttribute(L"stop-color"))
						continue;
				
					float offset;
					std::wstringstream(stop->getAttribute(L"offset")->getValue()) >> offset;
					
					Color4ub color;
					parseColor(stop->getAttribute(L"stop-color")->getValue(), color);
					
					gradient->addStop(offset, color);
				}
				m_gradients[id] = gradient;
			}
		}
		else
			log::error << L"Unknown definition element \"" << name << L"\"" << Endl;
	}
}

Ref< Style > SvgParser::parseStyle(xml::Element* elm)
{
	if (!elm)
		return 0;

	Ref< Style > style;
	Color4ub color;

	if (elm->hasAttribute(L"fill"))
	{
		style = new Style();

		std::wstring fillDesc = elm->getAttribute(L"fill")->getValue();
		if (parseColor(fillDesc, color))
		{
			style->setFillEnable(true);
			style->setFill(color);
		}
		else
			style->setFillEnable(false);
	}
	else if (elm->hasAttribute(L"style"))
	{
		style = new Style();

		std::vector< std::wstring > styles;
		Split< std::wstring >::any(elm->getAttribute(L"style")->getValue(), L";", styles);
		
		for (std::vector< std::wstring >::iterator i = styles.begin(); i != styles.end(); ++i)
		{
			std::wstring::size_type j = i->find(L':');
			if (j == std::string::npos)
				continue;

			std::wstring key = trim(i->substr(0, j));
			std::wstring value = trim(i->substr(j + 1));

			if (key == L"fill")
			{
				if (parseColor(value, color))
				{
					style->setFillEnable(true);
					style->setFill(color);
				}
				else
					style->setFillEnable(false);
			}
			else if (key == L"stroke")
			{
				if (parseColor(value, color))
				{
					style->setStrokeEnable(true);
					style->setStroke(color);
				}
				else
					style->setStrokeEnable(false);
			}
			else if (key == L"stroke-width")
			{
				float strokeWidth;
				std::wstringstream(value) >> strokeWidth;
				style->setStrokeWidth(strokeWidth);
			}
			else
				log::error << L"Unknown CSS style \"" << key << L"\"" << Endl;
		}
	}

	return style;
}

Matrix33 SvgParser::parseTransform(xml::Element* elm)
{
	if (!elm || !elm->hasAttribute(L"transform"))
		return Matrix33::identity();

	Matrix33 transform = Matrix33::identity();

	std::wstring transformDesc = elm->getAttribute(L"transform")->getValue();
	std::wstring::iterator i = transformDesc.begin();

	while (i != transformDesc.end())
	{
		skipUntilNot(i, transformDesc.end(), isWhiteSpace);

		std::wstring::iterator j = i;

		while (i != transformDesc.end() && *i != L'(')
			++i;

		if (i == transformDesc.end())
			break;

		std::wstring fnc(j, i);

		j = ++i;

		while (i != transformDesc.end() && *i != L')')
			++i;

		if (i == transformDesc.end())
			break;

		std::wstring args(j, i);

		if (fnc == L"matrix")
		{
			std::vector< float > argv;
			Split< std::wstring, float >::any(args, L",", argv);

			if (argv.size() >= 6)
				transform *= Matrix33(
					argv[0], argv[1], 0.0f,
					argv[2], argv[3], 0.0f,
					argv[4], argv[5], 1.0f
				);
		}
		else if (fnc == L"translate")
		{
			std::vector< float > argv;
			Split< std::wstring, float >::any(args, L",", argv);
			
			if (argv.size() >= 2)
				transform *= translate(argv[0], argv[1]);
		}
		else if (fnc == L"scale")
		{
			std::vector< float > argv;
			Split< std::wstring, float >::any(args, L",", argv);

			if (argv.size() >= 1)
				transform * scale(argv[0], argv[0]);
		}
		else
			log::error << L"Unknown transform function \"" << fnc << L"\"" << Endl;

		++i;
	}
	
	return transform;
}

float SvgParser::parseAttr(xml::Element* elm, const std::wstring& attrName, float defValue) const
{
	if (elm && elm->hasAttribute(attrName))
	{
		std::wstring attrValue = elm->getAttribute(attrName)->getValue();
		std::wstringstream ss(attrValue);
		ss >> defValue;
	}
	return defValue;
}

	}
}
