#include <cmath>
#include "Core/Math/Variance.h"

namespace traktor
{

Variance::Variance()
:	m_count(0)
,	m_mean(0.0f)
,	m_meanDistSquared(0.0f)
{
}

void Variance::insert(float value)
{
    float delta = value - m_mean;
    m_mean += delta / ++m_count;
    m_meanDistSquared += delta * (value - m_mean);
}

bool Variance::empty() const
{
    return (bool)(m_count < 2);
}

float Variance::getMean() const
{
    return m_mean;
}

float Variance::getVariance() const
{
    return m_meanDistSquared / (m_count - 1);
}

bool Variance::stop(float accept, float confidence) const
{
    if (!empty())
    {
        double threshold = accept / confidence;
        double standardError = std::sqrt(getVariance() / m_count);
        return standardError < m_mean * threshold;
    }
    else
        return false;
}

}
