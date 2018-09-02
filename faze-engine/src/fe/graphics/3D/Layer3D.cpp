#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "fe/graphics/3D/Layer3D.h"

namespace fe { namespace graphics {

	Layer3D::Layer3D() :
		mRenderer3D(glm::perspective(kFOV, kWidth / float(kHeight) , kZNear, kZFar)) {}


	void Layer3D::addRenderable3D(const Renderable3D* renderable3D)
	{
		if (renderable3D) {
			mRenderable3Ds.push_back(renderable3D);
		}
	}


	void Layer3D::removeRenderable3D(const Renderable3D* renderable3D)
	{
		mRenderable3Ds.erase(
			std::remove(mRenderable3Ds.begin(), mRenderable3Ds.end(), renderable3D),
			mRenderable3Ds.end()
		);
	}


	void Layer3D::addPointLight(const PointLight* pointLight)
	{
		if (pointLight) {
			mPointLights.push_back(pointLight);
		}
	}


	void Layer3D::removePointLight(const PointLight* pointLight)
	{
		mPointLights.erase(
			std::remove(mPointLights.begin(), mPointLights.end(), pointLight),
			mPointLights.end()
		);
	}


	void Layer3D::render()
	{
		for (const Renderable3D* renderable3D : mRenderable3Ds)
			mRenderer3D.submit(renderable3D);
		mRenderer3D.render(mCamera, mPointLights);
	}

}}
