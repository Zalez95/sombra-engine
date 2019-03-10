#ifndef PROGRAM_3D_H
#define PROGRAM_3D_H

#include <vector>
#include <glm/glm.hpp>

namespace se::graphics {

	class Program;
	struct Material;
	class PointLight;


	/**
	 * Program3D class, it's a high level Program used by the
	 * SceneRenderer so it doesn't need to search and set the uniform
	 * variables
	 */
	class Program3D
	{
	private:	// Nested types
		struct TextureUnits {
			static constexpr unsigned int kBaseColor			= 0;
			static constexpr unsigned int kMetallicRoughness	= 1;
			static constexpr unsigned int kNormal				= 2;
			static constexpr unsigned int kOcclusion			= 3;
			static constexpr unsigned int kEmissive				= 4;
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
			unsigned int modelMatrix;
			unsigned int viewMatrix;
			unsigned int projectionMatrix;

			struct
			{
				struct
				{
					unsigned int baseColorFactor;
					unsigned int baseColorTexture;
					unsigned int metallicFactor;
					unsigned int roughnessFactor;
					unsigned int metallicRoughnessTexture;
				} pbrMetallicRoughness;

				unsigned int normalTexture;
				unsigned int occlusionTexture;
				unsigned int emissiveTexture;
				unsigned int emissiveFactor;
			} material;

			unsigned int numPointLights;
			struct
			{
				struct
				{
					unsigned int diffuseColor;
					unsigned int specularColor;
				} baseLight;

				struct
				{
					unsigned int constant;
					unsigned int linear;
					unsigned int exponential;
				} attenuation;
			} pointLights[kMaxPointLights];
			unsigned int pointLightsPositions[kMaxPointLights];
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
		void setModelMatrix(const glm::mat4& modelMatrix);

		/** Sets the uniform variables for the given View matrix
		 *
		 * @param	viewMatrix the matrix that we want to set as the
		 *			View matrix in the shaders */
		void setViewMatrix(const glm::mat4& viewMatrix);

		/** Sets the uniform variables for the given Projection matrix
		 *
		 * @param	projectionMatrix the matrix that we want to set as the
		 *			Projection matrix in the shaders */
		void setProjectionMatrix(const glm::mat4& projectionMatrix);

		/** Sets the uniform variables for the given material
		 *
		 * @param	material the material with the data that we want to set as
		 *			uniform variables in the shaders */
		void setMaterial(const Material& material);

		/** Sets the uniform variables for the given PointLights
		 *
		 * @param	pointLights a vector of pointer to the PointLights with the
		 *			data that we want to set as uniform variables in the
		 *			shaders
		 * @note	the maximum number of PointLights is MAX_LIGHTS, so if
		 *			there are more lights in the given vector only the first
		 *			lights of the vector will be submited */
		void setLights(const std::vector<const PointLight*>& pointLights);
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
