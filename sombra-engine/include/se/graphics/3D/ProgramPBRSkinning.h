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
		virtual ~ProgramPBRSkinning() {};

		/** Function called for initializing all the needed resources
		 *
		 * @return	true on success, false otherwise */
		virtual bool init() override;

		/** Sets the uniform variables for the given joint matrices
		 *
		 * @param	jointMatrices a vector with all the joint matrices */
		void setJointMatrices(
			const std::vector<glm::mat4>& jointMatrices
		) const;
	protected:
		/** Adds the uniform variables to the program */
		virtual bool addUniforms() override;
	};

}

#endif		// PROGRAM_PBR_SKINNING_H
