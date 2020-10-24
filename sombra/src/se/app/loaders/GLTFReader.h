#ifndef GLTF_READER_H
#define GLTF_READER_H

#include <string>
#include <nlohmann/json_fwd.hpp>
#include "se/graphics/3D/RenderableMesh.h"
#include "se/animation/IAnimation.h"
#include "se/animation/CompositeAnimator.h"
#include "se/animation/AnimationNode.h"
#include "se/app/loaders/SceneReader.h"
#include "se/app/graphics/Image.h"
#include "se/app/graphics/Material.h"
#include "se/app/MeshComponent.h"
#include "se/app/SkinComponent.h"
#include "se/app/CameraComponent.h"
#include "se/app/LightComponent.h"

namespace se::app {

	/**
	 * Class GLTFReader, TODO:
	 */
	class GLTFReader : public SceneReader
	{
	private:	// Nested types
		using Buffer = std::vector<std::byte>;
		using TextureSPtr = std::shared_ptr<graphics::Texture>;
		using LightSourceSPtr = std::shared_ptr<LightSource>;
		using SkinSPtr = std::shared_ptr<Skin>;
		using SceneUPtr = std::unique_ptr<Scene>;
		using Vec3Animation = animation::IAnimation<glm::vec3>;
		using QuatAnimation = animation::IAnimation<glm::quat>;
		using Vec3AnimationSPtr = std::shared_ptr<Vec3Animation>;
		using QuatAnimationSPtr = std::shared_ptr<QuatAnimation>;
		using IAnimatorUPtr = std::unique_ptr<animation::IAnimator>;
		using CAnimatorSPtr = std::shared_ptr<animation::CompositeAnimator>;
		using IndexVector = std::vector<std::size_t>;

		/** Struct FileFormat, holds the version of a valid GLTF file format */
		struct FileFormat
		{
			static constexpr int sVersion = 2;
			static constexpr int sRevision = 0;
		};

		/** Struct Accessor, holds the data of a GLTF accessor */
		struct Accessor
		{
			std::size_t bufferViewId, byteOffset, count, componentSize;
			graphics::TypeId componentTypeId;
			bool normalized;
		};

		/** Struct BufferView, holds the data of a GLTF bufferView */
		struct BufferView
		{
			std::size_t bufferId, length, offset, stride;
			enum class Target { Array, ElementArray, Undefined } target;
		};

		/** Struct Sampler, holds the data of a GLTF sampler */
		struct Sampler
		{
			graphics::TextureFilter filters[2];
			graphics::TextureWrap wraps[2];
		};

		/** Struct MaterialTechnique holds the data related to a Material */
		struct MaterialTechnique
		{
			std::string name;
			Material material;
			TechniqueSPtr technique;
			TechniqueSPtr techniqueSkin;
		};

		/** Struct Node, holds the data of a Node in the GLTF Node hierarchy */
		struct Node
		{
			animation::NodeData nodeData;
			std::vector<std::size_t> children;
			Entity entity = kNullEntity;
			bool hasSkin = false;
			std::size_t skinIndex = 0;
			animation::AnimationNode* animationNode = nullptr;
		};

		/** Struct GLTFData, it holds temporaly loaded data from a GLTF file */
		struct GLTFData
		{
			std::string fileName;
			std::string basePath;
			Scene& scene;

			std::vector<Accessor> accessors;
			std::vector<Buffer> buffers;
			std::vector<BufferView> bufferViews;
			std::vector<Sampler> samplers;
			std::vector<Image<unsigned char>> images;
			std::vector<TextureSPtr> textures;
			std::vector<MaterialTechnique> materials;
			std::vector<MeshComponent> meshComponents;
			std::vector<LightSourceSPtr> lightSources;
			std::vector<SkinSPtr> skins;
			std::vector<IndexVector> jointIndices;
			std::vector<CameraComponent> cameraComponents;
			std::vector<Node> nodes;
			std::vector<SceneUPtr> scenes;
			std::vector<CAnimatorSPtr> compositeAnimators;

			GLTFData(Scene& scene) : scene(scene) {};
		};

	private:	// Attributes
		/** The temporarily read GLTF data */
		std::unique_ptr<GLTFData> mGLTFData;

	public:		// Functions
		/** @copydoc SceneReader::SceneReader(Application&,TechniqueBuilder&) */
		GLTFReader(
			Application& application, TechniqueBuilder& techniqueBuilder
		) : SceneReader(application, techniqueBuilder) {};

		/** Parses the given GLTF file and stores the result in the given
		 * Scenes object
		 *
		 * @param	path the path to the GLTF file to parse
		 * @param	output the Scene object where the file data will be stored
		 * @return	a Result object with the result of the load operation */
		Result load(const std::string& path, Scene& output) override;
	private:
		/** Reads the JSON file located at the given path
		 *
		 * @param	path the path where the file to read is located
		 * @param	output the JSON object where the data will be stored
		 * @return	a Result object with the result of the read operation */
		static Result readJSON(const std::string& path, nlohmann::json& output);

		/** Parses the given JSON object to the given Scenes object
		 *
		 * @param	jsonGLTF the JSON object where the source data is stored
		 * @return	a Result object with the result of the parse operation */
		Result parseGLTF(const nlohmann::json& jsonGLTF);

		/** Checks the version of the given GLTF JSON asset
		 *
		 * @param	jsonAsset the GLTF JSON asset to check
		 * @param	version the expected version number of the asset
		 * @param	revision the expected revision number of the asset
		 * @return	true if the JSON asset version is accepted, false
		 *			otherwise */
		static bool checkAssetVersion(
			const nlohmann::json& jsonAsset, int version, int revision
		);

		/** Reads the Buffer from the given GLTF JSON Buffer and appends it to
		 * mGLTFData
		 *
		 * @param	jsonBuffer the json object with the buffer data to load
		 * @return	a Result object with the result of the parse operation
		 * @note	it only supports buffers in GLB files */
		Result parseBuffer(const nlohmann::json& jsonBuffer);

		/** Reads the given GLTF JSON BufferView and appends it to mGLTFData
		 *
		 * @param	jsonBufferView the json object with the BufferView data to
		 *			load
		 * @return	a Result object with the result of the parse operation */
		Result parseBufferView(const nlohmann::json& jsonBufferView);

		/** Loads the Array/Index Buffer from the given GLTF JSON Accessor and
		 * appends it to mGLTFData
		 *
		 * @param	jsonAccessor the json object with the Accessor data to load
		 * @return	a Result object with the result of the parse operation */
		Result parseAccessor(const nlohmann::json& jsonAccessor);

		/** Loads the texture Sampler from the given GLTF JSON Sampler and
		 * appends it to mGLTFData
		 *
		 * @param	jsonSampler the json object with the Sampler data to load
		 * @return	a Result object with the result of the parse operation */
		Result parseSampler(const nlohmann::json& jsonSampler);

		/** Loads the given GLTF JSON image and appends it to mGLTFData
		 *
		 * @param	jsonImage the json object with the Image data to load
		 * @return	a Result object with the result of the parse operation
		 * @note	bufferView images aren't supported yet */
		Result parseImage(const nlohmann::json& jsonImage);

		/** Loads the texture from the given GLTF JSON Texture and appends it
		 * to mGLTFData
		 *
		 * @param	jsonTexture the json object with the Texture data to load
		 * @return	a Result object with the result of the parse operation */
		Result parseTexture(const nlohmann::json& jsonTexture);

		/** Creates a new Material from the given GLTF JSON Material and
		 * appends it to mGLTFData
		 *
		 * @param	jsonMaterial the JSON object with the Material to parse
		 * @return	a Result object with the result of the parse operation */
		Result parseMaterial(const nlohmann::json& jsonMaterial);

		/** Creates a new RenderableMesh from the given GLTF JSON primitive and
		 * returns it in the given parameter
		 *
		 * @param	jsonPrimitive the JSON object with the Primitive to parse
		 * @param	out a reference to the RenderableMesh where the Primitive
		 *			data will be stored
		 * @return	a Result object with the result of the parse operation */
		Result parsePrimitive(
			const nlohmann::json& jsonPrimitive, graphics::RenderableMesh& out
		);

		/** Creates new MeshComponents from the given GLTF JSON Mesh and appends
		 * them to mGLTFData.
		 *
		 * @param	jsonMesh the JSON object with the Mesh to parse
		 * @return	a Result object with the result of the parse operation
		 * @note	Morph targets nor Meshes without indices arent supported
		 *			yet */
		Result parseMesh(const nlohmann::json& jsonMesh);

		/** Creates a new Skin from the given GLTF JSON Skin and appends
		 * it to the mGLTFData
		 *
		 * @param	jsonSkin the JSON object with the Skin to parse
		 * @return	a Result object with the result of the parse operation */
		Result parseSkin(const nlohmann::json& jsonSkin);

		/** Creates a new Camera from the given GLTF JSON Camera and appends
		 * it to mGLTFData
		 *
		 * @param	jsonCamera the JSON object with the Camera to parse
		 * @return	a Result object with the result of the parse operation */
		Result parseCamera(const nlohmann::json& jsonCamera);

		/** Creates a new IAnimation from the given GLTF JSON Animation Sampler
		 * and returns it in one of the given output parameters
		 *
		 * @param	jsonSampler the JSON object with the Animation sampler to
		 *			parse
		 * @param	out1 the pointer used for returning Vec3 IAnimations
		 * @param	out2 the pointer used for returning Quat IAnimations
		 * @return	a Result object with the result of the parse operation
		 * @note	AnimationNodes must have been already loaded */
		Result parseAnimationSampler(
			const nlohmann::json& jsonSampler,
			std::unique_ptr<Vec3Animation>& out1,
			std::unique_ptr<QuatAnimation>& out2
		) const;

		/** Creates a new TransformationAnimation from the given GLTF JSON
		 * Animation Channel and returns it in the given output parameters
		 *
		 * @param	jsonChannel the JSON object with the Animation Channel to
		 *			parse
		 * @param	vec3Animations a map with the Vec3 IAnimations
		 * @param	quatAnimations a map with the Quat IAnimations
		 * @param	out the pointer used for returning the
		 *			TransformationAnimation
		 * @return	a Result object with the result of the parse operation */
		Result parseAnimationChannel(
			const nlohmann::json& jsonChannel,
			const std::map<std::size_t, Vec3AnimationSPtr>& vec3Animations,
			const std::map<std::size_t, QuatAnimationSPtr>& quatAnimations,
			IAnimatorUPtr& out
		) const;

		/** Creates a new Animation from the given GLTF JSON Animation and
		 * appends it to mGLTFData
		 *
		 * @param	jsonAnimation the JSON object with the Animation to parse
		 * @return	a Result object with the result of the parse operation
		 * @note	AnimationNodes must have been already loaded */
		Result parseAnimation(const nlohmann::json& jsonAnimation);

		/** Used to parse the KHR_lights_punctual extension
		 *
		 * @param	jsonKHRLights the JSON object with the KHR_lights_punctual
		 *			object to parse
		 * @return	a Result object with the result of the parse operation */
		Result parseKHRLights(const nlohmann::json& jsonKHRLights);

		/** Creates a new LightSource from the given GLTF JSON Light and appends
		 * it to the mGLTFData
		 *
		 * @param	jsonLight the JSON object with the LightSource to parse
		 * @return	a Result object with the result of the parse operation */
		Result parseLight(const nlohmann::json& jsonLight);

		/** Creates a new Node from the given GLTF JSON Node and appends it to
		 * mGLTFData
		 *
		 * @param	jsonNode the JSON object with the Node to parse
		 * @return	a Result object with the result of the parse operation */
		Result parseNode(const nlohmann::json& jsonNode);

		/** Creates a new Scene from the given GLTF JSON Scene and appends it to
		 * mGLTFData
		 *
		 * @param	jsonScene the JSON object with the Scene to parse
		 * @return	a Result object with the result of the parse operation */
		Result parseScene(const nlohmann::json& jsonScene);
	};

}

#endif		// GLTF_READER_H
