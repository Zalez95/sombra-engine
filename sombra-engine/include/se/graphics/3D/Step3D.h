#ifndef STEP_3D_H
#define STEP_3D_H

#include "../Technique.h"

namespace se::graphics {

	class Renderer3D;


	/**
	 * Class Step3D, TODO:
	 */
	class Step3D : public Step
	{
	private:	// Attributes
		/** The Renderer3D where the Renderable2D will be submitted */
		Renderer3D& mRenderer;

	public:
		/** Creates a new Step3D
		 *
		 * @param	renderer the Renderer3D of the Step3D */
		Step3D(Renderer3D& renderer) : mRenderer(renderer) {};

		/** Submits the given Renderable for rendering with the current Step3D
		 *
		 * @param	renderable the Renderable to submit. It must be a
		 *			Renderable2D */
		void submit(Renderable& renderable) override;
	};

}

#endif		// STEP_3D_H
