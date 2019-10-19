#ifndef PROGRAM_3D_SKINNING_H
#define PROGRAM_3D_SKINNING_H

#include "Program3D.h"

namespace se::graphics {

	/**
	 * Program3DSkinning class, it's a high level Program used by the
	 * SceneRenderer so it doesn't need to search and set the uniform variables
	 */
	class Program3DSkinning : public Program3D
	{
	protected:	// Attributes
		/** The maximum number of joints in the program */
		static constexpr int kMaxJoints = 64;

		/** Holds the uniform variables location so we don't have to get them
		 * in each render call */
		struct UniformLocations
		{
			int jointMatrices;
		} mUniformLocationsSkinning;

	public:		// Functions
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
		/** Gets the location of all the uniform variables and stores them in
		 * mUniformLocations */
		virtual void initUniformLocations() override;
	};

}

#endif		// PROGRAM_3D_SKINNING_H
