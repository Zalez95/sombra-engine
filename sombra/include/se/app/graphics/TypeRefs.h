#ifndef TYPE_REFS_H
#define TYPE_REFS_H

#include "../../graphics/core/Program.h"
#include "../../graphics/core/Texture.h"
#include "../../graphics/core/UniformVariable.h"
#include "../../graphics/3D/Mesh.h"
#include "../../graphics/3D/Particles.h"
#include "../../graphics/Context.h"

namespace se::app {

	using ProgramRef = graphics::Context::TBindableRef<graphics::Program>;
	using TextureRef = graphics::Context::TBindableRef<graphics::Texture>;
	using MeshRef = graphics::Context::TBindableRef<graphics::Mesh>;
	using ParticlesRef = graphics::Context::TBindableRef<graphics::Particles>;
	template <typename T>
	using UniformVVRef = graphics::Context::TBindableRef<
		graphics::UniformVariableValue<T>
	>;
	template <typename T>
	using UniformVVVRef = graphics::Context::TBindableRef<
		graphics::UniformVariableValueVector<T>
	>;

}

#endif		// TYPE_REFS_H
