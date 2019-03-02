#pragma once

#include "Amalgam/Game/IInputServer.h"
#include "Core/Platform.h"

namespace traktor
{

class PropertyGroup;

	namespace db
	{

class Database;

	}

	namespace input
	{

class IInputDevice;
class InputMappingSourceData;
class InputSourceFabricator;

	}

	namespace amalgam
	{

class IEnvironment;

/*! \brief
 * \ingroup Amalgam
 */
class InputServer : public IInputServer
{
	T_RTTI_CLASS;

public:
	InputServer();

	bool create(const PropertyGroup* defaultSettings, PropertyGroup* settings, db::Database* db, const SystemApplication& sysapp, const SystemWindow& syswin);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	int32_t reconfigure(const PropertyGroup* settings);

	void update(float deltaTime, bool renderViewActive);

	void updateRumble(float deltaTime, bool paused);

	virtual bool createInputMapping(const input::InputMappingStateData* stateData) override final;

	virtual bool fabricateInputSource(const std::wstring& sourceId, input::InputCategory category, bool analogue) override final;

	virtual bool isFabricating() const override final;

	virtual bool abortedFabricating() const override final;

	virtual bool resetInputSource(const std::wstring& sourceId) override final;

	virtual bool isIdle() const override final;

	virtual void apply() override final;

	virtual void revert() override final;

	virtual input::InputSystem* getInputSystem() override final;

	virtual input::InputMapping* getInputMapping() override final;

	virtual input::RumbleEffectPlayer* getRumbleEffectPlayer() override final;

private:
	Ref< PropertyGroup > m_settings;
	Ref< input::InputSystem > m_inputSystem;
	Ref< input::InputMapping > m_inputMapping;
	Ref< const input::InputMappingSourceData > m_inputMappingDefaultSourceData;
	Ref< input::InputMappingSourceData > m_inputMappingSourceData;
	Ref< const input::InputMappingStateData > m_inputMappingStateData;
	uint32_t m_inputConstantsHash;
	Ref< input::InputSourceFabricator > m_inputSourceFabricator;
	std::wstring m_inputSourceFabricatorId;
	Ref< input::IInputDevice > m_inputFabricatorAbortDevice;
	int32_t m_inputFabricatorAbortControl;
	bool m_inputFabricatorAbortUnbind;
	bool m_inputFabricatorAborted;
	bool m_inputActive;
	Ref< input::RumbleEffectPlayer > m_rumbleEffectPlayer;
};

	}
}

