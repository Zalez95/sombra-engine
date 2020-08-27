#include <gtest/gtest.h>
#include <se/animation/TransformationAnimators.h>
#include <se/animation/LinearAnimations.h>

using namespace se::animation;
static constexpr float kTolerance = 0.000001f;

TEST(TransformationAnimator, loopTime1)
{
	auto at1 = std::make_shared<AnimationVec3Linear>();
	at1->addKeyFrame({ {-0.510563910f,-0.355170249f, 0.783057153f }, 2.211552098f });
	at1->addKeyFrame({ { 0.226587504f, 0.960337698f,-0.162510499f }, 3.833204925f });
	at1->addKeyFrame({ {-3.182263720f, 8.633092795f, 8.014790691f }, 0.650173135f });
	Vec3Animator animator(at1);

	EXPECT_NEAR(animator.getLoopTime(), 3.833204925f, kTolerance);

	animator.setLoopTime(3.6f);
	EXPECT_NEAR(animator.getLoopTime(), 3.6f, kTolerance);
}


TEST(TransformationAnimator, animate1)
{
	std::vector<AnimationNode> expectedNodes(3);
	expectedNodes[0].getData().localTransforms = { {-3.182263720f, 8.633092795f, 8.014790691f }, { 0.863763153f, 0.388888984f,-0.300208389f, 0.112042762f }, { 1.227616805f,-5.254432319f, 8.096772882f } };
	expectedNodes[1].getData().localTransforms = { { 0.279904574f,-0.034162148f,-0.959419786f }, {-0.684891402f,-0.166260287f, 0.639585793f, 0.306938827f }, { 0.681886255f, 0.491552144f, 0.541671037f } };
	expectedNodes[2].getData().localTransforms = { {-0.061247169f, 0.446672827f, 0.206704214f }, { 0.469563961f, 0.492710053f, 0.621901452f,-0.387278914f }, {-0.280316948f, 0.221195951f,-0.360640436f } };
	std::vector<AnimationNode> originalNodes(3);
	originalNodes[0].getData().localTransforms = { {-0.616233110f, 0.166248172f,-0.367524087f }, { 0.634908735f, 0.734051764f, 0.169194266f,-0.171558305f }, { 4.445192337f,-4.281722545f, 2.230783700f } };
	originalNodes[1].getData().localTransforms = { { 0.332452088f,-0.111552506f, 1.410963535f }, { 0.860006511f,-0.472376585f, 0.024241564f, 0.191472634f }, { 0.905142105f, 1.988715789f, 0.0f } };
	originalNodes[2].getData().localTransforms = { { 0.332452088f,-0.111552506f, 1.410963535f }, { 0.074382677f,-0.287267595f, 0.806229293f, 0.511799693f }, { 0.841695f, 0.296882f,-0.450525f } };

	auto at1 = std::make_shared<AnimationVec3Linear>();
	auto at2 = std::make_shared<AnimationVec3Linear>();
	auto at3 = std::make_shared<AnimationVec3Linear>();
	at1->addKeyFrame({ {-3.182263720f, 8.633092795f, 8.014790691f }, 0.650173135f });
	at2->addKeyFrame({ { 0.279904574f,-0.034162148f,-0.959419786f }, 8.115139435f });
	at3->addKeyFrame({ {-0.510563910f,-0.355170249f, 0.783057153f }, 2.211552098f });
	at3->addKeyFrame({ { 0.226587504f, 0.960337698f,-0.162510499f }, 3.833204925f });
	Vec3Animator atrT1(at1);
	Vec3Animator atrT2(at2);
	Vec3Animator atrT3(at3);
	atrT1.addNode(TransformationAnimator::TransformationType::Translation, &originalNodes[0]);
	atrT2.addNode(TransformationAnimator::TransformationType::Translation, &originalNodes[1]);
	atrT3.addNode(TransformationAnimator::TransformationType::Translation, &originalNodes[2]);
	atrT1.animate(3.2f);
	atrT2.animate(3.2f);
	atrT3.animate(2.0f);
	atrT3.animate(1.2f);

	auto ar1 = std::make_shared<AnimationQuatLinear>();
	auto ar2 = std::make_shared<AnimationQuatLinear>();
	auto ar3 = std::make_shared<AnimationQuatLinear>();
	ar1->addKeyFrame({ { 0.863763153f, 0.388888984f,-0.300208389f, 0.112042762f }, 3.156562131f });
	ar2->addKeyFrame({ {-0.684891402f,-0.166260287f, 0.639585793f, 0.306938827f }, 5.414548519f });
	ar3->addKeyFrame({ { 0.479462414f, 0.526573240f, 0.605637669f,-0.355020254f }, 3.106442645f });
	ar3->addKeyFrame({ { 0.334920555f, 0.151881396f, 0.693246245f,-0.619814455f }, 4.021546015f });
	QuatAnimator atrR1(ar1);
	QuatAnimator atrR2(ar2);
	QuatAnimator atrR3(ar3);
	atrR1.addNode(TransformationAnimator::TransformationType::Rotation, &originalNodes[0]);
	atrR2.addNode(TransformationAnimator::TransformationType::Rotation, &originalNodes[1]);
	atrR3.addNode(TransformationAnimator::TransformationType::Rotation, &originalNodes[2]);
	atrR1.animate(3.2f);
	atrR2.animate(3.2f);
	atrR3.animate(2.0f);
	atrR3.animate(1.2f);

	auto as1 = std::make_shared<AnimationVec3Linear>();
	auto as2 = std::make_shared<AnimationVec3Linear>();
	auto as3 = std::make_shared<AnimationVec3Linear>();
	as1->addKeyFrame({ { 1.227616805f,-5.254432319f, 8.096772882f }, 1.256742914f });
	as2->addKeyFrame({ { 0.681886255f, 0.491552144f, 0.541671037f }, 3.993066462f });
	as3->addKeyFrame({ {-0.626821994f,-0.344191372f,-0.699018180f }, 1.516154732f });
	as3->addKeyFrame({ { 0.183854460f, 0.978577852f, 0.092643812f }, 5.455646547f });
	Vec3Animator atrS1(as1);
	Vec3Animator atrS2(as2);
	Vec3Animator atrS3(as3);
	atrS1.addNode(TransformationAnimator::TransformationType::Scale, &originalNodes[0]);
	atrS2.addNode(TransformationAnimator::TransformationType::Scale, &originalNodes[1]);
	atrS3.addNode(TransformationAnimator::TransformationType::Scale, &originalNodes[2]);
	atrS1.animate(3.2f);
	atrS2.animate(3.2f);
	atrS3.animate(2.0f);
	atrS3.animate(1.2f);

	for (std::size_t i = 0; i < originalNodes.size(); ++i) {
		EXPECT_TRUE(originalNodes[i].getData().animated);
		for (int j = 0; j < 3; ++j) {
			EXPECT_NEAR(originalNodes[i].getData().localTransforms.position[j], expectedNodes[i].getData().localTransforms.position[j], kTolerance);
		}
		for (int j = 0; j < 4; ++j) {
			EXPECT_NEAR(originalNodes[i].getData().localTransforms.orientation[j], expectedNodes[i].getData().localTransforms.orientation[j], kTolerance);
		}
		for (int j = 0; j < 3; ++j) {
			EXPECT_NEAR(originalNodes[i].getData().localTransforms.scale[j], expectedNodes[i].getData().localTransforms.scale[j], kTolerance);
		}
	}
}


TEST(TransformationAnimator, resetNodesAnimatedState1)
{
	std::vector<AnimationNode> originalNodes(3);

	auto at1 = std::make_shared<AnimationVec3Linear>();
	Vec3Animator atrT1(at1);
	atrT1.addNode(TransformationAnimator::TransformationType::Translation, &originalNodes[0]);
	atrT1.addNode(TransformationAnimator::TransformationType::Translation, &originalNodes[1]);
	atrT1.addNode(TransformationAnimator::TransformationType::Translation, &originalNodes[2]);

	for (std::size_t i = 0; i < originalNodes.size(); ++i) {
		EXPECT_FALSE(originalNodes[i].getData().animated);
	}

	atrT1.animate(3.2f);
	for (std::size_t i = 0; i < originalNodes.size(); ++i) {
		EXPECT_TRUE(originalNodes[i].getData().animated);
	}

	atrT1.resetNodesAnimatedState();
	for (std::size_t i = 0; i < originalNodes.size(); ++i) {
		EXPECT_FALSE(originalNodes[i].getData().animated);
	}
}


TEST(TransformationAnimator, updateNodesWorldTransforms1)
{
	std::vector<AnimationNode> expectedNodes(1);
	expectedNodes[0].getData().worldTransforms = { {-3.182263720f, 8.633092795f, 8.014790691f }, { 0.634908735f, 0.734051764f, 0.169194266f,-0.171558305f }, { 4.445192337f,-4.281722545f, 2.230783700f } };
	std::vector<AnimationNode> originalNodes(1);
	originalNodes[0].getData().localTransforms = { {-0.616233110f, 0.166248172f,-0.367524087f }, { 0.634908735f, 0.734051764f, 0.169194266f,-0.171558305f }, { 4.445192337f,-4.281722545f, 2.230783700f } };

	auto at1 = std::make_shared<AnimationVec3Linear>();
	at1->addKeyFrame({ {-3.182263720f, 8.633092795f, 8.014790691f }, 0.650173135f });
	Vec3Animator atrT1(at1);
	atrT1.addNode(TransformationAnimator::TransformationType::Translation, &originalNodes[0]);

	atrT1.animate(3.2f);
	atrT1.updateNodesWorldTransforms();

	for (std::size_t i = 0; i < originalNodes.size(); ++i) {
		EXPECT_TRUE(originalNodes[i].getData().worldTransformsUpdated);
		for (int j = 0; j < 3; ++j) {
			EXPECT_NEAR(originalNodes[i].getData().worldTransforms.position[j], expectedNodes[i].getData().worldTransforms.position[j], kTolerance);
		}
		for (int j = 0; j < 4; ++j) {
			EXPECT_NEAR(originalNodes[i].getData().worldTransforms.orientation[j], expectedNodes[i].getData().worldTransforms.orientation[j], kTolerance);
		}
		for (int j = 0; j < 3; ++j) {
			EXPECT_NEAR(originalNodes[i].getData().worldTransforms.scale[j], expectedNodes[i].getData().worldTransforms.scale[j], kTolerance);
		}
	}
}
