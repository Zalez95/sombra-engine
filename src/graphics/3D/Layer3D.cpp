#include "Layer3D.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace graphics {

// Static variables definition
	const float Layer3D::FOV = glm::radians(50.0f);
	const float Layer3D::Z_NEAR = 1.0f;
	const float Layer3D::Z_FAR = 100.0f;

// Public functions
	Layer3D::Layer3D() :
		mRenderer3D(glm::perspective(FOV, WIDTH / float(HEIGHT) , Z_NEAR, Z_FAR)) {}


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
		if (!mCamera) return;

		for (const Renderable3D* renderable3D : mRenderable3Ds)
			mRenderer3D.submit(renderable3D);
		mRenderer3D.render(*mCamera, mPointLights);
	}

}
