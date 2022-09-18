#include "Scene.h"

#include <Framework/Debug/Debug.hpp>

#include <glm/glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fbxsdk.h>

static const int TRIANGLE_VERTEX_COUNT = 3;

//////////////////////////////////////////////////////////////////////////
//                                Texture                               //
//////////////////////////////////////////////////////////////////////////
std::unique_ptr<Texture> Texture::Load(const char* filePath)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(filePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	Debug_AssertMsg(pixels != nullptr, "failed to load texture image!");

	std::unique_ptr<Texture> texture = std::make_unique<Texture>();
	texture->TextureWidth = texWidth;
	texture->TextureHeight = texHeight;
	texture->TextureChannels = texChannels;
	texture->Pixels = pixels;

	return texture;
}

void Texture::DestroyPixelBuffer()
{
	stbi_image_free(Pixels);
	Pixels = nullptr;
}

//////////////////////////////////////////////////////////////////////////
//                        Scene - FBX Converter                         //
//////////////////////////////////////////////////////////////////////////
static glm::vec3 FbxToGlm(const FbxColor& in)
{
	return glm::vec3(static_cast<float>(in[0]), static_cast<float>(in[1]), static_cast<float>(in[2]));
}

static glm::vec2 FbxToGlm(const FbxDouble2& in)
{
	return glm::vec2(static_cast<float>(in[0]), static_cast<float>(in[1]));
}

static glm::vec3 FbxToGlm(const FbxDouble3& in)
{
	return glm::vec3(static_cast<float>(in[0]), static_cast<float>(in[1]), static_cast<float>(in[2]));
}

static glm::vec4 FbxToGlm(const FbxDouble4& in)
{
	return glm::vec4(static_cast<float>(in[0]), static_cast<float>(in[1]), static_cast<float>(in[2]), static_cast<float>(in[3]));
}

static glm::mat4x4 FbxToGlm(const FbxDouble4x4& in)
{
	return glm::mat4x4(FbxToGlm(in[0]), FbxToGlm(in[1]), FbxToGlm(in[2]), FbxToGlm(in[3]));
}

static void UpdateSceneObject(SceneObject& obj, FbxObject* fbxObject)
{
	obj.Name = fbxObject->GetName();
}

static void UpdateSceneNode(SceneNode& obj, FbxNode* fbxNode)
{
	UpdateSceneObject(obj, fbxNode);

	FbxAMatrix& globalTransform = fbxNode->EvaluateGlobalTransform();
	obj.WorldTransform = FbxToGlm(globalTransform);

	FbxAMatrix& localTransform = fbxNode->EvaluateLocalTransform();
	obj.LocalTransform = FbxToGlm(localTransform);
}

static void BuildMaterials(Scene& scene, FbxScene* fbxScene)
{
	int materialCount = fbxScene->GetMaterialCount();
	for (int i = 0; i < materialCount; ++i)
	{
		FbxSurfaceMaterial* fbxMaterial = fbxScene->GetMaterial(i);

		std::unique_ptr<Material> material = std::make_unique<Material>();
		UpdateSceneObject(*material, fbxMaterial);

		const FbxProperty fbxProperty = fbxMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
		if (fbxProperty.IsValid())
		{
			const int textureCount = fbxProperty.GetSrcObjectCount<FbxFileTexture>();
			if (textureCount > 0)
			{
				const FbxFileTexture* fbxTexture = fbxProperty.GetSrcObject<FbxFileTexture>();
				if (fbxTexture != nullptr)
				{
					const char* filePath = fbxTexture->GetFileName();

					std::unique_ptr<Texture> texture = Texture::Load(filePath);
					material->DiffuseTexture = texture.get();
					scene.Textures.push_back(std::move(texture));
				}
			}
		}

		scene.Materials.push_back(std::move(material));
	}
}

static void BuildResource(Scene& scene, FbxScene* fbxScene, FbxNode* fbxNode, FbxMesh* fbxMesh)
{
	std::unique_ptr<Model> model = std::make_unique<Model>();
	UpdateSceneNode(*model, fbxNode);

	fbxMesh->RemoveBadPolygons();
	fbxMesh->GenerateNormals();

	Debug_Assert(fbxMesh->GetElementMaterial() != nullptr);

	const int polygonCount = fbxMesh->GetPolygonCount();
	Debug_Assert(polygonCount > 0);

	const int vertexCount = polygonCount * TRIANGLE_VERTEX_COUNT;

	// Count the polygon count of each material
	FbxLayerElementArrayTemplate<int>* materialIndexArray = &fbxMesh->GetElementMaterial()->GetIndexArray();
	FbxGeometryElement::EMappingMode materialMappingMode = fbxMesh->GetElementMaterial()->GetMappingMode();
	if (materialMappingMode == FbxGeometryElement::eByPolygon)
	{
		Debug_Assert(materialIndexArray->GetCount() == polygonCount);

		// Count the faces of each material
		for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
		{
			const size_t materialIndex = materialIndexArray->GetAt(polygonIndex);
			const size_t requiredMeshSize = materialIndex + 1;
			if (model->Meshs.size() < requiredMeshSize)
			{
				model->Meshs.resize(requiredMeshSize);
			}

			model->Meshs[materialIndex].TriangleCount += 1;
		}
	}
	else if (materialMappingMode == FbxGeometryElement::eAllSame)
	{
		model->Meshs.resize(1);
		model->Meshs[0].TriangleCount = polygonCount;
	}

	// Initialize the index offset values
	{
		int currentIndexOffset = 0;
		for (Mesh& mesh : model->Meshs)
		{
			mesh.IndexOffset = currentIndexOffset;
			currentIndexOffset += mesh.TriangleCount;

			// reset the triangle count to fill in the index buffer
			mesh.TriangleCount = 0;
		}
	}

	// Populate the index array
	{
		model->Indices.resize(polygonCount * TRIANGLE_VERTEX_COUNT);

		for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
		{
			// The material for current face
			int materiaindex = 0;
			if (materialMappingMode == FbxGeometryElement::eByPolygon)
			{
				materiaindex = materialIndexArray->GetAt(polygonIndex);
			}

			Mesh& mesh = model->Meshs[materiaindex];
			const int polygonIndexOffset = mesh.IndexOffset + (mesh.TriangleCount * TRIANGLE_VERTEX_COUNT);

			for (int vertexIndex = 0; vertexIndex < TRIANGLE_VERTEX_COUNT; ++vertexIndex)
			{
				int polygonVertexIndex = (polygonIndex * TRIANGLE_VERTEX_COUNT) + vertexIndex;
				model->Indices[polygonIndexOffset + vertexIndex] = static_cast<uint32_t>(polygonVertexIndex);
			}

			mesh.TriangleCount += 1;
		}
	}

	// Populate the index array
	{
		for (int i = 0; i < model->Meshs.size(); ++i)
		{
			FbxSurfaceMaterial* fbxMaterial = fbxMesh->GetNode()->GetMaterial(i);
			int materialCount = fbxScene->GetMaterialCount();
			for (int materialIndex = 0; materialIndex < materialCount; ++materialIndex)
			{
				if (fbxMaterial == fbxScene->GetMaterial(materialIndex))
				{
					model->Meshs[i].MaterialIndex = materialIndex;
				}
			}
		}
	}

	// Populate the vertex array
	{
		const FbxVector4* controlPoints = fbxMesh->GetControlPoints();
		const FbxGeometryElementNormal* normalElement = nullptr;
		const FbxGeometryElementUV* uvElement = nullptr;
		const FbxLayerElementVertexColor* vertexColorSet = nullptr;

		// Normals - only support ByControlPoint mapping
		if (fbxMesh->GetElementNormalCount() > 0)
		{
			FbxGeometryElement::EMappingMode normalMappingMode = fbxMesh->GetElementNormal(0)->GetMappingMode();
			if (normalMappingMode == FbxGeometryElement::eByControlPoint ||
				normalMappingMode == FbxGeometryElement::eByPolygonVertex)
			{
				normalElement = fbxMesh->GetElementNormal(0);
			}
		}

		// UVs - only support ByControlPoint mapping
		if (fbxMesh->GetElementUVCount() > 0)
		{
			FbxGeometryElement::EMappingMode uvMappingMode = fbxMesh->GetElementUV(0)->GetMappingMode();
			if (uvMappingMode == FbxGeometryElement::eByControlPoint ||
				uvMappingMode == FbxGeometryElement::eByPolygonVertex)
			{
				uvElement = fbxMesh->GetElementUV(0);
			}
		}

		if (fbxMesh->GetLayer(0)->GetVertexColors() != nullptr)
		{
			vertexColorSet = fbxMesh->GetLayer(0)->GetVertexColors();
		}

		model->Vertices.resize(vertexCount);

		for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
		{
			for (int vertexIndex = 0; vertexIndex < TRIANGLE_VERTEX_COUNT; ++vertexIndex)
			{
				int polygonVertexIndex = (polygonIndex * TRIANGLE_VERTEX_COUNT) + vertexIndex;
				int index = fbxMesh->GetPolygonVertex(polygonIndex, vertexIndex);

				Vertex& vertex = model->Vertices[polygonVertexIndex];

				// Save the vertex position
				vertex.Position = glm::vec3(
					static_cast<float>(controlPoints[index][0]),
					static_cast<float>(controlPoints[index][1]),
					static_cast<float>(controlPoints[index][2])
				);

				// Save the vertex normal
				if (normalElement != nullptr)
				{
					int elementIndex = (normalElement->GetMappingMode() == FbxGeometryElement::eByControlPoint) ? index : polygonVertexIndex;
					if (normalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
					{
						elementIndex = normalElement->GetIndexArray().GetAt(elementIndex);
					}

					FbxVector4 normal = normalElement->GetDirectArray().GetAt(elementIndex);
					vertex.Normal = FbxToGlm(normal);
				}
				else
				{
					Debug_AssertMsg(false, "failed to get normal");
				}

				// Save the vertex UV
				if (uvElement != nullptr)
				{
					int elementIndex = (uvElement->GetMappingMode() == FbxGeometryElement::eByControlPoint) ? index : polygonVertexIndex;
					if (uvElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
					{
						elementIndex = uvElement->GetIndexArray().GetAt(elementIndex);
					}

					FbxVector2 uv = uvElement->GetDirectArray().GetAt(elementIndex);
					vertex.UV = FbxToGlm(uv);
					vertex.UV.y = 1.0f - vertex.UV.y; // inverted V
				}

				// Save the vertex color
				if (vertexColorSet != nullptr)
				{
					int elementIndex = (vertexColorSet->GetMappingMode() == FbxGeometryElement::eByControlPoint) ? index : polygonVertexIndex;
					if (vertexColorSet->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
					{
						elementIndex = vertexColorSet->GetIndexArray().GetAt(elementIndex);
					}

					FbxColor color = vertexColorSet->GetDirectArray().GetAt(elementIndex);
					vertex.Color = FbxToGlm(color);
				}
				else
				{
					vertex.Color = glm::vec3(1.0f, 1.0f, 1.0f);
				}
			}
		}
	}

	scene.Models.push_back(std::move(model));
}

static void BuildResource(Scene& scene, FbxScene* fbxScene, FbxNode* fbxNode, FbxCamera* fbxCamera)
{
	std::unique_ptr<Camera> camera = std::make_unique<Camera>();

	UpdateSceneNode(*camera, fbxNode);
	camera->FieldOfView = static_cast<float>(fbxCamera->FieldOfView);

	scene.Cameras.push_back(std::move(camera));
}

static void BuildResource(Scene& scene, FbxScene* fbxScene, FbxNode* fbxNode, FbxLight* fbxLight)
{
	FbxLight::EType fbxLightType = fbxLight->LightType.Get();

	LightType lightType = LightType::Unknown;
	if (fbxLightType == FbxLight::eDirectional) { lightType = LightType::Directional; }
	else if (fbxLightType == FbxLight::ePoint) { lightType = LightType::Point; }
	else if (fbxLightType == FbxLight::eSpot) { lightType = LightType::Spot; }
	else if (fbxLightType == FbxLight::eArea) { lightType = LightType::Area; }

	if (lightType == LightType::Unknown)
		return;

	std::unique_ptr<Light> light = std::make_unique<Light>();

	UpdateSceneNode(*light, fbxNode);
	light->LightType = lightType;
	light->Color = FbxToGlm(fbxLight->Color.Get());
	light->Intensity = (float)fbxLight->Intensity.Get();
	light->InnerAngle = (float)fbxLight->InnerAngle.Get();
	light->OuterAngle = (float)fbxLight->OuterAngle.Get();

	// Area Light Hacks
	if (light->LightType == LightType::Area)
	{
		light->InnerAngle = 45.0f;
		light->OuterAngle = 135.0;
	}

	// Note: Blender Lights
	// The power of sun lights is specified in Watts per square meter.
	// The power of point lights, spot lights, and area lights is specified in Watts.
	// But this is not the electrical Watts that consumer light bulbs are rated at.
	// It is Radiant Flux or Radiant Power which is also measured in Watts.
	// It is the energy radiated from the light in the form of visible light.

	float power = light->Intensity * 0.01f; // blender fbx export scales this value
	light->Intensity = power;

	scene.Lights.push_back(std::move(light));
}

static void BuildResources(Scene& scene, FbxScene* fbxScene, FbxNode* fbxNode)
{
	FbxNodeAttribute* nodeAttribute = fbxNode->GetNodeAttribute();
	if (nodeAttribute != nullptr)
	{
		if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			FbxMesh* fbxMesh = fbxNode->GetMesh();
			if (fbxMesh != nullptr)
			{
				BuildResource(scene, fbxScene, fbxNode, fbxMesh);
			}
		}

		if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eCamera)
		{
			FbxCamera* fbxCamera = fbxNode->GetCamera();
			if (fbxCamera != nullptr)
			{
				BuildResource(scene, fbxScene, fbxNode, fbxCamera);
			}
		}

		if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eLight)
		{
			FbxLight* fbxLight = fbxNode->GetLight();
			if (fbxLight != nullptr)
			{
				BuildResource(scene, fbxScene, fbxNode, fbxLight);
			}
		}
	}

	const int childCount = fbxNode->GetChildCount();
	for (int childIndex = 0; childIndex < childCount; ++childIndex)
	{
		BuildResources(scene, fbxScene, fbxNode->GetChild(childIndex));
	}
}

//////////////////////////////////////////////////////////////////////////
//                                Scene                                 //
//////////////////////////////////////////////////////////////////////////
std::unique_ptr<Scene> Scene::Load(const char* filePath)
{
	// The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
	FbxManager* fbxManager = FbxManager::Create();

	// Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ioSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
	fbxManager->SetIOSettings(ioSettings);

	// Create an FBX scene. This object holds most objects imported/exported from/to files.
	FbxScene* fbxScene = FbxScene::Create(fbxManager, "Scene");

	// Create the importer.
	FbxImporter* importer = FbxImporter::Create(fbxManager, "LoadScene");

	// Initialize the importer by providing a filename.
	std::unique_ptr<Scene> scene = std::make_unique<Scene>();
	if (importer->Initialize(filePath) == true)
	{
		if (importer->Import(fbxScene) == true)
		{
			// Convert Axis System to desired (Blender), if needed
			FbxAxisSystem sceneAxisSystem = fbxScene->GetGlobalSettings().GetAxisSystem();
			FbxAxisSystem desiredAxisSystem(FbxAxisSystem::eMayaZUp);
			if (sceneAxisSystem != desiredAxisSystem)
			{
				desiredAxisSystem.ConvertScene(fbxScene);
			}

			// Convert Unit System to desired, if needed
			FbxSystemUnit sceneSystemUnit = fbxScene->GetGlobalSettings().GetSystemUnit();
			FbxSystemUnit desiredSystemUnit = FbxSystemUnit::m;
			if (sceneSystemUnit != desiredSystemUnit)
			{
				desiredSystemUnit.ConvertScene(fbxScene);
			}

			// Convert mesh, NURBS and patch into triangle mesh

			// pReplace - If true, replace the original meshes with the new triangulated meshes on all the nodes, and delete the original meshes. Otherwise, original meshes are left untouched.
			constexpr bool TriangulateOpt_Replace = true;

			FbxGeometryConverter geomConverter(fbxManager);
			geomConverter.Triangulate(fbxScene, TriangulateOpt_Replace);

			// Build the graphics resources
			BuildMaterials(*scene, fbxScene);
			BuildResources(*scene, fbxScene, fbxScene->GetRootNode());
		}
		else
		{
			Debug_AssertMsg(false, "failed to import scene");
		}
	}
	else
	{
		Debug_AssertMsg(false, "failed to initialize impoter");
	}

	// Destroy the importer to release the file.
	importer->Destroy();
	importer = nullptr;

	// Destroy the manager.
	fbxManager->Destroy();
	fbxManager = nullptr;

	return scene;
}