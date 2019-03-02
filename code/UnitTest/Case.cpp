#include "UnitTest/Case.h"
#include "Core/Log/Log.h"

namespace traktor
{

void Case::execute(Context& context)
{
	m_context = &context;
	run();
}

void Case::succeeded(const std::wstring& message)
{
	m_context->succeeded++;
	log::info << message << Endl;
}

void Case::failed(const std::wstring& message)
{
	m_context->failed++;
	log::error << message << Endl;
}

}
