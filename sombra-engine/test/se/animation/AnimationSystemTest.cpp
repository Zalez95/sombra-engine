#include <gtest/gtest.h>
#include <se/animation/Scene.h>
#include <se/animation/Animators.h>
#include <se/animation/LinearAnimations.h>
#include <se/animation/AnimationSystem.h>

using namespace se::animation;
static constexpr float kTolerance = 0.000001f;

TEST(AnimationSystem, update1)
{
	std::vector<SceneNode> originalNodes = {
		{{ "b0", { 4.445192337f, -4.281722545f, 2.230783700f }, { 0.634908735f, 0.734051764f, 0.169194266f, -0.171558305f }, { -0.616233110f, 0.166248172f, -0.367524087f } }},
		{{ "b1", { 0.905142105f, 1.988715789f, 0.0f }, { 0.860006511f, -0.472376585f, 0.024241564f, 0.191472634f }, { 0.332452088f, -0.111552506f, 1.410963535f } }},
		{{ "b2", { 0.841695f, 0.296882f, -0.450525f }, { 0.074382677f, -0.287267595f, 0.806229293f, 0.511799693f }, { 0.332452088f, -0.111552506f, 1.410963535f } }},
		{{ "b3", { 0.25f, 1.0f, 2.75f }, { 0.525976598f, -0.087920218f, 0.640181660f, 0.552979230f }, { -0.369545638f, 0.504002809f, -0.842678189f } }}
	};
	const std::vector<SceneNode> expectedNodes = originalNodes;

	Vec3Animator animator1(std::make_shared<AnimationVec3Linear>());
	for (SceneNode& node : originalNodes) {
		animator1.addNode(IAnimator::TransformationType::Translation, &node);
	}
	QuatAnimator animator2(std::make_shared<AnimationQuatLinear>());
	for (SceneNode& node : originalNodes) {
		animator2.addNode(IAnimator::TransformationType::Rotation, &node);
	}
	Vec3Animator animator3(std::make_shared<AnimationVec3Linear>());
	for (SceneNode& node : originalNodes) {
		animator3.addNode(IAnimator::TransformationType::Scale, &node);
	}

	AnimationSystem animationSystem;
	animationSystem.addAnimator(&animator1);
	animationSystem.addAnimator(&animator2);
	animationSystem.addAnimator(&animator3);
	animationSystem.update(3.2f);

	for (std::size_t i = 0; i < originalNodes.size(); ++i) {
		EXPECT_EQ(originalNodes[i].getData().name, expectedNodes[i].getData().name);
		for (int j = 0; j < 3; ++j) {
			EXPECT_NEAR(originalNodes[i].getData().position[j], expectedNodes[i].getData().position[j], kTolerance);
		}
		for (int j = 0; j < 4; ++j) {
			EXPECT_NEAR(originalNodes[i].getData().orientation[j], expectedNodes[i].getData().orientation[j], kTolerance);
		}
		for (int j = 0; j < 3; ++j) {
			EXPECT_NEAR(originalNodes[i].getData().scale[j], expectedNodes[i].getData().scale[j], kTolerance);
		}
	}
}


TEST(AnimationSystem, update2)
{
	const std::vector<SceneNode> expectedNodes = {
		{{ "b0", {-3.798496723f, 8.799341201f, 7.647266387f }, { 0.332961529f, 0.848409652f,-0.193423464f,-0.363215088f }, { 5.456992626f, 22.498020172f, 18.062149047f } }},
		{{ "b1", { 0.332452088f,-0.111552506f, 1.410963535f }, { 0.860006511f,-0.472376585f, 0.024241564f, 0.191472634f }, { 0.905142105f, 1.988715789f, 0.0f } }},
		{{ "b2", { 0.884948372f,-0.937227845f, 1.525016903f }, {-0.850997865f,-0.043472886f,-0.517922818f,-0.075289249f }, { 0.610229551f, 0.198523178f,-0.074333041f } }},
		{{ "b3", {-0.430792808f, 0.950675666f,-0.635973989f }, { 0.098620802f,-0.588778734f, 0.738621354f,-0.313132643f }, {-0.033172354f, 0.462077677f,-0.595307648f } }},
	};
	std::vector<SceneNode> originalNodes = {
		{{ "b0", {-0.616233110f, 0.166248172f,-0.367524087f }, { 0.634908735f, 0.734051764f, 0.169194266f,-0.171558305f }, { 4.445192337f,-4.281722545f, 2.230783700f } }},
		{{ "b1", { 0.332452088f,-0.111552506f, 1.410963535f }, { 0.860006511f,-0.472376585f, 0.024241564f, 0.191472634f }, { 0.905142105f, 1.988715789f, 0.0f } }},
		{{ "b2", { 0.332452088f,-0.111552506f, 1.410963535f }, { 0.074382677f,-0.287267595f, 0.806229293f, 0.511799693f }, { 0.841695f, 0.296882f,-0.450525f } }},
		{{ "b3", {-0.369545638f, 0.504002809f,-0.842678189f }, { 0.525976598f,-0.087920218f, 0.640181660f, 0.552979230f }, { 0.25f, 1.0f, 2.75f } }}
	};

	AnimationSystem animationSystem;

	auto at1 = std::make_shared<AnimationVec3Linear>();
	auto at2 = std::make_shared<AnimationVec3Linear>();
	auto at3 = std::make_shared<AnimationVec3Linear>();
	auto at4 = std::make_shared<AnimationVec3Linear>();
	at1->addKeyFrame({ {-3.182263720f, 8.633092795f, 8.014790691f }, 0.650173135f });
	at2->addKeyFrame({ { 0.279904574f,-0.034162148f,-0.959419786f }, 2.993066462f });
	at3->addKeyFrame({ { 0.552496254f,-0.825675308f, 0.114053405f }, 2.060792734f });
	at4->addKeyFrame({ {-0.510563910f,-0.355170249f, 0.783057153f }, 2.211552098f });
	at4->addKeyFrame({ { 0.226587504f, 0.960337698f,-0.162510499f }, 3.833204925f });
	Vec3Animator atrT1(at1);
	Vec3Animator atrT2(at2);
	Vec3Animator atrT3(at3);
	Vec3Animator atrT4(at4);
	atrT1.addNode(IAnimator::TransformationType::Translation, &originalNodes[0]);
	atrT2.addNode(IAnimator::TransformationType::Translation, &originalNodes[1]);
	atrT3.addNode(IAnimator::TransformationType::Translation, &originalNodes[2]);
	atrT4.addNode(IAnimator::TransformationType::Translation, &originalNodes[3]);
	animationSystem.addAnimator(&atrT1);
	animationSystem.addAnimator(&atrT2);
	animationSystem.addAnimator(&atrT3);
	animationSystem.addAnimator(&atrT4);

	auto ar1 = std::make_shared<AnimationQuatLinear>();
	auto ar2 = std::make_shared<AnimationQuatLinear>();
	auto ar3 = std::make_shared<AnimationQuatLinear>();
	auto ar4 = std::make_shared<AnimationQuatLinear>();
	ar1->addKeyFrame({ { 0.863763153f, 0.388888984f,-0.300208389f, 0.112042762f }, 0.650173135f });
	ar2->addKeyFrame({ {-0.684891402f,-0.166260287f, 0.639585793f, 0.306938827f }, 2.993066462f });
	ar3->addKeyFrame({ {-0.506908714f,-0.452070087f, 0.691452503f, 0.246108666f }, 2.060792734f });
	ar4->addKeyFrame({ { 0.479462414f, 0.526573240f, 0.605637669f,-0.355020254f }, 2.211552098f });
	ar4->addKeyFrame({ { 0.334920555f, 0.151881396f, 0.693246245f,-0.619814455f }, 3.833204925f });
	QuatAnimator atrR1(ar1);
	QuatAnimator atrR2(ar2);
	QuatAnimator atrR3(ar3);
	QuatAnimator atrR4(ar4);
	atrR2.addNode(IAnimator::TransformationType::Rotation, &originalNodes[0]);
	atrR2.addNode(IAnimator::TransformationType::Rotation, &originalNodes[1]);
	atrR2.addNode(IAnimator::TransformationType::Rotation, &originalNodes[2]);
	atrR2.addNode(IAnimator::TransformationType::Rotation, &originalNodes[3]);
	animationSystem.addAnimator(&atrR1);
	animationSystem.addAnimator(&atrR2);
	animationSystem.addAnimator(&atrR3);
	animationSystem.addAnimator(&atrR4);

	auto as1 = std::make_shared<AnimationVec3Linear>();
	auto as2 = std::make_shared<AnimationVec3Linear>();
	auto as3 = std::make_shared<AnimationVec3Linear>();
	auto as4 = std::make_shared<AnimationVec3Linear>();
	as1->addKeyFrame({ { 1.227616805f,-5.254432319f, 8.096772882f }, 0.650173135f });
	as2->addKeyFrame({ { 0.681886255f, 0.491552144f, 0.541671037f }, 2.993066462f });
	as3->addKeyFrame({ { 0.725000798f, 0.668693900f, 0.164992049f }, 2.060792734f });
	as4->addKeyFrame({ {-0.626821994f,-0.344191372f,-0.699018180f }, 2.211552098f });
	as4->addKeyFrame({ { 0.183854460f, 0.978577852f, 0.092643812f }, 3.833204925f });
	Vec3Animator atrS1(as1);
	Vec3Animator atrS2(as2);
	Vec3Animator atrS3(as3);
	Vec3Animator atrS4(as4);
	atrS1.addNode(IAnimator::TransformationType::Scale, &originalNodes[0]);
	atrS2.addNode(IAnimator::TransformationType::Scale, &originalNodes[1]);
	atrS3.addNode(IAnimator::TransformationType::Scale, &originalNodes[2]);
	atrS4.addNode(IAnimator::TransformationType::Scale, &originalNodes[3]);
	animationSystem.addAnimator(&atrS1);
	animationSystem.addAnimator(&atrS2);
	animationSystem.addAnimator(&atrS3);
	animationSystem.addAnimator(&atrS4);

	animationSystem.update(3.2f);

	for (std::size_t i = 0; i < originalNodes.size(); ++i) {
		EXPECT_EQ(originalNodes[i].getData().name, expectedNodes[i].getData().name);
		for (int j = 0; j < 3; ++j) {
			EXPECT_NEAR(originalNodes[i].getData().position[j], expectedNodes[i].getData().position[j], kTolerance);
		}
		for (int j = 0; j < 4; ++j) {
			EXPECT_NEAR(originalNodes[i].getData().orientation[j], expectedNodes[i].getData().orientation[j], kTolerance);
		}
		for (int j = 0; j < 3; ++j) {
			EXPECT_NEAR(originalNodes[i].getData().scale[j], expectedNodes[i].getData().scale[j], kTolerance);
		}
	}
}
