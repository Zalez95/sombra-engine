#include <gtest/gtest.h>
#include <se/animation/LinearAnimations.h>

using namespace se::animation;
static constexpr float kTolerance = 0.000001f;

TEST(Animation, vec3LinearInterpolation)
{
	const glm::vec3 expectedValue1(0.0f);
	const glm::vec3 expectedValue2{ -2.717610597f, 2.315870285f, 1.709684491f };
	const glm::vec3 expectedValue3{ 4.445192337f, -4.281722545f, 2.230783700f };
	const glm::vec3 expectedValue4{ 2.439607381f, -2.434396266f, 2.084875822f };

	AnimationVec3Linear animation;
	animation.addKeyFrame({ { -2.717610597f, 2.315870285f, 1.709684491f }, 0.005f });
	animation.addKeyFrame({ { 4.445192337f, -4.281722545f, 2.230783700f }, 0.769285f });

	glm::vec3 result1 = animation.interpolate(0.0f);
	glm::vec3 result2 = animation.interpolate(0.005f);
	glm::vec3 result3 = animation.interpolate(0.769285f);
	glm::vec3 result4 = animation.interpolate(0.5552852f);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(result1[i], expectedValue1[i], kTolerance);
		EXPECT_NEAR(result2[i], expectedValue2[i], kTolerance);
		EXPECT_NEAR(result3[i], expectedValue3[i], kTolerance);
		EXPECT_NEAR(result4[i], expectedValue4[i], kTolerance);
	}
}


TEST(Animation, quatLinearInterpolation)
{
	const glm::quat expectedValue1{ 1.0f, 0.0f, 0.0f, 0.0f };
	const glm::quat expectedValue2{ 0.896704018f, -0.041954714f, 0.412458598f, 0.155047193f };
	const glm::quat expectedValue3{ 0.297078251f, 0.747193455f, 0.570527613f, 0.167167067f };
	const glm::quat expectedValue4{ 0.534737288f, 0.577074706f, 0.589202642f, 0.184068545f };

	AnimationQuatLinear animation;
	animation.addKeyFrame({ { 0.896704018f, -0.041954714f, 0.412458598f, 0.155047193f }, 0.005f });
	animation.addKeyFrame({ { 0.297078251f, 0.747193455f, 0.570527613f, 0.167167067f }, 0.769285f });

	glm::quat result1 = animation.interpolate(0.0f);
	glm::quat result2 = animation.interpolate(0.005f);
	glm::quat result3 = animation.interpolate(0.769285f);
	glm::quat result4 = animation.interpolate(0.5552852f);
	for (int i = 0; i < 4; ++i) {
		EXPECT_NEAR(result1[i], expectedValue1[i], kTolerance);
		EXPECT_NEAR(result2[i], expectedValue2[i], kTolerance);
		EXPECT_NEAR(result3[i], expectedValue3[i], kTolerance);
		EXPECT_NEAR(result4[i], expectedValue4[i], kTolerance);
	}
}
