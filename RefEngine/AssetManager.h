#pragma once

#include "graphics/GLTypes.h"
#include "graphics/Texture.h"
#include "graphics/Material.h"
#include "CachingFactory.h"

#include <memory>
#include <string>

namespace reng {

typedef CachingFactory<ShaderId, const char*, ShaderType> ShaderFactory;
typedef CachingFactory<ProgramId, ShaderId, ShaderId> ProgramFactory;
typedef CachingFactory<Texture, const char*, unsigned int> TextureFactory;
typedef CachingFactory<Material, const char*> MaterialFactory;


class BadAssetLoad : public std::runtime_error 
{
public:
	BadAssetLoad(const char* message) : std::runtime_error(message) {}
	BadAssetLoad(const std::string& message) : std::runtime_error(message.c_str()) {}
};

/**
 * A simple wrapper around Container to provide cached access to commonly used assets.
 */
class AssetManager
{
public:
	AssetManager();

	ShaderId CreateShader(const char* shaderFileName, ShaderType shaderType);

	Texture LoadTexture(const char* fileName, unsigned int textureUnit=0);

    ProgramId LoadProgram(const char* vertShaderFileName, const char* fragShaderFileName);

    ProgramId LinkProgram(ShaderId fragmentShaderId, ShaderId vertexShaderId);

    std::shared_ptr<Material> CreateMaterial(ProgramId program);

    std::shared_ptr<Material> CreateMaterial(const char* vertShaderFileName, const char* fragShaderFileName);

private:
	std::unique_ptr<ShaderFactory> m_pShaderFactory;
	std::unique_ptr<ProgramFactory> m_pProgramFactory;
    std::unique_ptr<TextureFactory> m_pTextureFactory;
    std::unique_ptr<MaterialFactory> m_pMaterialFactory;
};

}
