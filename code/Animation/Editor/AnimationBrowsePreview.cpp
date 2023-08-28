/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/AnimationAsset.h"
#include "Animation/Editor/AnimationBrowsePreview.h"
#include "Animation/Editor/SkeletonAsset.h"
#include "Core/Io/FileSystem.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Editor/IEditor.h"
#include "Model/Model.h"
#include "Model/Operations/Triangulate.h"
#include "Model/ModelCache.h"
#include "Model/ModelRasterizer.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimationBrowsePreview", 0, AnimationBrowsePreview, editor::IBrowsePreview)

TypeInfoSet AnimationBrowsePreview::getPreviewTypes() const
{
	return makeTypeInfoSet< AnimationAsset, SkeletonAsset >();
}

Ref< ui::Bitmap > AnimationBrowsePreview::generate(const editor::IEditor* editor, db::Instance* instance) const
{
    Ref< const editor::Asset > asset = instance->getObject< editor::Asset >();
	if (!asset)
		return nullptr;

	Ref< drawing::Image > meshThumb = new drawing::Image(
		drawing::PixelFormat::getR8G8B8A8(),
		128,
		128
	);
	meshThumb->clear(Color4f(0.4f, 0.4f, 0.6f, 0.0f));

	const std::wstring assetPath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	const std::wstring modelCachePath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.ModelCache.Path");

	const Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, asset->getFileName());

	Ref< model::Model > model = model::ModelCache(modelCachePath).get(fileName, L""); //, asset->getImportFilter());
	if (!model)
		return nullptr;

	if (!model::Triangulate().apply(*model))
		return nullptr;

	// Create list of texture references.
	// SmallMap< std::wstring, Guid > materialTextures;

	// First use textures from texture set.
	// const auto& textureSetId = asset->getTextureSet();
	// if (textureSetId.isNotNull())
	// {
	// 	Ref< const render::TextureSet > textureSet = editor->getSourceDatabase()->getObjectReadOnly< render::TextureSet >(textureSetId);
	// 	if (!textureSet)
	// 		return nullptr;

	// 	materialTextures = textureSet->get();
	// }

	// Then let explicit material textures override those from a texture set.
	// for (const auto& mt : asset->getMaterialTextures())
	// 	materialTextures[mt.first] = mt.second;

	// // Bind texture references in material maps.
	// for (auto& material : model->getMaterials())
	// {
	// 	auto diffuseMap = material.getDiffuseMap();
	// 	auto it = materialTextures.find(diffuseMap.name);
	// 	if (it != materialTextures.end())
	// 	{
	// 		diffuseMap.texture = it->second;
	// 		material.setDiffuseMap(diffuseMap);
	// 	}
	// }

	// Load texture images and attach to materials.
	// SmallMap< Path, Ref< drawing::Image > > images;
	// for (auto& material : model->getMaterials())
	// {
	// 	auto diffuseMap = material.getDiffuseMap();
	// 	if (diffuseMap.texture.isNotNull())
	// 	{
	// 		Ref< const render::TextureAsset > textureAsset = editor->getSourceDatabase()->getObjectReadOnly< render::TextureAsset >(diffuseMap.texture);
	// 		if (!textureAsset)
	// 			continue;

	// 		const Path filePath = FileSystem::getInstance().getAbsolutePath(assetPath, textureAsset->getFileName());
	// 		Ref< drawing::Image > image = images[filePath];
	// 		if (image == nullptr)
	// 		{
	// 			Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmRead);
	// 			if (file)
	// 			{
	// 				image = drawing::Image::load(file, textureAsset->getFileName().getExtension());
	// 				if (image && textureAsset->m_output.m_linearGamma)
	// 				{
	// 					// Convert to gamma color space.
	// 					drawing::GammaFilter gammaFilter(2.2f);
	// 					image->apply(&gammaFilter);							
	// 				}
	// 				images[filePath] = image;
	// 			}
	// 		}

	// 		diffuseMap.image = image;			
	// 		material.setDiffuseMap(diffuseMap);
	// 	}
	// }

	// Rasterize model.
	const Aabb3 boundingBox = model->getBoundingBox();
	const Scalar maxExtent = (boundingBox.getExtent() * Vector4(1.0f, 1.0f, 0.0f, 0.0f)).max();
	const Scalar invMaxExtent = 1.0_simd / maxExtent;
	const Matrix44 modelView = translate(0.0f, 0.0f, 3.0f) * scale(invMaxExtent, invMaxExtent, invMaxExtent) * rotateY(/*asset->getPreviewAngle()*/0.0f) * translate(-boundingBox.getCenter());
	model::ModelRasterizer().generate(model, modelView, meshThumb);

	drawing::ScaleFilter scaleFilter(
		64,
		64,
		drawing::ScaleFilter::MnAverage,
		drawing::ScaleFilter::MgLinear
	);
	meshThumb->apply(&scaleFilter);

	return new ui::Bitmap(meshThumb);
}

}
