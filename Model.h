#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"

#include <list> //std::list
#include <array> //std::array
#include <memory> //std::unique_ptr

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include "glm_helper.h"
#include "AnimData.h"



#define STB_IMAGE_IMPLEMENTATION  
#include "stb_image.h"

using namespace std;

class Model
{
public:
	// model data 
	vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
	vector<Mesh>    meshes;
	string directory;
	bool gammaCorrection;



	// constructor, expects a filepath to a 3D model.
	Model(string const& path, bool gamma = false) : gammaCorrection(gamma)
	{
		loadModel(path);
	}

	// draws the model, and thus all its meshes
	void Draw(Shader& shader)
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].Draw(shader);
	}

	auto& GetBoneInfoMap() { return m_BoneInfoMap; }
	int& GetBoneCount() { return m_BoneCounter; }


private:

	std::map<string, BoneInfo> m_BoneInfoMap;
	int m_BoneCounter = 0;

	// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void loadModel(string const& path)
	{
		// read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
		// check for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		// retrieve the directory path of the filepath
		directory = path.substr(0, path.find_last_of('/'));

		// process ASSIMP's root node recursively
		processNode(scene->mRootNode, scene);
	}

	// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode* node, const aiScene* scene)
	{
		// process each mesh located at the current node
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			// the node object only contains indices to index the actual objects in the scene. 
			// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}

	}

	void SetVertexBoneDataToDefault(Vertex& vertex)
	{
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
		{
			vertex.m_BoneIDs[i] = -1;
			vertex.m_Weights[i] = 0.0f;
		}
	}


	Mesh processMesh(aiMesh* mesh, const aiScene* scene)
	{
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			SetVertexBoneDataToDefault(vertex);
			vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
			vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);

			if (mesh->mTextureCoords[0])
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);

			vertices.push_back(vertex);
		}
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

		ExtractBoneWeightForVertices(vertices, mesh, scene);

		return Mesh(vertices, indices, textures);
	}

	void SetVertexBoneData(Vertex& vertex, int boneID, float weight)
	{
		for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
		{
			if (vertex.m_BoneIDs[i] < 0)
			{
				vertex.m_Weights[i] = weight;
				vertex.m_BoneIDs[i] = boneID;
				break;
			}
		}
	}


	void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
	{
		auto& boneInfoMap = m_BoneInfoMap;
		int& boneCount = m_BoneCounter;

		for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
		{
			int boneID = -1;
			std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
			if (boneInfoMap.find(boneName) == boneInfoMap.end())
			{
				BoneInfo newBoneInfo;
				newBoneInfo.id = boneCount;
				newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
				boneInfoMap[boneName] = newBoneInfo;
				boneID = boneCount;
				boneCount++;
			}
			else
			{
				boneID = boneInfoMap[boneName].id;
			}
			assert(boneID != -1);
			auto weights = mesh->mBones[boneIndex]->mWeights;
			int numWeights = mesh->mBones[boneIndex]->mNumWeights;

			for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
			{
				int vertexId = weights[weightIndex].mVertexId;
				float weight = weights[weightIndex].mWeight;
				assert(vertexId <= vertices.size());
				SetVertexBoneData(vertices[vertexId], boneID, weight);
			}
		}
	}


	unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false)
	{
		string filename = string(path);
		filename = directory + '/' + filename;

		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << path << std::endl;
			stbi_image_free(data);
		}

		return textureID;
	}

	// checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required info is returned as a Texture struct.
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
	{
		vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
			bool skip = false;
			for (unsigned int j = 0; j < textures_loaded.size(); j++)
			{
				if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
				{
					textures.push_back(textures_loaded[j]);
					skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
					break;
				}
			}
			if (!skip)
			{   // if texture hasn't been loaded already, load it
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory);
				texture.type = typeName;
				texture.path = str.C_Str();
				textures.push_back(texture);
				textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
			}
		}
		return textures;
	}
};



class Transform
{
protected:
	//Local space information
	glm::vec3 m_pos = { 0.0f, 0.0f, 0.0f };
	glm::vec3 m_eulerRot = { 0.0f, 0.0f, 0.0f }; //In degrees
	glm::vec3 m_scale = { 1.0f, 1.0f, 1.0f };

	//Global space informaiton concatenate in matrix
	glm::mat4 m_modelMatrix = glm::mat4(1.0f);

	//Dirty flag
	bool m_isDirty = true;

protected:
	glm::mat4 getLocalModelMatrix()
	{
		const glm::mat4 transformX = glm::rotate(glm::mat4(1.0f), glm::radians(m_eulerRot.x), glm::vec3(1.0f, 0.0f, 0.0f));
		const glm::mat4 transformY = glm::rotate(glm::mat4(1.0f), glm::radians(m_eulerRot.y), glm::vec3(0.0f, 1.0f, 0.0f));
		const glm::mat4 transformZ = glm::rotate(glm::mat4(1.0f), glm::radians(m_eulerRot.z), glm::vec3(0.0f, 0.0f, 1.0f));

		// Y * X * Z
		const glm::mat4 roationMatrix = transformY * transformX * transformZ;

		// translation * rotation * scale (also know as TRS matrix)
		return glm::translate(glm::mat4(1.0f), m_pos) * roationMatrix * glm::scale(glm::mat4(1.0f), m_scale);
	}
public:

	void computeModelMatrix()
	{
		m_modelMatrix = getLocalModelMatrix();
	}

	void computeModelMatrix(const glm::mat4& parentGlobalModelMatrix)
	{
		m_modelMatrix = parentGlobalModelMatrix * getLocalModelMatrix();
	}

	void setLocalPosition(const glm::vec3& newPosition)
	{
		m_pos = newPosition;
		m_isDirty = true;
	}

	void setLocalRotation(const glm::vec3& newRotation)
	{
		m_eulerRot = newRotation;
		m_isDirty = true;
	}

	void setLocalScale(const glm::vec3& newScale)
	{
		m_scale = newScale;
		m_isDirty = true;
	}

	const glm::vec3& getGlobalPosition() const
	{
		return glm::vec3(m_modelMatrix[3]);
	}

	const glm::vec3& getLocalPosition() const
	{
		return m_pos;
	}

	const glm::vec3& getLocalRotation() const
	{
		return m_eulerRot;
	}

	const glm::vec3& getLocalScale() const
	{
		return m_scale;
	}

	const glm::mat4& getModelMatrix() const
	{
		return m_modelMatrix;
	}

	glm::vec3 getRight() const
	{
		return glm::vec3(m_modelMatrix[0]);
	}


	glm::vec3 getUp() const
	{
		return glm::vec3(m_modelMatrix[1]);
	}

	glm::vec3 getBackward() const
	{
		return glm::vec3(m_modelMatrix[2]);
	}

	glm::vec3 getForward() const
	{
		return -glm::vec3(m_modelMatrix[2]);
	}

	glm::vec3 getGlobalScale() const
	{
		return { glm::length(getRight()), glm::length(getUp()), glm::length(getBackward()) };
	}

	bool isDirty() const
	{
		return m_isDirty;
	}
};

struct Plan
{
	glm::vec3 normal = { 0.f, 1.f, 0.f }; // unit vector
	float     distance = 0.f;        // Distance with origin

	Plan() = default;

	Plan(const glm::vec3& p1, const glm::vec3& norm)
		: normal(glm::normalize(norm)),
		distance(glm::dot(normal, p1))
	{}

	float getSignedDistanceToPlan(const glm::vec3& point) const
	{
		return glm::dot(normal, point) - distance;
	}
};

struct Frustum
{
	Plan topFace;
	Plan bottomFace;

	Plan rightFace;
	Plan leftFace;

	Plan farFace;
	Plan nearFace;

};

struct BoundingVolume
{
	virtual bool isOnFrustum(const Frustum& camFrustum, const Transform& transform) const = 0;

	virtual bool isOnOrForwardPlan(const Plan& plan) const = 0;

	bool isOnFrustum(const Frustum& camFrustum) const
	{
		return (isOnOrForwardPlan(camFrustum.leftFace) &&
			isOnOrForwardPlan(camFrustum.rightFace) &&
			isOnOrForwardPlan(camFrustum.topFace) &&
			isOnOrForwardPlan(camFrustum.bottomFace) &&
			isOnOrForwardPlan(camFrustum.nearFace) &&
			isOnOrForwardPlan(camFrustum.farFace));
	};
};

struct Sphere : public BoundingVolume
{
	glm::vec3 center{ 0.f, 0.f, 0.f };
	float radius{ 0.f };

	Sphere(const glm::vec3& inCenter, float inRadius)
		: BoundingVolume{}, center{ inCenter }, radius{ inRadius }
	{}

	bool isOnOrForwardPlan(const Plan& plan) const final
	{
		return plan.getSignedDistanceToPlan(center) > -radius;
	}

	bool isOnFrustum(const Frustum& camFrustum, const Transform& transform) const final
	{
		//Get global scale thanks to our transform
		const glm::vec3 globalScale = transform.getGlobalScale();

		//Get our global center with process it with the global model matrix of our transform
		const glm::vec3 globalCenter{ transform.getModelMatrix() * glm::vec4(center, 1.f) };

		//To wrap correctly our shape, we need the maximum scale scalar.
		const float maxScale = std::max(std::max(globalScale.x, globalScale.y), globalScale.z);

		//Max scale is assuming for the diameter. So, we need the half to apply it to our radius
		Sphere globalSphere(globalCenter, radius * (maxScale * 0.5f));

		//Check Firstly the result that have the most chance to faillure to avoid to call all functions.
		return (globalSphere.isOnOrForwardPlan(camFrustum.leftFace) &&
			globalSphere.isOnOrForwardPlan(camFrustum.rightFace) &&
			globalSphere.isOnOrForwardPlan(camFrustum.farFace) &&
			globalSphere.isOnOrForwardPlan(camFrustum.nearFace) &&
			globalSphere.isOnOrForwardPlan(camFrustum.topFace) &&
			globalSphere.isOnOrForwardPlan(camFrustum.bottomFace));
	};
};

struct SquareAABB : public BoundingVolume
{
	glm::vec3 center{ 0.f, 0.f, 0.f };
	float extent{ 0.f };

	SquareAABB(const glm::vec3& inCenter, float inExtent)
		: BoundingVolume{}, center{ inCenter }, extent{ inExtent }
	{}

	bool isOnOrForwardPlan(const Plan& plan) const final
	{
		// Compute the projection interval radius of b onto L(t) = b.c + t * p.n
		const float r = extent * (std::abs(plan.normal.x) + std::abs(plan.normal.y) + std::abs(plan.normal.z));
		return -r <= plan.getSignedDistanceToPlan(center);
	}

	bool isOnFrustum(const Frustum& camFrustum, const Transform& transform) const final
	{
		//Get global scale thanks to our transform
		const glm::vec3 globalCenter{ transform.getModelMatrix() * glm::vec4(center, 1.f) };

		// Scaled orientation
		const glm::vec3 right = transform.getRight() * extent;
		const glm::vec3 up = transform.getUp() * extent;
		const glm::vec3 forward = transform.getForward() * extent;

		const float newIi = std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, right)) +
			std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, up)) +
			std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, forward));

		const float newIj = std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, right)) +
			std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, up)) +
			std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, forward));

		const float newIk = std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, right)) +
			std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, up)) +
			std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, forward));

		const SquareAABB globalAABB(globalCenter, std::max(std::max(newIi, newIj), newIk));

		return (globalAABB.isOnOrForwardPlan(camFrustum.leftFace) &&
			globalAABB.isOnOrForwardPlan(camFrustum.rightFace) &&
			globalAABB.isOnOrForwardPlan(camFrustum.topFace) &&
			globalAABB.isOnOrForwardPlan(camFrustum.bottomFace) &&
			globalAABB.isOnOrForwardPlan(camFrustum.nearFace) &&
			globalAABB.isOnOrForwardPlan(camFrustum.farFace));
	};
};

struct AABB : public BoundingVolume
{
	glm::vec3 center{ 0.f, 0.f, 0.f };
	glm::vec3 extents{ 0.f, 0.f, 0.f };

	AABB(const glm::vec3& min, const glm::vec3& max)
		: BoundingVolume{}, center{ (max + min) * 0.5f }, extents{ max.x - center.x, max.y - center.y, max.z - center.z }
	{}

	AABB(const glm::vec3& inCenter, float iI, float iJ, float iK)
		: BoundingVolume{}, center{ inCenter }, extents{ iI, iJ, iK }
	{}

	std::array<glm::vec3, 8> getVertice() const
	{
		std::array<glm::vec3, 8> vertice;
		vertice[0] = { center.x - extents.x, center.y - extents.y, center.z - extents.z };
		vertice[1] = { center.x + extents.x, center.y - extents.y, center.z - extents.z };
		vertice[2] = { center.x - extents.x, center.y + extents.y, center.z - extents.z };
		vertice[3] = { center.x + extents.x, center.y + extents.y, center.z - extents.z };
		vertice[4] = { center.x - extents.x, center.y - extents.y, center.z + extents.z };
		vertice[5] = { center.x + extents.x, center.y - extents.y, center.z + extents.z };
		vertice[6] = { center.x - extents.x, center.y + extents.y, center.z + extents.z };
		vertice[7] = { center.x + extents.x, center.y + extents.y, center.z + extents.z };
		return vertice;
	}

	bool isOnOrForwardPlan(const Plan& plan) const final
	{
		// Compute the projection interval radius of b onto L(t) = b.c + t * p.n
		const float r = extents.x * std::abs(plan.normal.x) + extents.y * std::abs(plan.normal.y) +
			extents.z * std::abs(plan.normal.z);

		return -r <= plan.getSignedDistanceToPlan(center);
	}

	bool isOnFrustum(const Frustum& camFrustum, const Transform& transform) const final
	{
		//Get global scale thanks to our transform
		const glm::vec3 globalCenter{ transform.getModelMatrix() * glm::vec4(center, 1.f) };

		// Scaled orientation
		const glm::vec3 right = transform.getRight() * extents.x;
		const glm::vec3 up = transform.getUp() * extents.y;
		const glm::vec3 forward = transform.getForward() * extents.z;

		const float newIi = std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, right)) +
			std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, up)) + 
			std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, forward));

		const float newIj = std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, right)) +
			std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, up)) +
			std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, forward));

		const float newIk = std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, right)) +
			std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, up)) +
			std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, forward));

		const AABB globalAABB(globalCenter, newIi, newIj, newIk);

		return (globalAABB.isOnOrForwardPlan(camFrustum.leftFace) &&
			globalAABB.isOnOrForwardPlan(camFrustum.rightFace) &&
			globalAABB.isOnOrForwardPlan(camFrustum.topFace) &&
			globalAABB.isOnOrForwardPlan(camFrustum.bottomFace) &&
			globalAABB.isOnOrForwardPlan(camFrustum.nearFace) &&
			globalAABB.isOnOrForwardPlan(camFrustum.farFace));
	};
};

Frustum createFrustumFromCamera(const Camera& cam, float aspect, float fovY, float zNear, float zFar)
{
	Frustum     frustum;
	const float halfVSide = zFar * tanf(fovY * .5f);
	const float halfHSide = halfVSide * aspect;
	const glm::vec3 frontMultFar = zFar * cam.Front;

	frustum.nearFace = { cam.Position + zNear * cam.Front, cam.Front };
	frustum.farFace = { cam.Position + frontMultFar, -cam.Front };
	frustum.rightFace = { cam.Position, glm::cross(cam.Up, frontMultFar + cam.Right * halfHSide) };
	frustum.leftFace = { cam.Position, glm::cross(frontMultFar - cam.Right * halfHSide, cam.Up) };
	frustum.topFace = { cam.Position, glm::cross(cam.Right, frontMultFar - cam.Up * halfVSide) };
	frustum.bottomFace = { cam.Position, glm::cross(3.0f *(frontMultFar + cam.Up * halfVSide), cam.Right) };

	return frustum;
}

AABB generateAABB(const Model& model)
{
	glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());
	for (auto&& mesh : model.meshes)
	{
		for (auto&& vertex : mesh.vertices)
		{
			minAABB.x = std::min(minAABB.x, vertex.Position.x);
			minAABB.y = std::min(minAABB.y, vertex.Position.y);
			minAABB.z = std::min(minAABB.z, vertex.Position.z);

			maxAABB.x = std::max(maxAABB.x, vertex.Position.x);
			maxAABB.y = std::max(maxAABB.y, vertex.Position.y);
			maxAABB.z = std::max(maxAABB.z, vertex.Position.z);
		}
	}
	return AABB(minAABB, maxAABB);
}

Sphere generateSphereBV(const Model& model)
{
	glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());
	for (auto&& mesh : model.meshes)
	{
		for (auto&& vertex : mesh.vertices)
		{
			minAABB.x = std::min(minAABB.x, vertex.Position.x);
			minAABB.y = std::min(minAABB.y, vertex.Position.y);
			minAABB.z = std::min(minAABB.z, vertex.Position.z);

			maxAABB.x = std::max(maxAABB.x, vertex.Position.x);
			maxAABB.y = std::max(maxAABB.y, vertex.Position.y);
			maxAABB.z = std::max(maxAABB.z, vertex.Position.z);
		}
	}

	return Sphere((maxAABB + minAABB) * 0.5f, glm::length(minAABB - maxAABB));
}

class Entity
{
public:
	//Scene graph
	std::list<std::unique_ptr<Entity>> children;
	Entity* parent = nullptr;

	//Space information
	Transform transform;

	Model* pModel = nullptr;
	std::unique_ptr<AABB> boundingVolume;


	// constructor, expects a filepath to a 3D model.
	Entity(Model& model) : pModel{ &model }
	{
		boundingVolume = std::make_unique<AABB>(generateAABB(model));
		//boundingVolume = std::make_unique<Sphere>(generateSphereBV(model));
	}

	AABB getGlobalAABB()
	{
		//Get global scale thanks to our transform
		const glm::vec3 globalCenter{ transform.getModelMatrix() * glm::vec4(boundingVolume->center, 1.f) };

		// Scaled orientation
		const glm::vec3 right = transform.getRight() * boundingVolume->extents.x;
		const glm::vec3 up = transform.getUp() * boundingVolume->extents.y;
		const glm::vec3 forward = transform.getForward() * boundingVolume->extents.z;

		const float newIi = std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, right)) +
			std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, up)) +
			std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, forward));

		const float newIj = std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, right)) +
			std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, up)) +
			std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, forward));

		const float newIk = std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, right)) +
			std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, up)) +
			std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, forward));

		return AABB(globalCenter, newIi, newIj, newIk);
	}

	//Add child. Argument input is argument of any constructor that you create. By default you can use the default constructor and don't put argument input.
	template<typename... TArgs>
	void addChild(TArgs&... args)
	{
		children.emplace_back(std::make_unique<Entity>(args...));
		children.back()->parent = this;
	}

	//Update transform if it was changed
	void updateSelfAndChild()
	{
		if (!transform.isDirty())
			return;

		forceUpdateSelfAndChild();
	}

	//Force update of transform even if local space don't change
	void forceUpdateSelfAndChild()
	{
		if (parent)
			transform.computeModelMatrix(parent->transform.getModelMatrix());
		else
			transform.computeModelMatrix();

		for (auto&& child : children)
		{
			child->forceUpdateSelfAndChild();
		}
	}


	void drawSelfAndChild(const Frustum& frustum, Shader& ourShader, unsigned int& display, unsigned int& total)
	{
		if (boundingVolume->isOnFrustum(frustum, transform))
		{
			ourShader.setMat4("model", transform.getModelMatrix());
			pModel->Draw(ourShader);
			display++;
		}
		total++;

		for (auto&& child : children)
		{
			child->drawSelfAndChild(frustum, ourShader, display, total);
		}
	}

	void locAndScale(glm::vec3 newPosition,const float scale) {
		
		transform.setLocalPosition(newPosition);
		
		transform.setLocalScale({scale, scale, scale});
	}


};

bool CheckCollision(Entity& one, Entity& two) // AABB - AABB collision
{

	AABB posOne = one.getGlobalAABB();
	AABB posTwo = two.getGlobalAABB();




	// Calculate the separation distance between the two boxes on each axis
	float xSeparation = abs(posOne.center.x - posTwo.center.x) - (posOne.extents.x + posTwo.extents.x) / 2.0f;
	float ySeparation = abs(posOne.center.y - posTwo.center.y) - (posOne.extents.y + posTwo.extents.y) / 2.0f;
	float zSeparation = abs(posOne.center.z - posTwo.center.z) - (posOne.extents.z + posTwo.extents.z) / 2.0f;

	// If the separation distance is greater than zero on any axis, the boxes are not colliding
	if (xSeparation * 1.5f > 0 || ySeparation * 1.5f > 0 || zSeparation * 1.5f > 0) {
		return false;
	}
	else {
		return true;
	}
}



#endif