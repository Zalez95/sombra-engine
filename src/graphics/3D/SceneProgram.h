#ifndef SCENE_PROGRAM
#define SCENE_PROGRAM

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

namespace graphics {

	class Program;
	class Material;
	class PointLight;


	/** SceneProgram class, it's a high level Program used by the
	 * SceneRenderer so it doesn't need to search and set the uniform
	 * variables */
	class SceneProgram
	{
	private:	// Nested types
		/** The maximum number of point lights in the program */
		static const unsigned int MAX_POINT_LIGHTS = 4;

		/** Struct UniformLocations, it holds the uniform variables location
		 * so we don't have to get them in each render call */
		struct UniformLocations
		{
			GLuint mModelViewMatrix;
			GLuint mProjectionMatrix;

			struct
			{
				GLuint mAmbientColor;
				GLuint mDiffuseColor;
				GLuint mSpecularColor;
				GLuint mShininess;
			} mMaterial;

			struct BaseLight
			{
				GLuint mAmbientIntensity;
				GLuint mIntensity;
			};

			struct Attenuation
			{
				GLuint mConstant;
				GLuint mLinear;
				GLuint mExponential;
			};

			GLuint mNumPointLights;
			struct
			{
				BaseLight mBaseLight;
				Attenuation mAttenuation;
			} mPointLights[MAX_POINT_LIGHTS];
			GLuint mPointLightsPositions[MAX_POINT_LIGHTS];
		};

	private:	// Attributes
		/** The Program of the renderer */
		Program* mProgram;

		/** The locations of uniform variables in the shader */
		UniformLocations mUniformLocations;

	public:		// Functions
		/** Creates a new SceneProgram */
		SceneProgram();

		/** Class destructor */
		~SceneProgram();

		/** Uses the current shader object so they can be used as part
		 * of the current rendering state */
		void enable() const;

		/** Resets the current shader object */
		void disable() const;

		/** Sets the uniform variables fot the given Projection matrix
		 *
		 * @param	projectionMatrix the matrix that we want to set as the
		 *			Projection matrix in the shaders */
		void setProjectionMatrix(const glm::mat4& projectionMatrix);

		/** Sets the uniform variables fot the given ModelView matrix
		 * 
		 * @param	modelViewMatrix the matrix that we want to set as the
		 *			ModelView matrix in the shaders */
		void setModelViewMatrix(const glm::mat4& modelViewMatrix);

		/** Sets the uniform variables for the given material
		 * 
		 * @param	material a pointer to the material with the data that we
		 *			want to set as uniform variables in the shaders */
		void setMaterial(const Material* material);

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

#endif		// SCENE_PROGRAM
