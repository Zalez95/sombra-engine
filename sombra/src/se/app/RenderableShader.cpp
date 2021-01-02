#include "se/app/RenderableShader.h"
#include "se/app/events/ShaderEvent.h"

namespace se::app {

	RenderableShader& RenderableShader::addPass(const PassSPtr& pass)
	{
		mTechnique->addPass(pass);
		mEventManager.publish(new ShaderEvent(ShaderEvent::Operation::Add, shared_from_this(), pass));
		return *this;
	}


	RenderableShader& RenderableShader::removePass(const PassSPtr& pass)
	{
		mTechnique->removePass(pass);
		mEventManager.publish(new ShaderEvent(ShaderEvent::Operation::Remove, shared_from_this(), pass));
		return *this;
	}

}
