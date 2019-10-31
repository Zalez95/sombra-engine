#include <gtest/gtest.h>
#include <se/animation/StepAnimations.h>

using namespace se::animation;
static constexpr float kTolerance = 0.000001f;

TEST(Animation, vec3StepInterpolation)
{
	const glm::vec3 expectedValues[] = {
		{ -2.717610597f, 2.315870285f, 1.709684491f },
		{ -2.717610597f, 2.315870285f, 1.709684491f },
		{ -2.717610597f, 2.315870285f, 1.709684491f },
		{ 4.445192337f, -4.281722545f, 2.230783700f },
		{ 4.445192337f, -4.281722545f, 2.230783700f }
	};

	AnimationVec3Step animation;
	animation.addKeyFrame({ { -2.717610597f, 2.315870285f, 1.709684491f }, 0.005f });
	animation.addKeyFrame({ { 4.445192337f, -4.281722545f, 2.230783700f }, 0.769285f });

	glm::vec3 results[5];
	results[0] = animation.interpolate(0.0f);
	results[1] = animation.interpolate(0.005f);
	results[2] = animation.interpolate(0.5552852f);
	results[3] = animation.interpolate(0.769285f);
	results[4] = animation.interpolate(1.0f);
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 3; ++j) {
			EXPECT_NEAR(results[i][j], expectedValues[i][j], kTolerance);
		}
	}
}


TEST(Animation, quatStepInterpolation)
{
	const glm::quat expectedValues[] = {
		{ 0.896704018f, -0.041954714f, 0.412458598f, 0.155047193f },
		{ 0.896704018f, -0.041954714f, 0.412458598f, 0.155047193f },
		{ 0.896704018f, -0.041954714f, 0.412458598f, 0.155047193f },
		{ 0.297078251f, 0.747193455f, 0.570527613f, 0.167167067f },
		{ 0.297078251f, 0.747193455f, 0.570527613f, 0.167167067f }
	};

	AnimationQuatStep animation;
	animation.addKeyFrame({ { 0.896704018f, -0.041954714f, 0.412458598f, 0.155047193f }, 0.005f });
	animation.addKeyFrame({ { 0.297078251f, 0.747193455f, 0.570527613f, 0.167167067f }, 0.769285f });

	glm::quat results[5];
	results[0] = animation.interpolate(0.0f);
	results[1] = animation.interpolate(0.005f);
	results[2] = animation.interpolate(0.5552852f);
	results[3] = animation.interpolate(0.769285f);
	results[4] = animation.interpolate(1.0f);
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 4; ++j) {
			EXPECT_NEAR(results[i][j], expectedValues[i][j], kTolerance);
		}
	}
}
