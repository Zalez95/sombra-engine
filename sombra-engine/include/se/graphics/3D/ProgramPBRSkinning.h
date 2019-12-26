#ifndef PROGRAM_PBR_SKINNING_H
#define PROGRAM_PBR_SKINNING_H

#include "ProgramPBR.h"

namespace se::graphics {

	/**
	 * Interface ProgramPBRSkinning, it's a high level Program used by the
	 * Renderer so it doesn't need to search and set the uniform variables
	 */
	class ProgramPBRSkinning : public ProgramPBR
	{
	protected:	// Attributes
		/** The maximum number of joints in the program */
		static constexpr int kMaxJoints = 64;

	public:		// Functions
		/** Class destructor */
		virtual ~ProgramPBRSkinning() = default;

		/** Sets the uniform variables for the given joint matrices
		 *
		 * @param	jointMatrices a vector with all the joint matrices */
		void setJointMatrices(const std::vector<glm::mat4>& jointMatrices);
	protected:
		/** Creates the Shaders and the Program that the current class will use
		 * for setting the uniform variables
		 *
		 * @return	true if the shaders were loaded successfully, false
		 *			otherwise */
		virtual bool createProgram() override;

		/** Adds the uniform variables to the program
		 *
		 * @return	true if the uniform variables were found, false otherwise */
		virtual bool addUniforms() override;
	};

}

#endif		// PROGRAM_PBR_SKINNING_H
