#ifndef COLLIDER_H
#define COLLIDER_H

#include <memory>
#include <bitset>
#include <glm/glm.hpp>
#include "AABB.h"

namespace se::physics {

	class RigidBody;


	/**
	 * Class Collider, a Collider is used to store the basic data of an object
	 * that can collide with other Colliders
	 */
	class Collider
	{
	public:		// Attributes
		/** The maximum number of layers of a Collider */
		static constexpr std::size_t kMaxLayers = 32;
	protected:
		/** The parent RigidBody of the Collider */
		RigidBody* mParent = nullptr;

		/** A bitmap with the layers of the Collider Only the Colliders that
		 * are in the same layer can collide, by default all the Colliders are
		 * in the zero layer */
		std::bitset<kMaxLayers> mLayers = { 0x1 };

		/** If the Collider has been updated or not */
		bool mUpdated = true;

	public:		// Functions
		/** Creates a new Collider */
		Collider() = default;
		Collider(const Collider& other) = default;
		Collider(Collider&& other) = default;

		/** Class destructor */
		virtual ~Collider() = default;

		/** Assignment operator */
		Collider& operator=(const Collider& other) = default;
		Collider& operator=(Collider&& other) = default;

		/** @return	a pointer to a copy of the current Collider */
		virtual std::unique_ptr<Collider> clone() const = 0;

		/** Sets the parent RigidBody of the Collider
		 *
		 * @param	parent the new parent RigidBody of the Collider */
		void setParent(RigidBody* parent)
		{ mParent = parent; mUpdated = true; };

		/** @return	the parent RigidBody of the Collider, nullptr if it doesn't
		 *			have one */
		RigidBody* getParent() const { return mParent; };

		/** Sets the value of a layer of the Collider
		 *
		 * @param	layerIdx the index of the layer to set, it must be a value
		 *			in the range [0, 31]
		 * @param	value the new value of the layer */
		void setLayer(std::size_t layerIdx, bool value)
		{ mLayers[layerIdx] = value; mUpdated = true; };

		/** Sets the layers of the Collider
		 *
		 * @param	layers the new layers of the Collider */
		void setLayers(const std::bitset<kMaxLayers>& layers)
		{ mLayers = layers; mUpdated = true; };

		/** @return	the layers of the Collider */
		std::bitset<kMaxLayers> getLayers() const { return mLayers; };

		/** Updates the scale, translation and orientation of the Collider
		 * with the data of the given transformations matrix
		 *
		 * @param	transforms the transformations matrix used to update the
		 *			scale, position and orientation of the Collider */
		virtual void setTransforms(const glm::mat4& transforms) = 0;

		/** @return	the transformations matrix currently applied to the
		 *			Collider */
		virtual glm::mat4 getTransforms() const = 0;

		/** @return the Axis Aligned Bounding Box that contains the
		 *			Collider */
		virtual AABB getAABB() const = 0;

		/** @return	true if the Collider has been updated since the last call
		 *			to the resetUpdatedState function, false otherwise */
		virtual bool updated() const { return mUpdated; };

		/** Resets the updated state of the Collider */
		virtual void resetUpdatedState() { mUpdated = false; };
	};

}

#endif		// COLLIDER_H
