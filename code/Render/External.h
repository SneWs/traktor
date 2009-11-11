#ifndef traktor_render_External_H
#define traktor_render_External_H

#include <map>
#include <string>
#include "Core/Guid.h"
#include "Core/Heap/Ref.h"
#include "Core/Heap/RefArray.h"
#include "Render/Node.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ShaderGraph;

/*! \brief External shader node.
 * \ingroup Render
 *
 * External shader node is a special kind of node
 * which reference another shader graph, a fragment.
 * When the shader is created the external fragment is
 * loaded and merged into the final shader.
 */
class T_DLLCLASS External : public Node
{
	T_RTTI_CLASS(External)

public:
	External();

	External(const Guid& fragmentGuid, ShaderGraph* fragmentGraph);

	void setFragmentGuid(const Guid& fragmentGuid);

	const Guid& getFragmentGuid() const;

	void setValue(const std::wstring& name, float value);

	float getValue(const std::wstring& name, float defaultValue) const;

	virtual std::wstring getInformation() const;

	virtual int getInputPinCount() const;

	virtual Ref< const InputPin > getInputPin(int index) const;

	virtual int getOutputPinCount() const;

	virtual Ref< const OutputPin > getOutputPin(int index) const;	

	virtual int getVersion() const;

	virtual bool serialize(Serializer& s);

	inline void setInputPins(const RefArray< InputPin >& inputPins) { m_inputPins = inputPins; }

	inline RefArray< InputPin >& getInputPins() { return m_inputPins; }

	inline void setOutputPins(const RefArray< OutputPin >& outputPins) { m_outputPins = outputPins; }

	inline RefArray< OutputPin >& getOutputPins() { return m_outputPins; }

	inline const std::map< std::wstring, float >& getValues() const { return m_values; }

private:
	Guid m_fragmentGuid;
	RefArray< InputPin > m_inputPins;
	RefArray< OutputPin > m_outputPins;
	std::map< std::wstring, float > m_values;
};

	}
}

#endif	// traktor_render_External_H
