#include "Core/Guid.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/System/OS.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/Pipeline/PipelineDependency.h"
#include "I18N/Format.h"
#include "I18N/Text.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityDependencyInvestigator.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Bitmap.h"
#include "Ui/MethodHandler.h"
#include "Ui/TableLayout.h"
#include "Ui/TreeView.h"
#include "Ui/TreeViewItem.h"
#include "Ui/Events/CommandEvent.h"
#include "World/Entity/EntityData.h"

// Resources
#include "Resources/Types.h"
#include "Resources/EntityTypes.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityDependencyInvestigator", EntityDependencyInvestigator, ui::Container)

EntityDependencyInvestigator::EntityDependencyInvestigator(SceneEditorContext* context)
:	m_context(context)
{
}

void EntityDependencyInvestigator::destroy()
{
	safeDestroy(m_dependencyTree);
	m_currentEntityAdapter = 0;
	m_context = 0;
	ui::Container::destroy();
}

bool EntityDependencyInvestigator::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0)))
		return false;

	setText(i18n::Text(L"SCENE_EDITOR_DEPENDENCY_INVESTIGATOR"));

	m_dependencyTree = new ui::TreeView();
	m_dependencyTree->create(this, ui::TreeView::WsDefault & ~ui::WsClientBorder);
	m_dependencyTree->addImage(ui::Bitmap::load(c_ResourceTypes, sizeof(c_ResourceTypes), L"png"), 30);
	m_dependencyTree->addImage(ui::Bitmap::load(c_ResourceEntityTypes, sizeof(c_ResourceEntityTypes), L"png"), 4);
	m_dependencyTree->addActivateEventHandler(ui::createMethodHandler(this, &EntityDependencyInvestigator::eventDependencyActivate));

	m_context->addSelectEventHandler(ui::createMethodHandler(this, &EntityDependencyInvestigator::eventContextSelect));
	return true;
}

void EntityDependencyInvestigator::setEntityAdapter(EntityAdapter* entityAdapter)
{
	if (!isVisible(false))
		return;

	m_dependencyTree->removeAllItems();

	if (entityAdapter)
	{
		RefArray< editor::PipelineDependency > dependencies;
		m_context->getEditor()->buildAssetDependencies(entityAdapter->getEntityData(), 1, dependencies);

		Ref< ui::TreeViewItem > entityRootItem = m_dependencyTree->createItem(0, entityAdapter->getName(), 15);
		entityRootItem->expand();

		std::map< const TypeInfo*, Ref< ui::TreeViewItem > > typeGroups;
		std::set< Path > externalFiles;

		for (RefArray< editor::PipelineDependency >::iterator i = dependencies.begin(); i != dependencies.end(); ++i)
		{
			const TypeInfo* assetType = &type_of((*i)->sourceAsset);
			T_ASSERT (assetType);

			Ref< ui::TreeViewItem > typeGroup = typeGroups[assetType];
			if (!typeGroup)
			{
				typeGroup = m_dependencyTree->createItem(entityRootItem, i18n::Format(L"SCENE_EDITOR_DEPENDENCY_GROUP", assetType->getName()), 0, 1);
				typeGroups[assetType] = typeGroup;
			}

			Ref< ui::TreeViewItem > dependencyItem = m_dependencyTree->createItem(typeGroup, (*i)->outputPath, 2);
			dependencyItem->setData(L"DEPENDENCY", (*i));

			externalFiles.insert((*i)->files.begin(), (*i)->files.end());
		}

		if (!externalFiles.empty())
		{
			Ref< ui::TreeViewItem > filesGroup = m_dependencyTree->createItem(entityRootItem, i18n::Text(L"SCENE_EDITOR_DEPENDENCY_FILES"), 0, 1);
			for (std::set< Path >::iterator i = externalFiles.begin(); i != externalFiles.end(); ++i)
			{
				Ref< ui::TreeViewItem > fileItem = m_dependencyTree->createItem(filesGroup, i->getFileName(), 2);
				fileItem->setData(L"FILE", new Path(*i));
			}
		}
	}
}

void EntityDependencyInvestigator::eventDependencyActivate(ui::Event* event)
{
	Ref< ui::TreeViewItem > item = checked_type_cast< ui::TreeViewItem* >(
		checked_type_cast< ui::CommandEvent* >(event)->getItem()
	);

	Ref< editor::PipelineDependency > dependency = item->getData< editor::PipelineDependency >(L"DEPENDENCY");
	if (dependency)
	{
		Ref< db::Instance > instance = m_context->getEditor()->getSourceDatabase()->getInstance(dependency->outputGuid);
		if (instance)
			m_context->getEditor()->openEditor(instance);
	}

	Ref< Path > file = item->getData< Path >(L"FILE");
	if (file)
	{
		if (!OS::getInstance().editFile(*file))
			log::error << L"Unable to edit file \"" << file->getFileName() << L"\"" << Endl;
	}
}

void EntityDependencyInvestigator::eventContextSelect(ui::Event* event)
{
	RefArray< EntityAdapter > selectedEntityAdapters;
	if (m_context->getEntities(selectedEntityAdapters, SceneEditorContext::GfDescendants | SceneEditorContext::GfSelectedOnly) == 1)
		setEntityAdapter(selectedEntityAdapters[0]);
	else
		setEntityAdapter(0);
}

	}
}
