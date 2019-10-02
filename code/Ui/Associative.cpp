#include <map>
#include "Ui/Associative.h"

namespace traktor
{
	namespace ui
	{

class Associative::Impl
{
public:
	void setData(const std::wstring& key, Object* data)
	{
		if (data)
			m_data[key] = data;
		else
		{
			auto it = m_data.find(key);
			if (it != m_data.end())
				m_data.erase(it);
		}
	}

	Object* getData(const std::wstring& key) const
	{
		auto it = m_data.find(key);
		return (it != m_data.end()) ? it->second.ptr() : nullptr;
	}

private:
	std::map< std::wstring, Ref< Object > > m_data;
};

Associative::Associative()
:	m_impl(nullptr)
{
}

Associative::~Associative()
{
	delete m_impl;
}

void Associative::removeAllData()
{
	delete m_impl;
	m_impl = nullptr;
}

void Associative::setData(const std::wstring& key, Object* data)
{
	if (!m_impl)
		m_impl = new Impl();

	m_impl->setData(key, data);
}

Object* Associative::getData(const std::wstring& key) const
{
	if (m_impl)
		return m_impl->getData(key);
	else
		return nullptr;
}

	}
}
