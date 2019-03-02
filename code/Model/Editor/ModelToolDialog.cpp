#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyInteger.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Model/Model.h"
#include "Model/ModelAdjacency.h"
#include "Model/ModelFormat.h"
#include "Model/Pose.h"
#include "Model/Editor/ModelToolDialog.h"
#include "Model/Operations/BakePixelOcclusion.h"
#include "Model/Operations/CalculateConvexHull.h"
#include "Model/Operations/CalculateOccluder.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Model/Operations/Clear.h"
#include "Model/Operations/CullDistantFaces.h"
#include "Model/Operations/FlattenDoubleSided.h"
#include "Model/Operations/Quantize.h"
#include "Model/Operations/Reduce.h"
#include "Model/Operations/ReduceSimplygon.h"
#include "Model/Operations/ScaleAlongNormal.h"
#include "Model/Operations/Transform.h"
#include "Model/Operations/Triangulate.h"
#include "Model/Operations/MergeCoplanarAdjacents.h"
#include "Model/Operations/Unweld.h"
#include "Model/Operations/UnwrapUV.h"
#include "Model/Operations/WeldHoles.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Resource/IResourceManager.h"
#include "Ui/Application.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/TableLayout.h"
#include "Ui/FileDialog.h"
#include "Ui/FloodLayout.h"
#include "Ui/Splitter.h"
#include "Ui/StyleBitmap.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridView.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarDropDown.h"
#include "Ui/ToolBar/ToolBarSeparator.h"
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewItem.h"
#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

const resource::Id< render::ITexture > c_textureDebug(Guid(L"{0163BEDD-9297-A64F-AAD5-360E27E37C6E}"));

void updateSkeletonTree(Model* model, ui::TreeView* treeView, ui::TreeViewItem* parentItem, uint32_t parentNodeIndex)
{
	int32_t jointCount = model->getJointCount();
	for (int32_t i = 0; i < jointCount; ++i)
	{
		const Joint& joint = model->getJoint(i);
		if (joint.getParent() == parentNodeIndex)
		{
			int32_t affecting = 0;
			for (const auto vtx : model->getVertices())
			{
				if (vtx.getJointInfluence(i) > FUZZY_EPSILON)
					++affecting;
			}

			Ref< ui::TreeViewItem > itemJoint = treeView->createItem(
				parentItem,
				joint.getName() + L" (" + toString(affecting) + L")",
				1
			);
			itemJoint->setImage(0, 1);
			itemJoint->setData(L"JOINT", new PropertyInteger(i));

			updateSkeletonTree(model, treeView, itemJoint, i);
		}
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.ModelToolDialog", ModelToolDialog, ui::Dialog)

ModelToolDialog::ModelToolDialog(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem
)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_cameraHead(0.0f)
,	m_cameraPitch(0.0f)
,	m_cameraZ(10.0f)
,	m_normalScale(1.0f)
,	m_lastMousePosition(0, 0)
{
}

bool ModelToolDialog::create(ui::Widget* parent, const std::wstring& fileName, float scale)
{
	if (!ui::Dialog::create(parent, L"Model Tool", ui::dpi96(1000), ui::dpi96(800), ui::Dialog::WsDefaultResizable, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	setIcon(new ui::StyleBitmap(L"Editor.Icon"));
	addEventHandler< ui::CloseEvent >(this, &ModelToolDialog::eventDialogClose);

	Ref< ui::ToolBar > toolBar = new ui::ToolBar();
	toolBar->create(this);
	toolBar->addItem(new ui::ToolBarButton(L"Load...", ui::Command(L"ModelTool.Load"), ui::ToolBarButton::BsText));
	toolBar->addItem(new ui::ToolBarSeparator());

	m_toolSolid = new ui::ToolBarButton(L"Solid", ui::Command(L"ModelTool.ToggleSolid"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggled);
	toolBar->addItem(m_toolSolid);

	m_toolWire = new ui::ToolBarButton(L"Wire", ui::Command(L"ModelTool.ToggleWire"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggled);
	toolBar->addItem(m_toolWire);

	m_toolNormals = new ui::ToolBarButton(L"Normals", ui::Command(L"ModelTool.ToggleNormals"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggled);
	toolBar->addItem(m_toolNormals);

	m_toolVertices = new ui::ToolBarButton(L"Vertices", ui::Command(L"ModelTool.ToggleVertices"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggled);
	toolBar->addItem(m_toolVertices);

	m_toolCull = new ui::ToolBarButton(L"Cull Backfaces", ui::Command(L"ModelTool.ToggleCullBackfaces"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggled);
	toolBar->addItem(m_toolCull);

	m_toolNonSharedEdges = new ui::ToolBarButton(L"Non-shared Edges", ui::Command(L"ModelTool.ToggleNonSharedEdges"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggle);
	toolBar->addItem(m_toolNonSharedEdges);

	m_toolUV = new ui::ToolBarButton(L"UV", ui::Command(L"ModelTool.ToggleUV"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggle);
	toolBar->addItem(m_toolUV);

	m_toolChannel = new ui::ToolBarDropDown(ui::Command(L"ModelTool.Channel"), ui::dpi96(100), L"Channels");
	toolBar->addItem(m_toolChannel);

	m_toolWeight = new ui::ToolBarButton(L"Weights", ui::Command(L"ModelTool.ToggleWeights"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggle);
	toolBar->addItem(m_toolWeight);

	m_toolJointRest = new ui::ToolBarButton(L"Rest", ui::Command(L"ModelTool.ToggleJointRest"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggle);
	toolBar->addItem(m_toolJointRest);

	toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &ModelToolDialog::eventToolBarClick);

	Ref< ui::Splitter > splitter = new ui::Splitter();
	splitter->create(this, true, ui::dpi96(300), false);

	Ref< ui::Splitter > splitterH = new ui::Splitter();
	splitterH->create(splitter, false, 30, true);

	m_modelTree = new ui::TreeView();
	m_modelTree->create(splitterH, ui::WsDoubleBuffer);
	m_modelTree->addEventHandler< ui::MouseButtonDownEvent >(this, &ModelToolDialog::eventModelTreeButtonDown);
	m_modelTree->addEventHandler< ui::SelectionChangeEvent >(this, &ModelToolDialog::eventModelTreeSelect);

	Ref< ui::Tab > tab = new ui::Tab();
	tab->create(splitterH, ui::WsDoubleBuffer);

	// Material tab.
	Ref< ui::TabPage > tabPageMaterial = new ui::TabPage();
	tabPageMaterial->create(tab, L"Materials", new ui::FloodLayout());
	tab->addPage(tabPageMaterial);

	m_materialGrid = new ui::GridView();
	m_materialGrid->create(tabPageMaterial, ui::WsDoubleBuffer | ui::GridView::WsColumnHeader);
	m_materialGrid->addColumn(new ui::GridColumn(L"Name", ui::dpi96(100)));
	m_materialGrid->addColumn(new ui::GridColumn(L"Diffuse Map", ui::dpi96(100)));
	m_materialGrid->addColumn(new ui::GridColumn(L"Specular Map", ui::dpi96(100)));
	m_materialGrid->addColumn(new ui::GridColumn(L"Roughness Map", ui::dpi96(100)));
	m_materialGrid->addColumn(new ui::GridColumn(L"Metalness Map", ui::dpi96(100)));
	m_materialGrid->addColumn(new ui::GridColumn(L"Transparency Map", ui::dpi96(100)));
	m_materialGrid->addColumn(new ui::GridColumn(L"Emissive Map", ui::dpi96(100)));
	m_materialGrid->addColumn(new ui::GridColumn(L"Reflective Map", ui::dpi96(100)));
	m_materialGrid->addColumn(new ui::GridColumn(L"Normal Map", ui::dpi96(100)));
	m_materialGrid->addColumn(new ui::GridColumn(L"Light Map", ui::dpi96(100)));
	m_materialGrid->addColumn(new ui::GridColumn(L"Color", ui::dpi96(110)));
	m_materialGrid->addColumn(new ui::GridColumn(L"Diffuse Term", ui::dpi96(100)));
	m_materialGrid->addColumn(new ui::GridColumn(L"Specular Term", ui::dpi96(100)));
	m_materialGrid->addColumn(new ui::GridColumn(L"Roughness", ui::dpi96(100)));
	m_materialGrid->addColumn(new ui::GridColumn(L"Metalness", ui::dpi96(100)));
	m_materialGrid->addColumn(new ui::GridColumn(L"Transparency", ui::dpi96(100)));
	m_materialGrid->addColumn(new ui::GridColumn(L"Emissive", ui::dpi96(100)));
	m_materialGrid->addColumn(new ui::GridColumn(L"Reflective", ui::dpi96(100)));
	m_materialGrid->addColumn(new ui::GridColumn(L"Rim Light Intensity", ui::dpi96(100)));
	m_materialGrid->addColumn(new ui::GridColumn(L"Blend Operator", ui::dpi96(100)));
	m_materialGrid->addColumn(new ui::GridColumn(L"Double Sided", ui::dpi96(100)));

	// Skeleton tab.
	Ref< ui::TabPage > tabPageSkeleton = new ui::TabPage();
	tabPageSkeleton->create(tab, L"Skeleton", new ui::FloodLayout());
	tab->addPage(tabPageSkeleton);

	m_skeletonTree = new ui::TreeView();
	m_skeletonTree->create(tabPageSkeleton, ui::WsDoubleBuffer);
	m_skeletonTree->addImage(new ui::StyleBitmap(L"Animation.Bones"), 2);

	// Statistic tab.
	Ref< ui::TabPage > tabPageStatistics = new ui::TabPage();
	tabPageStatistics->create(tab, L"Statistics", new ui::FloodLayout());
	tab->addPage(tabPageStatistics);

	m_statisticGrid = new ui::GridView();
	m_statisticGrid->create(tabPageStatistics, ui::WsDoubleBuffer | ui::GridView::WsColumnHeader);
	m_statisticGrid->addColumn(new ui::GridColumn(L"Name", ui::dpi96(120)));
	m_statisticGrid->addColumn(new ui::GridColumn(L"Value", ui::dpi96(400)));

	tab->setActivePage(tabPageMaterial);

	m_modelRootPopup = new ui::Menu();

	Ref< ui::MenuItem > modelRootPopupAdd = new ui::MenuItem(L"Add Operation...");
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.Clear"), L"Clear"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.CalculateTangents"), L"Calculate Tangents"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.CleanDegenerate"), L"Clean Degenerate"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.CleanDuplicates"), L"Clean Duplicates"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.ConvexHull"), L"Convex Hull"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.CullDistantFaces"), L"Cull Distant Faces"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.FlattenDoubleSided"), L"Flatten Double Sided"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.MergeCoplanar"), L"Merge Coplanar"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.Occluder"), L"Occluder"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.Quantize"), L"Quantize"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.Reduce"), L"Reduce"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.ScaleAlongNormal"), L"Scale Along Normal"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.Triangulate"), L"Triangulate"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.Unweld"), L"Unweld"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.UnwrapUV"), L"Unwrap UV"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.WeldHoles"), L"Weld Holes"));
	m_modelRootPopup->add(modelRootPopupAdd);

	Ref< ui::MenuItem > modelRootPopupPerform = new ui::MenuItem(L"Perform Operation...");
	modelRootPopupPerform->add(new ui::MenuItem(ui::Command(L"ModelTool.BakeOcclusion"), L"Bake Occlusion..."));
	m_modelRootPopup->add(modelRootPopupPerform);

	m_modelRootPopup->add(new ui::MenuItem(L"-"));
	m_modelRootPopup->add(new ui::MenuItem(ui::Command(L"ModelTool.SaveAs"), L"Save As..."));
	m_modelRootPopup->add(new ui::MenuItem(ui::Command(L"ModelTool.Remove"), L"Remove"));

	m_modelChildPopup = new ui::Menu();

	Ref< ui::MenuItem > modelChildPopupPerform = new ui::MenuItem(L"Perform Operation...");
	modelChildPopupPerform->add(new ui::MenuItem(ui::Command(L"ModelTool.BakeOcclusion"), L"Bake Occlusion..."));
	m_modelChildPopup->add(modelChildPopupPerform);

	m_modelChildPopup->add(new ui::MenuItem(L"-"));
	m_modelChildPopup->add(new ui::MenuItem(ui::Command(L"ModelTool.SaveAs"), L"Save As..."));
	m_modelChildPopup->add(new ui::MenuItem(ui::Command(L"ModelTool.Remove"), L"Remove"));

	m_renderWidget = new ui::Widget();
	m_renderWidget->create(splitter, ui::WsNone);
	m_renderWidget->addEventHandler< ui::MouseButtonDownEvent >(this, &ModelToolDialog::eventMouseDown);
	m_renderWidget->addEventHandler< ui::MouseButtonUpEvent >(this, &ModelToolDialog::eventMouseUp);
	m_renderWidget->addEventHandler< ui::MouseMoveEvent >(this, &ModelToolDialog::eventMouseMove);
	m_renderWidget->addEventHandler< ui::SizeEvent >(this, &ModelToolDialog::eventRenderSize);
	m_renderWidget->addEventHandler< ui::PaintEvent >(this, &ModelToolDialog::eventRenderPaint);

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 0;
	desc.multiSample = 0;
	desc.waitVBlanks = 0;
	desc.syswin = m_renderWidget->getIWidget()->getSystemWindow();

	m_renderView = m_renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(m_resourceManager, m_renderSystem, 1))
		return false;

	m_resourceManager->bind(c_textureDebug, m_textureDebug);

	if (!fileName.empty())
	{
		Ref< Model > model = ModelFormat::readAny(fileName);
		if (model)
		{
			if (std::abs(scale - 1.0f) > FUZZY_EPSILON)
				Transform(traktor::scale(scale, scale, scale)).apply(*model);

			Ref< ui::TreeViewItem > item = m_modelTree->createItem(0, fileName, 0);
			item->setData(L"MODEL", model);
		}
		else
			log::error << L"Unable to load \"" << fileName << L"\"." << Endl;
	}

	m_timer.start();

	update();
	show();

	return true;
}

void ModelToolDialog::destroy()
{
	safeClose(m_renderView);
	ui::Dialog::destroy();
}

bool ModelToolDialog::loadModel()
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, type_name(this), L"Load model(s)...", L"All files;*.*"))
		return false;

	std::vector< Path > fileNames;
	if (fileDialog.showModal(fileNames) != ui::DrOk)
	{
		fileDialog.destroy();
		return true;
	}
	fileDialog.destroy();

	for (std::vector< Path >::const_iterator i = fileNames.begin(); i != fileNames.end(); ++i)
	{
		Ref< Model > model = ModelFormat::readAny(*i);
		if (!model)
			continue;

		Ref< ui::TreeViewItem > item = m_modelTree->createItem(0, i->getFileName(), 0);
		item->setData(L"MODEL", model);
	}

	m_modelTree->update();
	return true;
}

bool ModelToolDialog::saveModel(Model* model)
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, type_name(this), L"Save model as...", L"All files;*.*", true))
		return false;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DrOk)
	{
		fileDialog.destroy();
		return true;
	}
	fileDialog.destroy();

	Model clone = *model;

	int32_t channel = m_toolChannel->getSelected();
	if (channel >= 0)
	{
		auto vertices = clone.getVertices();
		for (auto& vertex : vertices)
		{
			uint32_t tc = vertex.getTexCoord(channel);
			vertex.clearTexCoords();
			vertex.setTexCoord(0, tc);
		}
		clone.setVertices(vertices);
		CleanDuplicates(0.01f).apply(clone);
	}

	return ModelFormat::writeAny(fileName, &clone);
}

void ModelToolDialog::bakeOcclusion(Model* model)
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, type_name(this), L"Save occlusion image as...", L"All files;*.*", true))
		return;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DrOk)
	{
		fileDialog.destroy();
		return;
	}
	fileDialog.destroy();

	Ref< drawing::Image > imageOcclusion = new drawing::Image(
		drawing::PixelFormat::getA8R8G8B8(),
		1024,
		1024
	);
	Ref< IModelOperation > operation = new BakePixelOcclusion(
		imageOcclusion,
		64,
		0.75f,
		0.05f
	);
	if (operation->apply(*model))
	{
		if (imageOcclusion->save(fileName))
			log::info << L"Occlusion.png saved successfully!" << Endl;
		else
			log::error << L"Unable to save " << fileName.getPathName() << Endl;
	}
	else
		log::error << L"Unable to bake occlusion" << Endl;
}

void ModelToolDialog::updateOperations(ui::TreeViewItem* itemModel)
{
	T_ASSERT (itemModel->getParent() == 0);

	Ref< Model > model = itemModel->getData< Model >(L"MODEL");

	const RefArray< ui::TreeViewItem >& children = itemModel->getChildren();
	for (RefArray< ui::TreeViewItem >::const_iterator i = children.begin(); i != children.end(); ++i)
	{
		const IModelOperation* operation = (*i)->getData< IModelOperation >(L"OPERATION");
		T_ASSERT (operation != 0);

		model = new Model(*model);
		operation->apply(*model);

		(*i)->setData(L"MODEL", model);
	}
}

void ModelToolDialog::addStatistic(const std::wstring& name, const std::wstring& value)
{
	Ref< ui::GridRow > row = new ui::GridRow();
	row->add(new ui::GridItem(name));
	row->add(new ui::GridItem(value));
	m_statisticGrid->addRow(row);
}

void ModelToolDialog::eventDialogClose(ui::CloseEvent* event)
{
	destroy();
}

void ModelToolDialog::eventToolBarClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (command == L"ModelTool.Load")
		loadModel();

	m_renderWidget->update();
}

void ModelToolDialog::eventModelTreeButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	RefArray< ui::TreeViewItem > items;
	if (m_modelTree->getItems(items, ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly) != 1)
		return;

	T_ASSERT (items.front());
	if (items.front()->getParent() == 0)
	{
		Ref< ui::TreeViewItem > itemModel = items.front();
		const ui::MenuItem* selected = m_modelRootPopup->showModal(m_modelTree, event->getPosition());
		if (selected)
		{
			const ui::Command& command = selected->getCommand();
			if (command == L"ModelTool.Clear")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Clear", 0);
				itemOperation->setData(L"OPERATION", new Clear( Model::CfMaterials | Model::CfColors | Model::CfNormals | Model::CfTexCoords | Model::CfJoints ));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.CalculateTangents")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Calculate Tangents", 0);
				itemOperation->setData(L"OPERATION", new CalculateTangents());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.CleanDegenerate")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Clean Degenerate", 0);
				itemOperation->setData(L"OPERATION", new CleanDegenerate());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.CleanDuplicates")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Clean Duplicates", 0);
				itemOperation->setData(L"OPERATION", new CleanDuplicates(0.001f));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.ConvexHull")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Convex Hull", 0);
				itemOperation->setData(L"OPERATION", new CalculateConvexHull());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.CullDistantFaces")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Cull Distant Faces", 0);
				itemOperation->setData(L"OPERATION", new CullDistantFaces(Aabb3()));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.FlattenDoubleSided")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Flatten Double Sided", 0);
				itemOperation->setData(L"OPERATION", new FlattenDoubleSided());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.MergeCoplanar")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Merge Coplanar", 0);
				itemOperation->setData(L"OPERATION", new MergeCoplanarAdjacents(true));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.Occluder")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Occluder", 0);
				itemOperation->setData(L"OPERATION", new CalculateOccluder());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.Quantize")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Quantize", 0);
				itemOperation->setData(L"OPERATION", new Quantize(0.5f));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.Reduce")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Reduce", 0);
				itemOperation->setData(L"OPERATION", new Reduce(0.5f));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.ScaleAlongNormal")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Scale Along Normal", 0);
				itemOperation->setData(L"OPERATION", new ScaleAlongNormal(1.0f));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.Triangulate")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Triangulate", 0);
				itemOperation->setData(L"OPERATION", new Triangulate());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.Unweld")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Unweld", 0);
				itemOperation->setData(L"OPERATION", new Unweld());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.UnwrapUV")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Unwrap UV", 0);
				itemOperation->setData(L"OPERATION", new UnwrapUV(0));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.WeldHoles")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Weld Holes", 0);
				itemOperation->setData(L"OPERATION", new WeldHoles());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.BakeOcclusion")
			{
				bakeOcclusion(itemModel->getData< Model >(L"MODEL"));
			}
			else if (command == L"ModelTool.SaveAs")
			{
				saveModel(itemModel->getData< Model >(L"MODEL"));
			}
			else if (command == L"ModelTool.Remove")
			{
				m_modelTree->removeItem(itemModel);
				m_model = 0;
				m_modelTris = 0;
				m_modelAdjacency = 0;
				m_renderWidget->update();
			}
		}
	}
	else
	{
		Ref< ui::TreeViewItem > itemOperation = items.front();
		Ref< ui::TreeViewItem > itemModel = itemOperation->getParent();
		const ui::MenuItem* selected = m_modelChildPopup->showModal(m_modelTree, event->getPosition());
		if (selected)
		{
			const ui::Command& command = selected->getCommand();
			if (command == L"ModelTool.BakeOcclusion")
			{
				bakeOcclusion(itemOperation->getData< Model >(L"MODEL"));
			}
			else if (command == L"ModelTool.SaveAs")
			{
				saveModel(itemModel->getData< Model >(L"MODEL"));
			}
			else if (command == L"ModelTool.Remove")
			{
				m_modelTree->removeItem(itemOperation);
				updateOperations(itemModel);
				m_model = 0;
				m_modelTris = 0;
				m_modelAdjacency = 0;
				m_renderWidget->update();
			}
		}
	}
}

void ModelToolDialog::eventModelTreeSelect(ui::SelectionChangeEvent* event)
{
	RefArray< ui::TreeViewItem > items;
	m_modelTree->getItems(items, ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly);

	if (items.size() == 1)
		m_model = items[0]->getData< Model >(L"MODEL");
	else
	{
		m_model = 0;
		m_modelTris = 0;
		m_modelAdjacency = 0;
	}

	m_materialGrid->removeAllRows();
	m_statisticGrid->removeAllRows();
	m_toolChannel->removeAll();
	m_skeletonTree->removeAllItems();

	if (m_model)
	{
		m_modelTris = new Model(*m_model);
		Triangulate().apply(*m_modelTris);

		Aabb3 boundingBox = m_model->getBoundingBox();
		Vector4 extent = boundingBox.getExtent();
		float minExtent = extent[minorAxis3(extent)];
		m_normalScale = minExtent / 10.0f;

		const AlignedVector< Material >& materials = m_model->getMaterials();
		for (AlignedVector< Material >::const_iterator i = materials.begin(); i != materials.end(); ++i)
		{
			const auto& cl = i->getColor();

			Ref< ui::GridRow > row = new ui::GridRow();
			row->add(new ui::GridItem(i->getName()));
			row->add(new ui::GridItem(i->getDiffuseMap().name));
			row->add(new ui::GridItem(i->getSpecularMap().name));
			row->add(new ui::GridItem(i->getRoughnessMap().name));
			row->add(new ui::GridItem(i->getMetalnessMap().name));
			row->add(new ui::GridItem(i->getTransparencyMap().name));
			row->add(new ui::GridItem(i->getEmissiveMap().name));
			row->add(new ui::GridItem(i->getReflectiveMap().name));
			row->add(new ui::GridItem(i->getNormalMap().name));
			row->add(new ui::GridItem(i->getLightMap().name));
			row->add(new ui::GridItem( toString((int32_t)cl.r) + L", " + toString((int32_t)cl.g) + L", " + toString((int32_t)cl.b) + L", " + toString((int32_t)cl.a)));
			row->add(new ui::GridItem(toString(i->getDiffuseTerm())));
			row->add(new ui::GridItem(toString(i->getSpecularTerm())));
			row->add(new ui::GridItem(toString(i->getRoughness())));
			row->add(new ui::GridItem(toString(i->getMetalness())));
			row->add(new ui::GridItem(toString(i->getTransparency())));
			row->add(new ui::GridItem(toString(i->getEmissive())));
			row->add(new ui::GridItem(toString(i->getReflective())));
			row->add(new ui::GridItem(toString(i->getRimLightIntensity())));
			row->add(new ui::GridItem(L"Default"));
			row->add(new ui::GridItem(i->isDoubleSided() ? L"Yes" : L"No"));
			m_materialGrid->addRow(row);
		}

		{
			addStatistic(L"# materials", toString(m_model->getMaterials().size()));
			addStatistic(L"# vertices", toString(m_model->getVertexCount()));
			addStatistic(L"# polygons", toString(m_model->getPolygonCount()));

			std::map< uint32_t, uint32_t > polSizes;
			for (const auto& pol : m_model->getPolygons())
				polSizes[pol.getVertexCount()]++;

			for (auto polSize : polSizes)
				addStatistic(L"# " + toString(polSize.first) + L"-polygons", toString(polSize.second));

			addStatistic(L"# positions", toString(m_model->getPositionCount()));
			addStatistic(L"# colors", toString(m_model->getColorCount()));
			addStatistic(L"# normals", toString(m_model->getNormalCount()));
			addStatistic(L"# texcoords", toString(m_model->getTexCoords().size()));
			addStatistic(L"# texture channels", toString(m_model->getTexCoordChannels().size()));
			addStatistic(L"# joints", toString(m_model->getJointCount()));
			addStatistic(L"# animations ", toString(m_model->getAnimationCount()));
			addStatistic(L"# blend targets", toString(m_model->getBlendTargetCount()));
		}

		uint32_t nextChannel = 8; // m_model->getAvailableTexCoordChannel();
		if (nextChannel > 0)
		{
			for (uint32_t i = 0; i < nextChannel; ++i)
				m_toolChannel->add(toString(i));

			m_toolChannel->select(0);
			m_toolChannel->setEnable(true);
		}
		else
			m_toolChannel->setEnable(false);

		updateSkeletonTree(m_model, m_skeletonTree, nullptr, c_InvalidIndex);
	}

	m_renderWidget->update();
}

void ModelToolDialog::eventMouseDown(ui::MouseButtonDownEvent* event)
{
	m_lastMousePosition = event->getPosition();
	m_renderWidget->setCapture();
	m_renderWidget->setFocus();
}

void ModelToolDialog::eventMouseUp(ui::MouseButtonUpEvent* event)
{
	if (m_renderWidget->hasCapture())
		m_renderWidget->releaseCapture();
}

void ModelToolDialog::eventMouseMove(ui::MouseMoveEvent* event)
{
	if (!m_renderWidget->hasCapture())
		return;

	ui::Point mousePosition = event->getPosition();

	Vector2 mouseDelta(
		float(m_lastMousePosition.x - mousePosition.x),
		float(m_lastMousePosition.y - mousePosition.y)
	);

	if (event->getButton() != ui::MbtRight)
	{
		m_cameraHead += mouseDelta.x / 100.0f;
		m_cameraPitch += mouseDelta.y / 100.0f;
	}
	else
		m_cameraZ -= mouseDelta.y * 0.1f;

	m_lastMousePosition = mousePosition;

	m_renderWidget->update();
}

void ModelToolDialog::eventRenderSize(ui::SizeEvent* event)
{
	if (!m_renderView)
		return;

	ui::Size sz = event->getSize();
	m_renderView->reset(sz.cx, sz.cy);
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));
}

void ModelToolDialog::eventRenderPaint(ui::PaintEvent* event)
{
	ui::Rect rc = m_renderWidget->getInnerRect();

	T_ASSERT (m_renderView);
	T_ASSERT (m_primitiveRenderer);

	if (!m_renderView->begin(render::EtCyclop))
		return;

	const Color4f clearColor(46/255.0f, 56/255.0f, 92/255.0f, 1.0f);
	m_renderView->clear(
		render::CfColor | render::CfDepth,
		&clearColor,
		1.0f,
		128
	);

	render::Viewport viewport = m_renderView->getViewport();
	float aspect = float(viewport.width) / viewport.height;

	Matrix44 viewTransform = translate(0.0f, 0.0f, m_cameraZ) * rotateY(m_cameraHead) * rotateX(m_cameraPitch);
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
				(x == 0) ? 2.0f : 0.0f,
				Color4ub(0, 0, 0, 80)
			);
			m_primitiveRenderer->drawLine(
				Vector4(-10.0f, 0.0f, float(x), 1.0f),
				Vector4(10.0f, 0.0f, float(x), 1.0f),
				(x == 0) ? 2.0f : 0.0f,
				Color4ub(0, 0, 0, 80)
			);
		}

		if (m_model)
		{
			T_ASSERT (m_modelTris);

			int32_t channel = m_toolChannel->getSelected();

			RefArray< ui::TreeViewItem > selectedItems;
			m_skeletonTree->getItems(selectedItems, ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly);

			int32_t weightJoint = -1;
			if (selectedItems.size() == 1)
				weightJoint = *selectedItems.front()->getData< PropertyInteger >(L"JOINT");

			// Render solid.
			if (m_toolSolid->isToggled())
			{
				bool cull = m_toolCull->isToggled();

				Vector4 eyePosition = viewTransform.inverse().translation().xyz1();	// Eye position in object space.
				Vector4 lightDir = viewTransform.inverse().axisZ();	// Light direction in object space.

				const AlignedVector< Vertex >& vertices = m_modelTris->getVertices();
				const AlignedVector< Polygon >& polygons = m_modelTris->getPolygons();
				const AlignedVector< Vector4 >& positions = m_modelTris->getPositions();

				m_primitiveRenderer->pushDepthState(true, true, false);
				for (AlignedVector< Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
				{
					const AlignedVector< uint32_t >& indices = i->getVertices();
					T_ASSERT (indices.size() == 3);

					Vector4 p[3];

					for (uint32_t i = 0; i < indices.size(); ++i)
					{
						const Vertex& vx0 = vertices[indices[i]];
						p[i] = positions[vx0.getPosition()];
					}

					Vector4 N = cross(p[0] - p[1], p[2] - p[1]).normalized();
					float diffuse = abs(dot3(lightDir, N)) * 0.5f + 0.5f;

					if (cull)
					{
						if (dot3(eyePosition - p[0], N) < 0)
							continue;
					}

					if (!m_toolWeight->isToggled())
					{
						if (vertices[indices[0]].getTexCoordCount() > channel)
						{
							m_primitiveRenderer->drawTextureTriangle(
								p[2], m_modelTris->getTexCoord(vertices[indices[2]].getTexCoord(channel)),
								p[1], m_modelTris->getTexCoord(vertices[indices[1]].getTexCoord(channel)),
								p[0], m_modelTris->getTexCoord(vertices[indices[0]].getTexCoord(channel)),
								Color4ub(
									int32_t(diffuse * 255),
									int32_t(diffuse * 255),
									int32_t(diffuse * 255),
									255
								),
								m_textureDebug
							);
						}
						else
						{
							m_primitiveRenderer->drawSolidTriangle(p[2], p[1], p[0], Color4ub(
								int32_t(diffuse * 81),
								int32_t(diffuse * 105),
								int32_t(diffuse * 195),
								255
							));
						}
					}
					else
					{
						const Color4ub c_errorWeight(0, 0, 255, 255);
						const Color4ub c_noWeight(0, 255, 0, 255);
						const Color4ub c_fullWeight(255, 0, 0, 255);

						m_primitiveRenderer->drawSolidTriangle(
							p[2], (vertices[indices[2]].getJointInfluenceCount() > 0) ? lerp(c_noWeight, c_fullWeight, vertices[indices[2]].getJointInfluence(weightJoint)) : c_errorWeight,
							p[1], (vertices[indices[1]].getJointInfluenceCount() > 0) ? lerp(c_noWeight, c_fullWeight, vertices[indices[1]].getJointInfluence(weightJoint)) : c_errorWeight,
							p[0], (vertices[indices[0]].getJointInfluenceCount() > 0) ? lerp(c_noWeight, c_fullWeight, vertices[indices[0]].getJointInfluence(weightJoint)) : c_errorWeight
						);
					}
				}
				m_primitiveRenderer->popDepthState();
			}

			const AlignedVector< Vertex >& vertices = m_model->getVertices();
			const AlignedVector< Polygon >& polygons = m_model->getPolygons();
			const AlignedVector< Vector4 >& positions = m_model->getPositions();
			const AlignedVector< Vector4 >& normals = m_model->getNormals();
			const AlignedVector< Vector2 >& texCoords = m_model->getTexCoords();
			const AlignedVector< Joint >& joints = m_model->getJoints();

			// Render wire-frame.
			if (m_toolWire->isToggled())
			{
				m_primitiveRenderer->pushDepthState(true, false, false);
				for (AlignedVector< Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
				{
					const AlignedVector< uint32_t >& indices = i->getVertices();

					for (uint32_t i = 0; i < indices.size(); ++i)
					{
						const Vertex& vx0 = vertices[indices[i]];
						const Vertex& vx1 = vertices[indices[(i + 1) % indices.size()]];

						const Vector4& p0 = positions[vx0.getPosition()];
						const Vector4& p1 = positions[vx1.getPosition()];

						m_primitiveRenderer->drawLine(p0, p1, Color4ub(255, 255, 255, 200));
					}
				}
				m_primitiveRenderer->popDepthState();
			}

			// Render non-shared edges.
			if (m_toolNonSharedEdges->isToggled())
			{
				// Lazy create adjacency information as it's pretty costly.
				if (!m_modelAdjacency)
					m_modelAdjacency = new ModelAdjacency(m_model, ModelAdjacency::MdByPosition);

				m_primitiveRenderer->pushDepthState(true, false, false);
				for (uint32_t i = 0; i < polygons.size(); ++i)
				{
					const Polygon& polygon = polygons[i];
					const AlignedVector< uint32_t >& indices = polygon.getVertices();

					for (uint32_t j = 0; j < indices.size(); ++j)
					{
						uint32_t share = m_modelAdjacency->getSharedEdgeCount(i, j);
						if (share == 0)
						{
							const Vertex& vx0 = vertices[indices[j]];
							const Vertex& vx1 = vertices[indices[(j + 1) % indices.size()]];

							const Vector4& p0 = positions[vx0.getPosition()];
							const Vector4& p1 = positions[vx1.getPosition()];

							m_primitiveRenderer->drawLine(p0, p1, Color4ub(255, 40, 40, 200));
						}
					}
				}
				m_primitiveRenderer->popDepthState();
			}

			if (m_toolNormals->isToggled())
			{
				m_primitiveRenderer->pushDepthState(true, false, false);
				for (AlignedVector< Vertex >::const_iterator i = vertices.begin(); i != vertices.end(); ++i)
				{
					if (i->getNormal() != c_InvalidIndex)
					{
						const Vector4& p = positions[i->getPosition()];
						const Vector4& n = normals[i->getNormal()];

						m_primitiveRenderer->drawLine(p, p + n * Scalar(m_normalScale), Color4ub(0, 255, 0, 200));
					}
				}
				m_primitiveRenderer->popDepthState();
			}

			if (m_toolVertices->isToggled())
			{
				m_primitiveRenderer->pushDepthState(true, false, false);
				for (AlignedVector< Vector4 >::const_iterator i = positions.begin(); i != positions.end(); ++i)
				{
					m_primitiveRenderer->drawSolidPoint(*i, 2.0f, Color4ub(255, 255, 0, 200));
				}
				m_primitiveRenderer->popDepthState();
			}

			if (true /*joints*/)
			{
				AlignedVector< uint32_t > childJointIds;

				m_primitiveRenderer->pushDepthState(false, false, false);

				for (uint32_t i = 0; i < joints.size(); ++i)
				{
					const auto colorRest = (i == weightJoint) ? Color4ub(80, 255, 80, 255) : Color4ub(120, 255, 120, 255);

					childJointIds.resize(0);
					m_modelTris->findChildJoints(i, childJointIds);

					auto Tjoint = m_modelTris->getJointGlobalTransform(i);
					if (!childJointIds.empty())
					{
						for (auto childId : childJointIds)
						{
							auto Tchild = m_modelTris->getJointGlobalTransform(childId);

							m_primitiveRenderer->drawLine(
								Tjoint.translation(),
								Tchild.translation(),
								2.0f,
								colorRest
							);
						}
					}
					else
						m_primitiveRenderer->drawSolidPoint(
							Tjoint.translation(),
							2.0f,
							colorRest
						);
				}

				bool showOnlyRest = m_toolJointRest->isToggled();
				if (!showOnlyRest && m_modelTris->getAnimationCount() > 0)
				{
					const Animation* anim = m_modelTris->getAnimation(0);

					int32_t frame = (int32_t)(m_timer.getElapsedTime() * 10.0f);
					frame %= anim->getKeyFrameCount();

					const Pose* pose = anim->getKeyFramePose(frame);

					for (uint32_t i = 0; i < joints.size(); ++i)
					{
						const auto colorPose = (i == weightJoint) ? Color4ub(255, 255, 80, 255) : Color4ub(255, 180, 120, 255);

						childJointIds.resize(0);
						m_modelTris->findChildJoints(i, childJointIds);

						auto TjointPose = pose->getJointGlobalTransform(m_modelTris, i);
						if (!childJointIds.empty())
						{
							for (auto childId : childJointIds)
							{
								auto TchildPose = pose->getJointGlobalTransform(m_modelTris, childId);

								m_primitiveRenderer->drawLine(
									TjointPose.translation(),
									TchildPose.translation(),
									2.0f,
									colorPose
								);
							}
						}
						else
							m_primitiveRenderer->drawSolidPoint(
								TjointPose.translation(),
								2.0f,
								colorPose
							);
					}
				}

				m_primitiveRenderer->popDepthState();
			}

			if (m_toolUV->isToggled())
			{
				int32_t channel = m_toolChannel->getSelected();

				m_primitiveRenderer->setProjection(orthoLh(-2.0f, 2.0f, 2.0f, -2.0f, 0.0f, 1.0f));
				m_primitiveRenderer->pushView(Matrix44::identity());
				m_primitiveRenderer->pushDepthState(false, false, false);

				for (uint32_t i = 0; i < polygons.size(); ++i)
				{
					const Polygon& polygon = polygons[i];
					const AlignedVector< uint32_t >& indices = polygon.getVertices();

					for (uint32_t j = 0; j < indices.size(); ++j)
					{
						const Vertex& vx0 = vertices[indices[j]];
						const Vertex& vx1 = vertices[indices[(j + 1) % indices.size()]];

						if (vx0.getTexCoord(channel) != c_InvalidIndex && vx1.getTexCoord(channel) != c_InvalidIndex)
						{
							const Vector2& uv0 = texCoords[vx0.getTexCoord(channel)];
							const Vector2& uv1 = texCoords[vx1.getTexCoord(channel)];

							m_primitiveRenderer->drawLine(
								Vector4(uv0.x, uv0.y, 0.5f, 1.0f),
								Vector4(uv1.x, uv1.y, 0.5f, 1.0f),
								Color4ub(255, 255, 255, 200)
							);
						}
					}
				}

				m_primitiveRenderer->popView();
				m_primitiveRenderer->popDepthState();
			}
		}

		m_primitiveRenderer->end(0);
		m_primitiveRenderer->render(m_renderView, 0);
	}

	m_renderView->end();
	m_renderView->present();

	event->consume();
}

	}
}
