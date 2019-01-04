#include <gtest/gtest.h>
#include <se/animation/Animation.h>

using namespace se::animation;
static constexpr float kTolerance = 0.000001f;

TEST(Animation, keyFrameLinearInterpolation1)
{
	const KeyFrame k1(
		{ -1.5878210911f, -4.5368907519f, 3.6477282017f },
		{ 0.896704018f, -0.041954714f, 0.412458598f, 0.155047193f },
		{ -2.717610597f, 2.315870285f, 1.709684491f },
		0.005f
	);
	const KeyFrame k2(
		{ -1.5878210911f, -4.5368907519f, 3.6477282017f },
		{ 0.297078251f, 0.747193455f, 0.570527613f, 0.167167067f },
		{ 4.445192337f, -4.281722545f, 2.230783700f },
		0.769285f
	);
	const KeyFrame expectedKeyFrame = k1;

	KeyFrame result = keyFrameLinearInterpolation(k1, k2, 0.0f);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(result.translation[i], expectedKeyFrame.translation[i], kTolerance);
	}
	for (int i = 0; i < 4; ++i) {
		EXPECT_NEAR(result.rotation[i], expectedKeyFrame.rotation[i], kTolerance);
	}
	EXPECT_EQ(result.timePoint, expectedKeyFrame.timePoint);
}


TEST(Animation, keyFrameLinearInterpolation2)
{
	const KeyFrame k1(
		{ -1.5878210911f, -4.5368907519f, 3.6477282017f },
		{ 0.896704018f, -0.041954714f, 0.412458598f, 0.155047193f },
		{ -2.717610597f, 2.315870285f, 1.709684491f },
		0.005f
	);
	const KeyFrame k2(
		{ 0.784301467f, -1.060801166f, 3.399628281f },
		{ 0.297078251f, 0.747193455f, 0.570527613f, 0.167167067f },
		{ 4.445192337f, -4.281722545f, 2.230783700f },
		0.769285f
	);
	const KeyFrame expectedKeyFrame = k2;

	KeyFrame result = keyFrameLinearInterpolation(k1, k2, 1.0f);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(result.translation[i], expectedKeyFrame.translation[i], kTolerance);
	}
	for (int i = 0; i < 4; ++i) {
		EXPECT_NEAR(result.rotation[i], expectedKeyFrame.rotation[i], kTolerance);
	}
	EXPECT_EQ(result.timePoint, expectedKeyFrame.timePoint);
}


TEST(Animation, keyFrameLinearInterpolation3)
{
	const KeyFrame expectedKeyFrame(
		{ -0.923626899f, -3.563585996f, 3.578260421f },
		{ 0.534737288f, 0.577074706f, 0.589202642f, 0.184068545f },
		{ 2.439607381f, -2.434396266f, 2.084875822f },
		0.5552852f
	);

	const KeyFrame k1(
		{ -1.5878210911f, -4.5368907519f, 3.6477282017f },
		{ 0.896704018f, -0.041954714f, 0.412458598f, 0.155047193f },
		{ -2.717610597f, 2.315870285f, 1.709684491f },
		0.005f
	);
	const KeyFrame k2(
		{ 0.784301467f, -1.060801166f, 3.399628281f },
		{ 0.297078251f, 0.747193455f, 0.570527613f, 0.167167067f },
		{ 4.445192337f, -4.281722545f, 2.230783700f },
		0.769285f
	);

	KeyFrame result = keyFrameLinearInterpolation(k1, k2, 0.72f);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(result.translation[i], expectedKeyFrame.translation[i], kTolerance);
	}
	for (int i = 0; i < 4; ++i) {
		EXPECT_NEAR(result.rotation[i], expectedKeyFrame.rotation[i], kTolerance);
	}
	EXPECT_EQ(result.timePoint, expectedKeyFrame.timePoint);
}
