#include "Flash/Acc/AccShapeResources.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const resource::Id< render::Shader > c_idShaderSolid(Guid(L"{4F6F6CCE-97EC-624D-96B7-842F1D99D060}"));
const resource::Id< render::Shader > c_idShaderTextured(Guid(L"{049F4B08-1A54-DB4C-86CC-B533BCFFC65D}"));
const resource::Id< render::Shader > c_idShaderSolidMask(Guid(L"{D46877B9-0F90-3A42-AB2D-7346AA607233}"));
const resource::Id< render::Shader > c_idShaderTexturedMask(Guid(L"{5CDDBEC8-1629-0A4E-ACE5-C8186072D694}"));
const resource::Id< render::Shader > c_idShaderIncrementMask(Guid(L"{8DCBCF05-4640-884E-95AC-F090510788F4}"));
const resource::Id< render::Shader > c_idShaderDecrementMask(Guid(L"{57F6F4DF-F4EE-6740-907C-027A3A2596D7}"));

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AccShapeResources", AccShapeResources, Object)

bool AccShapeResources::create(resource::IResourceManager* resourceManager)
{
	if (!resourceManager->bind(c_idShaderSolid, m_shaderSolid))
		return false;
	if (!resourceManager->bind(c_idShaderTextured, m_shaderTextured))
		return false;
	if (!resourceManager->bind(c_idShaderSolidMask, m_shaderSolidMask))
		return false;
	if (!resourceManager->bind(c_idShaderTexturedMask, m_shaderTexturedMask))
		return false;
	if (!resourceManager->bind(c_idShaderIncrementMask, m_shaderIncrementMask))
		return false;
	if (!resourceManager->bind(c_idShaderDecrementMask, m_shaderDecrementMask))
		return false;

	m_handleTransform = render::getParameterHandle(L"Flash_Transform");
	m_handleFrameSize = render::getParameterHandle(L"Flash_FrameSize");
	m_handleViewSize = render::getParameterHandle(L"Flash_ViewSize");
	m_handleViewOffset = render::getParameterHandle(L"Flash_ViewOffset");
	m_handleScreenOffsetScale = render::getParameterHandle(L"Flash_ScreenOffsetScale");
	m_handleCxFormMul = render::getParameterHandle(L"Flash_CxFormMul");
	m_handleCxFormAdd = render::getParameterHandle(L"Flash_CxFormAdd");
	m_handleTexture = render::getParameterHandle(L"Flash_Texture");
	m_handleTextureRect = render::getParameterHandle(L"Flash_TextureRect");
	m_handleTextureMatrix0 = render::getParameterHandle(L"Flash_TextureMatrix0");
	m_handleTextureMatrix1 = render::getParameterHandle(L"Flash_TextureMatrix1");
	m_handleTextureClamp = render::getParameterHandle(L"Flash_TextureClamp");

	return true;
}

void AccShapeResources::destroy()
{
	m_shaderSolid.clear();
	m_shaderTextured.clear();
	m_shaderSolidMask.clear();
	m_shaderTexturedMask.clear();
	m_shaderIncrementMask.clear();
	m_shaderDecrementMask.clear();
}

	}
}
