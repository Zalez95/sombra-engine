#ifndef RENDERABLE_SHADER_EVENT_H
#define RENDERABLE_SHADER_EVENT_H

#include "../Entity.h"
#include "../RenderableShader.h"
#include "Event.h"

namespace se::app {

	/**
	 * Class RenderableShaderEvent, its an event used for notify of a
	 * RenderableShader change on a Renderable Component
	 */
	class RenderableShaderEvent : public Event<Topic::RShader>
	{
	public:		// Nested types
		using RenderableShaderSPtr = std::shared_ptr<RenderableShader>;

		/** The different operations to perform with the
		 * RenderableShaderEvent */
		enum class Operation { Add, Remove };

	private:	// Attributes
		/** The Operation of the RenderableShaderEvent */
		Operation mOperation;

		/** The Entity that owns the Components updated */
		Entity mEntity;

		/** If the Component updated is a RenderableTerrain or not
		 * (MeshComponent) */
		bool mIsTerrain;

		/** The index of the RenderableMesh updated if @see mIsTerrain is
		 * false */
		std::size_t mRIndex;

		/** The RenderableShader used in the Operation */
		RenderableShaderSPtr mShader;

	public:		// Functions
		/** Creates a new RenderableShaderEvent used for notifying of a
		 * RenderableShader add/remove Operation on a RenderableTerrain
		 *
		 * @param	operation the Operation to perform
		 * @param	entity the Entity that owns the MeshComponent updated
		 * @param	shader a pointer to the RenderableShader used in the
		 *			Operation */
		RenderableShaderEvent(
			Operation operation, Entity entity, RenderableShaderSPtr shader
		) : mOperation(operation), mEntity(entity),
			mIsTerrain(true), mRIndex(0), mShader(shader) {};

		/** Creates a new RenderableShaderEvent used for notifying of a
		 * RenderableShader add/remove Operation on a MeshComponent
		 *
		 * @param	operation the Operation to perform
		 * @param	entity the Entity that owns the MeshComponent updated
		 * @param	rIndex the index of the RenderableMesh updated */
		RenderableShaderEvent(
			Operation operation, Entity entity, std::size_t rIndex,
			RenderableShaderSPtr shader
		) : mOperation(operation), mEntity(entity),
			mIsTerrain(false), mRIndex(rIndex), mShader(shader) {};

		/** @return	the Operation to perform */
		Operation getOperation() const { return mOperation; };

		/** @return	the Entity to notify */
		Entity getEntity() const { return mEntity; };

		/** @return	true if the Component affected by the RenderableShaderEvent
		 *			is a Terrain, false otherwise */
		bool isTerrain() const { return mIsTerrain; };

		/** @return	the index of the RenderableMesh updated */
		std::size_t getRIndex() const { return mRIndex; };

		/** @return	a pointer to the RenderableShader used in the Operaion */
		RenderableShaderSPtr getShader() const { return mShader; };
	};

}

#endif		// RENDERABLE_SHADER_EVENT_H
