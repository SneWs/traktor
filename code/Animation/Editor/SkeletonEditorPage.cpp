/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cmath>
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/Editor/SkeletonAsset.h"
#include "Animation/Editor/SkeletonEditorPage.h"
#include "Animation/Editor/SkeletonFormat.h"
#include "Core/Io/FileSystem.h"
#include "Core/Math/Const.h"
#include "Core/Math/Vector2.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "I18N/Text.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/Resource/TextureFactory.h"
#include "Render/Resource/SequenceTextureFactory.h"
#include "Render/Resource/ShaderFactory.h"
#include "Resource/ResourceManager.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/HierarchicalState.h"
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewContentChangeEvent.h"
#include "Ui/TreeView/TreeViewItem.h"
#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace animation
	{
		namespace
		{

int findIndexOfJoint(const Skeleton* skeleton, const Joint* joint)
{
	for (int i = 0; i < int(skeleton->getJointCount()); ++i)
	{
		if (skeleton->getJoint(i) == joint)
			return i;
	}
	return -1;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.SkeletonEditorPage", SkeletonEditorPage, editor::IEditorPage)

SkeletonEditorPage::SkeletonEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
,	m_selectedJoint(-1)
,	m_cameraHead(0.0f)
,	m_cameraY(0.0f)
,	m_cameraZ(0.0f)
,	m_cameraMoveScaleY(1.0f)
,	m_cameraMoveScaleZ(1.0f)
,	m_cameraBoneScale(1.0f)
{
}

bool SkeletonEditorPage::create(ui::Container* parent)
{
	render::IRenderSystem* renderSystem = m_editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!renderSystem)
		return false;

	auto doc = m_document->getObject(0);
	if (is_a< SkeletonAsset >(doc))
	{
		m_skeletonAsset = static_cast< SkeletonAsset* >(doc);

		// Get full path to skeleton asset.
		auto assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
		auto filePath = FileSystem::getInstance().getAbsolutePath(Path(assetPath) + Path(m_skeletonAsset->getFileName()));

		// Parse skeleton.
		m_skeleton = SkeletonFormat::readAny(
			filePath,
			m_skeletonAsset->getOffset(),
			m_skeletonAsset->getScale(),
			m_skeletonAsset->getRadius(),
			m_skeletonAsset->getInvertX(),
			m_skeletonAsset->getInvertZ()
		);
	}
	else if (is_a< Skeleton >(doc))
	{
		m_skeletonAsset = nullptr;
		m_skeleton = static_cast< Skeleton* >(doc);
	}
	else
		return false;

	m_renderWidget = new ui::Widget();
	m_renderWidget->create(parent, ui::WsNone);
	m_renderWidget->addEventHandler< ui::MouseButtonDownEvent >(this, &SkeletonEditorPage::eventMouseDown);
	m_renderWidget->addEventHandler< ui::MouseButtonUpEvent >(this, &SkeletonEditorPage::eventMouseUp);
	m_renderWidget->addEventHandler< ui::MouseMoveEvent >(this, &SkeletonEditorPage::eventMouseMove);
	m_renderWidget->addEventHandler< ui::SizeEvent >(this, &SkeletonEditorPage::eventSize);
	m_renderWidget->addEventHandler< ui::PaintEvent >(this, &SkeletonEditorPage::eventPaint);

	m_boneMenu = new ui::Menu();
	m_boneMenu->add(new ui::MenuItem(ui::Command(L"Skeleton.Editor.AddBone"), i18n::Text(L"SKELETON_EDITOR_ADD_BONE")));
	m_boneMenu->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"SKELETON_EDITOR_DELETE_BONE")));

	// Create skeleton panel.
	m_skeletonPanel = new ui::Container();
	m_skeletonPanel->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0));
	m_skeletonPanel->setText(i18n::Text(L"SKELETON_EDITOR_SKELETON"));

	m_treeSkeleton = new ui::TreeView();
	m_treeSkeleton->create(m_skeletonPanel, (ui::TreeView::WsDefault | ui::WsAccelerated) & ~ui::WsClientBorder);
	m_treeSkeleton->addImage(new ui::StyleBitmap(L"Animation.Bones"), 2);
	m_treeSkeleton->addEventHandler< ui::MouseButtonDownEvent >(this, &SkeletonEditorPage::eventTreeButtonDown);
	m_treeSkeleton->addEventHandler< ui::SelectionChangeEvent >(this, &SkeletonEditorPage::eventTreeSelect);

	if (!m_skeletonAsset)
		m_treeSkeleton->addEventHandler< ui::TreeViewContentChangeEvent >(this, &SkeletonEditorPage::eventTreeEdited);

	m_site->createAdditionalPanel(m_skeletonPanel, ui::dpi96(250), false);

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 0;
	desc.multiSample = m_editor->getSettings()->getProperty< int32_t >(L"Editor.MultiSample", 4);
	desc.waitVBlanks = 0;
	desc.syswin = m_renderWidget->getIWidget()->getSystemWindow();

	m_renderView = renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	Ref< db::Database > database = m_editor->getOutputDatabase();

	m_resourceManager = new resource::ResourceManager(database, true);
	m_resourceManager->addFactory(new render::TextureFactory(renderSystem, 0));
	m_resourceManager->addFactory(new render::SequenceTextureFactory());
	m_resourceManager->addFactory(new render::ShaderFactory(renderSystem));

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(m_resourceManager, renderSystem, 1))
		return false;

	if (m_skeletonAsset)
		m_site->setPropertyObject(m_skeletonAsset);
	else
		m_site->setPropertyObject(m_skeleton);

	Aabb3 boundingBox = calculateBoundingBox(m_skeleton);

	int majorAxis = majorAxis3(boundingBox.getExtent());
	float majorExtent = boundingBox.getExtent()[majorAxis];

	m_cameraHead = 0.0f;
	m_cameraY = -0.7f;
	m_cameraZ = majorExtent * 2.0f;

	m_cameraMoveScaleY = majorExtent / 200.0f;
	m_cameraMoveScaleZ = majorExtent / 100.0f;
	m_cameraBoneScale = majorExtent / 100.0f;

	updateSettings();
	createSkeletonTreeNodes();

	m_renderWidget->startTimer(30);
	return true;
}

void SkeletonEditorPage::destroy()
{
	m_site->destroyAdditionalPanel(m_skeletonPanel);
	
	safeClose(m_renderView);
	safeDestroy(m_skeletonPanel);
	safeDestroy(m_renderWidget);
}

bool SkeletonEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool SkeletonEditorPage::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.SettingsChanged")
	{
		updateSettings();
		m_renderWidget->update();
	}
	else if (command == L"Editor.PropertiesChanged")
	{
		createSkeletonTreeNodes();
		m_renderWidget->update();
	}
	else if (command == L"Editor.Undo")
	{
		if (!m_document->undo())
			return false;

		m_skeleton = m_document->getObject< Skeleton >(0);
		T_ASSERT (m_skeleton);

		createSkeletonTreeNodes();

		m_site->setPropertyObject(m_skeleton);
		m_renderWidget->update();
	}
	else if (command == L"Editor.Redo")
	{
		if (!m_document->redo())
			return false;

		m_skeleton = m_document->getObject< Skeleton >(0);
		T_ASSERT (m_skeleton);

		createSkeletonTreeNodes();

		m_site->setPropertyObject(m_skeleton);
		m_renderWidget->update();
	}
	else if (command == L"Editor.Delete")
	{
		if (m_skeletonAsset)
			return false;

		RefArray< ui::TreeViewItem > selectedItems;
		if (m_treeSkeleton->getItems(selectedItems, ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly) == 0)
			return false;

		m_document->push();

		for (auto item : selectedItems)
		{
			Joint* joint = item->getData< Joint >(L"JOINT");

			int32_t jointIndex = findIndexOfJoint(m_skeleton, joint);
			T_ASSERT (jointIndex >= 0);

			for (int32_t i = 0; i < int32_t(m_skeleton->getJointCount()); ++i)
			{
				if (m_skeleton->getJoint(i)->getParent() == jointIndex)
					m_skeleton->getJoint(i)->setParent(-1);
			}

			m_skeleton->removeJoint(joint);
		}

		createSkeletonTreeNodes();

		m_site->setPropertyObject(m_skeleton);
		m_renderWidget->update();
	}
	else if (command == L"Skeleton.Editor.AddJoint")
	{
		if (m_skeletonAsset)
			return false;

		RefArray< ui::TreeViewItem > selectedItems;
		if (m_treeSkeleton->getItems(selectedItems, ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly) != 1)
			return false;

		m_document->push();

		Joint* parentJoint = selectedItems.front()->getData< Joint >(L"JOINT");

		Ref< Joint > joint = new Joint();
		joint->setName(L"Joint");
		joint->setParent(parentJoint ? findIndexOfJoint(m_skeleton, parentJoint) : -1);
		m_skeleton->addJoint(joint);

		createSkeletonTreeNodes();

		m_site->setPropertyObject(joint);
		m_renderWidget->update();
	}
	else
		return false;

	return true;
}

void SkeletonEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	if (m_resourceManager)
		m_resourceManager->reload(eventId, false);
}

void SkeletonEditorPage::updateSettings()
{
	Ref< PropertyGroup > colors = m_editor->getSettings()->getProperty< PropertyGroup >(L"Editor.Colors");
	m_colorClear = colors->getProperty< Color4ub >(L"Background");
	m_colorGrid = colors->getProperty< Color4ub >(L"Grid");
}

void SkeletonEditorPage::createSkeletonTreeNodes()
{
	Ref< ui::HierarchicalState > treeSkeletonState = m_treeSkeleton->captureState();

	m_treeSkeleton->removeAllItems();

	Ref< ui::TreeViewItem > itemRoot = m_treeSkeleton->createItem(0, i18n::Text(L"SKELETON_EDITOR_ROOT"), 1);
	itemRoot->setImage(0, 0);

	createSkeletonTreeNodes(itemRoot, -1);

	m_treeSkeleton->applyState(treeSkeletonState);
}

void SkeletonEditorPage::createSkeletonTreeNodes(ui::TreeViewItem* parentItem, int parentNodeIndex)
{
	int32_t jointCount = m_skeleton->getJointCount();
	for (int32_t i = 0; i < jointCount; ++i)
	{
		Joint* joint = m_skeleton->getJoint(i);
		if (joint->getParent() == parentNodeIndex)
		{
			Ref< ui::TreeViewItem > itemJoint = m_treeSkeleton->createItem(parentItem, joint->getName(), 1);
			itemJoint->setImage(0, 1);
			itemJoint->setData(L"JOINT", joint);

			createSkeletonTreeNodes(itemJoint, i);
		}
	}
}

void SkeletonEditorPage::eventMouseDown(ui::MouseButtonDownEvent* event)
{
	if (!m_skeleton)
		return;

	m_document->push();

	m_lastMousePosition = event->getPosition();
	m_renderWidget->setCapture();
	m_renderWidget->setFocus();
}

void SkeletonEditorPage::eventMouseUp(ui::MouseButtonUpEvent* event)
{
	if (m_renderWidget->hasCapture())
		m_renderWidget->releaseCapture();
}

void SkeletonEditorPage::eventMouseMove(ui::MouseMoveEvent* event)
{
	if (!m_skeleton || !m_renderWidget->hasCapture())
		return;

	ui::Point mousePosition = event->getPosition();

	Vector2 mouseDelta(
		float(m_lastMousePosition.x - mousePosition.x),
		float(m_lastMousePosition.y - mousePosition.y)
	);

	if ((event->getKeyState() & ui::KsControl) == 0)
	{
		if (!m_skeletonAsset && m_selectedJoint >= 0)
		{
			Joint* joint = m_skeleton->getJoint(m_selectedJoint);
			T_ASSERT (joint);

			mouseDelta /= 200.0f;

			if (event->getButton() == ui::MbtLeft)
			{
				Transform T = joint->getTransform();
				Quaternion orientation = T.rotation();

				if ((event->getKeyState() & ui::KsMenu) == 0)
				{
					orientation *= Quaternion::fromAxisAngle(Vector4(0.0f, 1.0f, 0.0f, 0.0f), mouseDelta.x);
					orientation *= Quaternion::fromAxisAngle(Vector4(1.0f, 0.0f, 0.0f, 0.0f), mouseDelta.y);
				}
				else
					orientation *= Quaternion::fromAxisAngle(Vector4(0.0f, 0.0f, 1.0f, 0.0f), mouseDelta.x);

				joint->setTransform(Transform(T.translation(), orientation));
			}
			else if (event->getButton() == ui::MbtRight)
			{
				// \fixme Translate
			}
		}
	}
	else
	{
		m_cameraHead += mouseDelta.x / 100.0f;
		if (event->getButton() == ui::MbtLeft)
			m_cameraZ -= mouseDelta.y * m_cameraMoveScaleZ;
		else
			m_cameraY += mouseDelta.y * m_cameraMoveScaleY;
	}

	m_lastMousePosition = mousePosition;

	m_renderWidget->update();
}

void SkeletonEditorPage::eventSize(ui::SizeEvent* event)
{
	if (!m_renderView)
		return;

	ui::Size sz = event->getSize();

	m_renderView->reset(sz.cx, sz.cy);
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));
}

void SkeletonEditorPage::eventPaint(ui::PaintEvent* event)
{
	ui::Rect rc = m_renderWidget->getInnerRect();

	T_ASSERT (m_renderView);
	T_ASSERT (m_primitiveRenderer);

	if (!m_skeleton)
		return;

	if (!m_renderView->begin(render::EtCyclop))
		return;

	float tmp[4];
	m_colorClear.getRGBA32F(tmp);
	Color4f clearColor(tmp[0], tmp[1], tmp[2], tmp[3]);

	m_renderView->clear(
		render::CfColor | render::CfDepth,
		&clearColor,
		1.0f,
		128
	);

	render::Viewport viewport = m_renderView->getViewport();
	float aspect = float(viewport.width) / viewport.height;

	Matrix44 viewTransform = translate(0.0f, m_cameraY, m_cameraZ) * rotateY(m_cameraHead);
	Matrix44 projectionTransform = perspectiveLh(
		80.0f * PI / 180.0f,
		aspect,
		0.1f,
		2000.0f
	);

	if (m_primitiveRenderer->begin(0, projectionTransform))
	{
		m_primitiveRenderer->pushView(viewTransform);

		for (int x = -10; x <= 10; ++x)
		{
			m_primitiveRenderer->drawLine(
				Vector4(float(x), 0.0f, -10.0f, 1.0f),
				Vector4(float(x), 0.0f, 10.0f, 1.0f),
				m_colorGrid
			);
			m_primitiveRenderer->drawLine(
				Vector4(-10.0f, 0.0f, float(x), 1.0f),
				Vector4(10.0f, 0.0f, float(x), 1.0f),
				m_colorGrid
			);
		}

		m_primitiveRenderer->drawArrowHead(
			Vector4(11.0f, 0.0f, 0.0f, 1.0f),
			Vector4(12.0f, 0.0f, 0.0f, 1.0f),
			0.8f,
			Color4ub(255, 64, 64)
		);
		m_primitiveRenderer->drawArrowHead(
			Vector4(0.0f, 0.0f, 11.0f, 1.0f),
			Vector4(0.0f, 0.0f, 12.0f, 1.0f),
			0.8f,
			Color4ub(64, 64, 255)
		);

		AlignedVector< Transform > jointTransforms;
		calculateJointTransforms(
			m_skeleton,
			jointTransforms
		);

		for (int32_t i = 0; i < int32_t(m_skeleton->getJointCount()); ++i)
		{
			const Joint* joint = m_skeleton->getJoint(i);
			T_ASSERT (joint);

			m_primitiveRenderer->drawWireFrame(jointTransforms[i].toMatrix44(), joint->getRadius() * 4.0f);

			if (joint->getParent() >= 0)
			{
				const Joint* parent = m_skeleton->getJoint(joint->getParent());
				T_ASSERT (parent);

				Color4ub color = (m_selectedJoint == joint->getParent()) ? Color4ub(255, 128, 255, 128) : Color4ub(255, 255, 0, 128);

				Vector4 start = jointTransforms[joint->getParent()].translation();
				Vector4 end = jointTransforms[i].translation();

				Vector4 z = (end - start).normalized();

				Vector4 x, y;
				orthogonalFrame(z, x, y);

				Scalar radius(parent->getRadius());
				x *= radius;
				y *= radius;
				z *= radius;

				m_primitiveRenderer->drawLine(start, start + z + x + y, color);
				m_primitiveRenderer->drawLine(start, start + z - x + y, color);
				m_primitiveRenderer->drawLine(start, start + z + x - y, color);
				m_primitiveRenderer->drawLine(start, start + z - x - y, color);

				m_primitiveRenderer->drawLine(start + z + x + y, end, color);
				m_primitiveRenderer->drawLine(start + z - x + y, end, color);
				m_primitiveRenderer->drawLine(start + z + x - y, end, color);
				m_primitiveRenderer->drawLine(start + z - x - y, end, color);

				m_primitiveRenderer->drawLine(start + z + x + y, start + z - x + y, color);
				m_primitiveRenderer->drawLine(start + z - x + y, start + z - x - y, color);
				m_primitiveRenderer->drawLine(start + z - x - y, start + z + x - y, color);
				m_primitiveRenderer->drawLine(start + z + x - y, start + z + x + y, color);
			}
		}

		m_primitiveRenderer->end(0);
		m_primitiveRenderer->render(m_renderView, 0);
	}

	m_renderView->end();
	m_renderView->present();

	event->consume();
}

void SkeletonEditorPage::eventTreeButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	const ui::MenuItem* menuItem = m_boneMenu->showModal(m_treeSkeleton, event->getPosition());
	if (menuItem)
	{
		if (handleCommand(menuItem->getCommand()))
			event->consume();
	}
}

void SkeletonEditorPage::eventTreeSelect(ui::SelectionChangeEvent* event)
{
	RefArray< ui::TreeViewItem > selectedItems;
	if (m_treeSkeleton->getItems(selectedItems, ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly) != 1)
		return;

	Joint* joint = selectedItems.front()->getData< Joint >(L"JOINT");
	m_selectedJoint = findIndexOfJoint(m_skeleton, joint);

	if (joint)
		m_site->setPropertyObject(joint);
	else if (m_skeletonAsset)
		m_site->setPropertyObject(m_skeletonAsset);
	else
		m_site->setPropertyObject(m_skeleton);

	m_renderWidget->update();
}

void SkeletonEditorPage::eventTreeEdited(ui::TreeViewContentChangeEvent* event)
{
	T_ASSERT (!m_skeletonAsset);
	ui::TreeViewItem* selectedItem = event->getItem();
	Joint* joint = selectedItem->getData< Joint >(L"JOINT");
	if (joint)
	{
		std::wstring name = selectedItem->getText();
		joint->setName(name);
	}
}

	}
}
