#ifndef PROGRAM_3D_H
#define PROGRAM_3D_H

#include <vector>
#include <glm/glm.hpp>

namespace se::graphics {

	class Program;
	struct Material;
	struct PointLight;


	/**
	 * Program3D class, it's a high level Program used by the
	 * SceneRenderer so it doesn't need to search and set the uniform
	 * variables
	 */
	class Program3D
	{
	private:	// Nested types
		struct TextureUnits {
			static constexpr int kBaseColor			= 0;
			static constexpr int kMetallicRoughness	= 1;
			static constexpr int kNormal			= 2;
			static constexpr int kOcclusion			= 3;
			static constexpr int kEmissive			= 4;
		};

	private:	// Attributes
		/** The maximum number of point lights in the program */
		static constexpr int kMaxPointLights = 4;

		/** The Program of the renderer */
		Program* mProgram;

		/** Holds the uniform variables location so we don't have to get them
		 * in each render call */
		struct UniformLocations
		{
			int modelMatrix;
			int viewMatrix;
			int projectionMatrix;

			struct
			{
				struct
				{
					int baseColorFactor;
					int useBaseColorTexture;
					int baseColorTexture;
					int metallicFactor;
					int roughnessFactor;
					int useMetallicRoughnessTexture;
					int metallicRoughnessTexture;
				} pbrMetallicRoughness;

				int useNormalTexture;
				int normalTexture;
				int occlusionTexture;
				int emissiveTexture;
				int emissiveFactor;
				int checkAlphaCutoff;
				int alphaCutoff;
			} material;

			int numPointLights;
			struct
			{
				struct
				{
					int lightColor;
				} baseLight;

				struct
				{
					int constant;
					int linear;
					int exponential;
				} attenuation;
			} pointLights[kMaxPointLights];
			int pointLightsPositions[kMaxPointLights];
		} mUniformLocations;

	public:		// Functions
		/** Creates a new Program3D */
		Program3D();

		/** Class destructor */
		~Program3D();

		/** Uses the current shader object so they can be used as part
		 * of the current rendering state */
		void enable() const;

		/** Resets the current shader object */
		void disable() const;

		/** Sets the uniform variables fot the given Model matrix
		 *
		 * @param	modelMatrix the matrix that we want to set as the
		 *			Model matrix in the shaders */
		void setModelMatrix(const glm::mat4& modelMatrix) const;

		/** Sets the uniform variables for the given View matrix
		 *
		 * @param	viewMatrix the matrix that we want to set as the
		 *			View matrix in the shaders */
		void setViewMatrix(const glm::mat4& viewMatrix) const;

		/** Sets the uniform variables for the given Projection matrix
		 *
		 * @param	projectionMatrix the matrix that we want to set as the
		 *			Projection matrix in the shaders */
		void setProjectionMatrix(const glm::mat4& projectionMatrix) const;

		/** Sets the uniform variables for the given material
		 *
		 * @param	material the material with the data that we want to set as
		 *			uniform variables in the shaders */
		void setMaterial(const Material& material) const;

		/** Sets the uniform variables for the given PointLights
		 *
		 * @param	pointLights a vector of pointer to the PointLights with the
		 *			data that we want to set as uniform variables in the
		 *			shaders
		 * @note	the maximum number of PointLights is MAX_LIGHTS, so if
		 *			there are more lights in the given vector only the first
		 *			lights of the vector will be submited */
		void setLights(const std::vector<const PointLight*>& pointLights) const;
	private:
		/** Creates the Shaders and the Program that the current class will use
		 * for setting the uniform variables */
		void initShaders();

		/** Gets the location of all the uniform variables and stores them in
		 * mUniformLocations */
		void initUniformLocations();
	};

}

#endif		// PROGRAM_3D_H
