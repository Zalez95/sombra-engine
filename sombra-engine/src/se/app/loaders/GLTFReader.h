#ifndef GLTF_READER_H
#define GLTF_READER_H

#include <string>
#include <nlohmann/json_fwd.hpp>
#include "se/app/loaders/SceneReader.h"
#include "se/app/graphics/Image.h"
#include "se/animation/IAnimation.h"

namespace se::app {

	/**
	 * Class GLTFReader, it's used to create meshes from the given texts
	 */
	class GLTFReader : public SceneReader
	{
	private:	// Nested types
		using MaterialUPtr = std::unique_ptr<Material>;
		using TextureSPtr = std::shared_ptr<graphics::Texture>;
		using MeshUPtr = std::unique_ptr<graphics::Mesh>;
		using Primitives = std::vector<Scenes::Primitive>;
		using LightSourceUPtr = std::unique_ptr<LightSource>;
		using SkinUPtr = std::unique_ptr<Skin>;
		using CameraUPtr = std::unique_ptr<Camera>;
		using SceneUPtr = std::unique_ptr<Scene>;
		using Vec3Animation = animation::IAnimation<glm::vec3>;
		using QuatAnimation = animation::IAnimation<glm::quat>;
		using Vec3AnimationSPtr = std::shared_ptr<Vec3Animation>;
		using QuatAnimationSPtr = std::shared_ptr<QuatAnimation>;
		using IAnimatorUPtr = std::unique_ptr<animation::IAnimator>;
		using CAnimatorUPtr = std::unique_ptr<animation::CompositeAnimator>;
		using Buffer = std::vector<std::byte>;

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

		/** Struct Node, holds the data of a Node in the GLTF Node hierarchy */
		struct Node
		{
			Scene::Entity sceneEntity;
			std::vector<std::size_t> children;
			animation::NodeData nodeData;
		};

		/** Struct GLTFData, it holds validated GLTF data */
		struct GLTFData
		{
			std::vector<Accessor> accessors;
			std::vector<Buffer> buffers;
			std::vector<BufferView> bufferViews;
			std::vector<Sampler> samplers;
			std::vector<Image> images;
			std::vector<TextureSPtr> textures;
			std::vector<MaterialUPtr> materials;
			std::vector<MeshUPtr> meshes;
			std::vector<Primitives> primitives;
			std::vector<LightSourceUPtr> lightSources;
			std::vector<SkinUPtr> skins;
			std::vector<CameraUPtr> cameras;
			std::vector<Node> nodes;
			std::vector<SceneUPtr> scenes;
			std::vector<CAnimatorUPtr> compositeAnimators;
		};

	private:	// Attributes
		/** The base path of the file to parse */
		std::string mBasePath;

		/** The temporarily read GLTF data */
		GLTFData mGLTFData;

	public:		// Functions
		/** Creates a new GLTFReader */
		GLTFReader();

		/** Parses the given GLTF file and stores the result in the given
		 * Scenes object
		 *
		 * @param	path the path to the GLTF file to parse
		 * @param	output the Scenes object where the file data will be stored
		 * @return	a Result object with the result of the load operation */
		Result load(const std::string& path, Scenes& output) override;
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
		 * @param	output the Scenes object where the loaded data will be
		 *			stored
		 * @return	a Result object with the result of the parse operation */
		Result parseGLTF(const nlohmann::json& jsonGLTF, Scenes& output);

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

		/** Creates a new Material from the given GLTF JSON Material and appends
		 * it to mGLTFData
		 *
		 * @param	jsonMaterial the JSON object with the Material to parse
		 * @return	a Result object with the result of the parse operation */
		Result parseMaterial(const nlohmann::json& jsonMaterial);

		/** Creates a new Primitive from the given GLTF JSON primitive and
		 * appends it to mGLTFData
		 *
		 * @param	jsonPrimitive the JSON object with the Primitive to parse
		 * @param	out a reference to the pair where the indices of the new
		 *			mesh and the material of the Primitive will be stored
		 * @return	a Result object with the result of the parse operation */
		Result parsePrimitive(
			const nlohmann::json& jsonPrimitive, Scenes::Primitive& out
		);

		/** Creates new Primitives from the given GLTF JSON Mesh and appends
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

		/** Creates a new IAnimation from the given GLTF JSON Animation Sampler
		 * and returns it in one of the given output parameters
		 *
		 * @param	jsonSampler the JSON object with the Animation sampler to
		 *			parse
		 * @param	out1 the pointer used for returning Vec3 IAnimations
		 * @param	out2 the pointer used for returning Quat IAnimations
		 * @return	a Result object with the result of the parse operation */
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
		 * @return	a Result object with the result of the parse operation */
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
	};

}

#endif		// GLTF_READER_H
