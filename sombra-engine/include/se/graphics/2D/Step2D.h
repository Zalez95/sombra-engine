#ifndef STEP_2D_H
#define STEP_2D_H

#include "../Technique.h"

namespace se::graphics {

	class Renderer2D;


	/**
	 * Class Step2D, TODO:
	 * @note	Every Step2D must have up to @see Renderer2D::kMaxTextures
	 *			UniformVariables added if you want to use Textures in your
	 *			Shaders
	 */
	class Step2D : public Step
	{
	private:	// Attributes
		/** The Renderer2D where the Renderable2D will be submitted */
		Renderer2D& mRenderer;

	public:		// Functions
		/** Creates a new Step2D
		 *
		 * @param	renderer the Renderer2D of the Step2D
		 * @note	the uniform won't be added as a bindable, for that it's
		 *			needed to use @see addBindable */
		Step2D(Renderer2D& renderer) : mRenderer(renderer) {};

		/** Submits the given Renderable for rendering with the current Step2D
		 *
		 * @param	renderable the Renderable to submit. It must be a
		 *			Renderable2D */
		void submit(Renderable& renderable) override;
	};

}

#endif		// STEP_2D_H
