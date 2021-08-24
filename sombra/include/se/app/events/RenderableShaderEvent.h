#ifndef RENDERABLE_SHADER_EVENT_H
#define RENDERABLE_SHADER_EVENT_H

#include "../Entity.h"
#include "../graphics/RenderableShader.h"
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

		/** The different Renderable components where a RenderableShader
		 * can be attached */
		enum class RComponentType { Mesh, Terrain, ParticleSystem, Light };

	private:	// Attributes
		/** The Operation of the RenderableShaderEvent */
		Operation mOperation;

		/** The Entity that owns the Components updated */
		Entity mEntity;

		/** The type of the Renderable component to update */
		RComponentType mRComponentType;

		/** The index of the RenderableMesh updated if @see mRenderableType is
		 * Mesh */
		std::size_t mRIndex;

		/** The RenderableShader used in the Operation */
		RenderableShaderSPtr mShader;

	public:		// Functions
		/** Creates a new RenderableShaderEvent used for notifying of a
		 * RenderableShader add/remove Operation on a RenderableTerrain
		 *
		 * @param	operation the Operation to perform
		 * @param	entity the Entity that owns the MeshComponent updated
		 * @param	rComponentType the type of the Renderable component to
		 *			update
		 * @param	shader a pointer to the RenderableShader used in the
		 *			Operation */
		RenderableShaderEvent(
			Operation operation, Entity entity, RComponentType rComponentType,
			RenderableShaderSPtr shader
		) : mOperation(operation), mEntity(entity),
			mRComponentType(rComponentType), mRIndex(0), mShader(shader) {};

		/** Creates a new RenderableShaderEvent used for notifying of a
		 * RenderableShader add/remove Operation on a MeshComponent
		 *
		 * @param	operation the Operation to perform
		 * @param	entity the Entity that owns the MeshComponent updated
		 * @param	rIndex the index of the RenderableMesh updated
		 * @param	shader a pointer to the RenderableShader used in the
		 *			Operation */
		RenderableShaderEvent(
			Operation operation, Entity entity, std::size_t rIndex,
			RenderableShaderSPtr shader
		) : mOperation(operation), mEntity(entity),
			mRComponentType(RComponentType::Mesh), mRIndex(rIndex),
			mShader(shader) {};

		/** @return	the Operation to perform */
		Operation getOperation() const { return mOperation; };

		/** @return	the Entity to notify */
		Entity getEntity() const { return mEntity; };

		/** @return	the type of the Renderable Component to update */
		RComponentType getRComponentType() const { return mRComponentType; };

		/** @return	the index of the RenderableMesh updated */
		std::size_t getRIndex() const { return mRIndex; };

		/** @return	a pointer to the RenderableShader used in the Operaion */
		RenderableShaderSPtr getShader() const { return mShader; };
	private:
		/** Prints the operation to the given stream
		 *
		 * @param	os the stream to print to
		 * @param	o the operation to print
		 * @return	the stream */
		friend constexpr std::ostream& operator<<(
			std::ostream& os, const Operation& o
		) {
			switch (o) {
				case Operation::Add:	return os << "Operation::Add";
				case Operation::Remove:	return os << "Operation::Remove";
				default:				return os;
			}
		};

		/** Prints the RComponentType to the given stream
		 *
		 * @param	os the stream to print to
		 * @param	r the RComponentType to print
		 * @return	the stream */
		friend constexpr std::ostream& operator<<(
			std::ostream& os, const RComponentType& r
		) {
			switch (r) {
				case RComponentType::Mesh:
					return os << "RComponentType::Mesh";
				case RComponentType::Terrain:
					return os << "RComponentType::Terrain";
				case RComponentType::ParticleSystem:
					return os << "RComponentType::ParticleSystem";
				case RComponentType::Light:
					return os << "RComponentType::Light";
				default:
					return os;
			}
		};

		/** @copydoc Event::printTo() */
		virtual void printTo(std::ostream& os) const override
		{
			os	<< "{ kTopic : " << kTopic
				<< ", mOperation : " << mOperation
				<< ", mEntity : " << mEntity
				<< ", mRComponentType : " << mRComponentType
				<< ", mRIndex : " << mRIndex
				<< ", mShader : " << mShader.get() << " }";
		};
	};

}

#endif		// RENDERABLE_SHADER_EVENT_H
