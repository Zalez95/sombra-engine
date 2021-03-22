#include <gtest/gtest.h>
#include <se/animation/TransformationAnimator.h>
#include <se/animation/SkeletonAnimator.h>
#include <se/animation/LinearAnimations.h>

using namespace se::animation;
static constexpr float kTolerance = 0.000001f;

TEST(SkeletonAnimator, loopTime1)
{
	auto at1 = std::make_shared<AnimationVec3Linear>();
	at1->addKeyFrame({ {-0.510563910f,-0.355170249f, 0.783057153f }, 2.211552098f });
	at1->addKeyFrame({ { 0.226587504f, 0.960337698f,-0.162510499f }, 3.833204925f });
	at1->addKeyFrame({ {-3.182263720f, 8.633092795f, 8.014790691f }, 0.650173135f });

	auto atrT1 = std::make_unique<Vec3Animator>(at1);
	Vec3Animator* atrT1Ptr = atrT1.get();
	SkeletonAnimator atrSkeleton;

	EXPECT_NEAR(atrSkeleton.getLoopTime(), 0.0f, kTolerance);

	atrSkeleton.addAnimator("", TransformationAnimator::TransformationType::Translation, std::move(atrT1));

	EXPECT_NEAR(atrSkeleton.getLoopTime(), 0.0f, kTolerance);
	EXPECT_NEAR(atrT1Ptr->getLoopTime(), 0.0f, kTolerance);

	atrSkeleton.setLoopTime(3.6f);

	EXPECT_NEAR(atrSkeleton.getLoopTime(), 3.6f, kTolerance);
	EXPECT_NEAR(atrT1Ptr->getLoopTime(), 3.6f, kTolerance);
}


TEST(SkeletonAnimator, animate1)
{
	std::string n1Str = "n1", n2Str = "n2", n3Str = "n3";
	std::array<char, NodeData::kMaxLength> n1{}, n2{}, n3{};
	std::copy(n1Str.begin(), n1Str.end(), n1.begin());
	std::copy(n2Str.begin(), n2Str.end(), n2.begin());
	std::copy(n3Str.begin(), n3Str.end(), n3.begin());

	std::vector<AnimationNode> expectedNodes(3);
	expectedNodes[0].getData().name = n1;
	expectedNodes[0].getData().localTransforms = { {-3.182263720f, 8.633092795f, 8.014790691f }, { 0.863763153f, 0.388888984f,-0.300208389f, 0.112042762f }, { 1.227616805f,-5.254432319f, 8.096772882f } };
	expectedNodes[1].getData().name = n2;
	expectedNodes[1].getData().localTransforms = { { 0.279904574f,-0.034162148f,-0.959419786f }, {-0.684891402f,-0.166260287f, 0.639585793f, 0.306938827f }, { 0.681886255f, 0.491552144f, 0.541671037f } };
	expectedNodes[2].getData().name = n3;
	expectedNodes[2].getData().localTransforms = { {-0.260901302f, 0.090373396f, 0.462806969f }, { 0.479462414f, 0.526573240f, 0.605637669f,-0.355020254f }, {-0.370700001f, 0.073719248f,-0.448903560f } };
	AnimationNode root;
	auto it = root.emplace(root.cend());
	it->getData().name = n1;
	it->getData().localTransforms = { {-0.616233110f, 0.166248172f,-0.367524087f }, { 0.634908735f, 0.734051764f, 0.169194266f,-0.171558305f }, { 4.445192337f,-4.281722545f, 2.230783700f } };
	it = root.emplace(root.cend());
	it->getData().name = n2;
	it->getData().localTransforms = { { 0.332452088f,-0.111552506f, 1.410963535f }, { 0.860006511f,-0.472376585f, 0.024241564f, 0.191472634f }, { 0.905142105f, 1.988715789f, 0.0f } };
	it = root.emplace(root.cend());
	it->getData().name = n3;
	it->getData().localTransforms = { { 0.332452088f,-0.111552506f, 1.410963535f }, { 0.074382677f,-0.287267595f, 0.806229293f, 0.511799693f }, { 0.841695f, 0.296882f,-0.450525f } };

	auto at1 = std::make_shared<AnimationVec3Linear>();
	auto at2 = std::make_shared<AnimationVec3Linear>();
	auto at3 = std::make_shared<AnimationVec3Linear>();
	at1->addKeyFrame({ {-3.182263720f, 8.633092795f, 8.014790691f }, 0.650173135f });
	at2->addKeyFrame({ { 0.279904574f,-0.034162148f,-0.959419786f }, 8.115139435f });
	at3->addKeyFrame({ {-0.510563910f,-0.355170249f, 0.783057153f }, 2.211552098f });
	at3->addKeyFrame({ { 0.226587504f, 0.960337698f,-0.162510499f }, 3.833204925f });
	auto atrT1 = std::make_unique<Vec3Animator>(at1);
	auto atrT2 = std::make_unique<Vec3Animator>(at2);
	auto atrT3 = std::make_unique<Vec3Animator>(at3);

	auto ar1 = std::make_shared<AnimationQuatLinear>();
	auto ar2 = std::make_shared<AnimationQuatLinear>();
	auto ar3 = std::make_shared<AnimationQuatLinear>();
	ar1->addKeyFrame({ { 0.863763153f, 0.388888984f,-0.300208389f, 0.112042762f }, 3.156562131f });
	ar2->addKeyFrame({ {-0.684891402f,-0.166260287f, 0.639585793f, 0.306938827f }, 5.414548519f });
	ar3->addKeyFrame({ { 0.479462414f, 0.526573240f, 0.605637669f,-0.355020254f }, 3.106442645f });
	ar3->addKeyFrame({ { 0.334920555f, 0.151881396f, 0.693246245f,-0.619814455f }, 4.021546015f });
	auto atrR1 = std::make_unique<QuatAnimator>(ar1);
	auto atrR2 = std::make_unique<QuatAnimator>(ar2);
	auto atrR3 = std::make_unique<QuatAnimator>(ar3);

	auto as1 = std::make_shared<AnimationVec3Linear>();
	auto as2 = std::make_shared<AnimationVec3Linear>();
	auto as3 = std::make_shared<AnimationVec3Linear>();
	as1->addKeyFrame({ { 1.227616805f,-5.254432319f, 8.096772882f }, 1.256742914f });
	as2->addKeyFrame({ { 0.681886255f, 0.491552144f, 0.541671037f }, 3.993066462f });
	as3->addKeyFrame({ {-0.626821994f,-0.344191372f,-0.699018180f }, 1.516154732f });
	as3->addKeyFrame({ { 0.183854460f, 0.978577852f, 0.092643812f }, 5.455646547f });
	auto atrS1 = std::make_unique<Vec3Animator>(as1);
	auto atrS2 = std::make_unique<Vec3Animator>(as2);
	auto atrS3 = std::make_unique<Vec3Animator>(as3);

	SkeletonAnimator atrSkeleton(4.0f);
	atrSkeleton.addAnimator(n1Str.c_str(), TransformationAnimator::TransformationType::Translation, std::move(atrT1));
	atrSkeleton.addAnimator(n2Str.c_str(), TransformationAnimator::TransformationType::Translation, std::move(atrT2));
	atrSkeleton.addAnimator(n3Str.c_str(), TransformationAnimator::TransformationType::Translation, std::move(atrT3));
	atrSkeleton.addAnimator(n1Str.c_str(), TransformationAnimator::TransformationType::Rotation, std::move(atrR1));
	atrSkeleton.addAnimator(n2Str.c_str(), TransformationAnimator::TransformationType::Rotation, std::move(atrR2));
	atrSkeleton.addAnimator(n3Str.c_str(), TransformationAnimator::TransformationType::Rotation, std::move(atrR3));
	atrSkeleton.addAnimator(n1Str.c_str(), TransformationAnimator::TransformationType::Scale, std::move(atrS1));
	atrSkeleton.addAnimator(n2Str.c_str(), TransformationAnimator::TransformationType::Scale, std::move(atrS2));
	atrSkeleton.addAnimator(n3Str.c_str(), TransformationAnimator::TransformationType::Scale, std::move(atrS3));

	atrSkeleton.addNodeHierarchy(root);

	atrSkeleton.animate(3.851118047f);
	atrSkeleton.animate(2.909664586f);

	std::size_t i = 0;
	for (auto itNode = root.cbegin(); itNode != root.cend(); ++itNode) {
		for (int j = 0; j < 3; ++j) {
			EXPECT_NEAR(itNode->getData().localTransforms.position[j], expectedNodes[i].getData().localTransforms.position[j], kTolerance);
		}
		for (int j = 0; j < 4; ++j) {
			EXPECT_NEAR(itNode->getData().localTransforms.orientation[j], expectedNodes[i].getData().localTransforms.orientation[j], kTolerance);
		}
		for (int j = 0; j < 3; ++j) {
			EXPECT_NEAR(itNode->getData().localTransforms.scale[j], expectedNodes[i].getData().localTransforms.scale[j], kTolerance);
		}
		++i;
	}
}


TEST(SkeletonAnimator, resetNodesAnimatedState1)
{
	std::string n1Str = "n1", n2Str = "n2", n3Str = "n3", n4Str = "n4";
	std::array<char, NodeData::kMaxLength> n1, n2, n3, n4;
	std::copy(n1Str.begin(), n1Str.end(), n1.begin());
	std::copy(n2Str.begin(), n2Str.end(), n2.begin());
	std::copy(n3Str.begin(), n3Str.end(), n3.begin());
	std::copy(n4Str.begin(), n4Str.end(), n4.begin());

	std::vector<AnimationNode> originalNodes(3);
	originalNodes[0].getData().name = n1;
	originalNodes[1].getData().name = n2;
	originalNodes[2].getData().name = n3;
	NodeData nodeData;
	nodeData.name = n4;
	originalNodes[0].emplace(originalNodes[0].cend(), nodeData);

	auto at1 = std::make_shared<AnimationVec3Linear>();
	auto atrT1 = std::make_unique<Vec3Animator>(at1);
	atrT1->addNode(TransformationAnimator::TransformationType::Translation, originalNodes[0]);
	atrT1->addNode(TransformationAnimator::TransformationType::Translation, originalNodes[1]);
	auto atrT2 = std::make_unique<Vec3Animator>(at1);
	atrT2->addNode(TransformationAnimator::TransformationType::Translation, originalNodes[2]);
	SkeletonAnimator atrSkeleton;
	atrSkeleton.addAnimator(n1Str.c_str(), TransformationAnimator::TransformationType::Translation, std::move(atrT1));
	atrSkeleton.addAnimator(n4Str.c_str(), TransformationAnimator::TransformationType::Translation, std::move(atrT2));
	atrSkeleton.addNodeHierarchy(originalNodes[0]);

	for (std::size_t i = 0; i < originalNodes.size(); ++i) {
		EXPECT_FALSE(originalNodes[i].getData().animated);
	}
	EXPECT_FALSE(originalNodes[0].begin()->getData().animated);

	atrSkeleton.animate(3.2f);
	atrSkeleton.updateNodesHierarchy();
	EXPECT_TRUE(originalNodes[0].getData().animated);
	EXPECT_TRUE(originalNodes[0].begin()->getData().animated);
	EXPECT_FALSE(originalNodes[1].getData().animated);
	EXPECT_FALSE(originalNodes[2].getData().animated);

	atrSkeleton.resetNodesAnimatedState();
	for (std::size_t i = 0; i < originalNodes.size(); ++i) {
		EXPECT_FALSE(originalNodes[i].getData().animated);
	}
	EXPECT_FALSE(originalNodes[0].begin()->getData().animated);
}


TEST(SkeletonAnimator, updateNodesHierarchy1)
{
	std::string nameStr = "NODE";
	std::array<char, NodeData::kMaxLength> name{};
	std::copy(nameStr.begin(), nameStr.end(), name.begin());

	std::vector<AnimationNode> expectedNodes(1);
	expectedNodes[0].getData().name = name;
	expectedNodes[0].getData().localTransforms = { {-3.182263720f, 8.633092795f, 8.014790691f }, { 0.634908735f, 0.734051764f, 0.169194266f,-0.171558305f }, { 4.445192337f,-4.281722545f, 2.230783700f } };
	expectedNodes[0].getData().worldTransforms = expectedNodes[0].getData().localTransforms;

	AnimationNode root;
	auto it = root.emplace(root.cend());
	it->getData().name = name;
	it->getData().localTransforms = { {-0.616233110f, 0.166248172f,-0.367524087f }, { 0.634908735f, 0.734051764f, 0.169194266f,-0.171558305f }, { 4.445192337f,-4.281722545f, 2.230783700f } };
	it->getData().worldTransforms = it->getData().localTransforms;

	auto at1 = std::make_shared<AnimationVec3Linear>();
	at1->addKeyFrame({ {-3.182263720f, 8.633092795f, 8.014790691f }, 0.650173135f });
	auto atrT1 = std::make_unique<Vec3Animator>(at1);
	SkeletonAnimator atrSkeleton(4.0f);
	atrSkeleton.addAnimator(nameStr.c_str(), TransformationAnimator::TransformationType::Translation, std::move(atrT1));

	atrSkeleton.addNodeHierarchy(root);

	atrSkeleton.animate(3.2f);
	atrSkeleton.updateNodesHierarchy();

	EXPECT_TRUE(root.getData().animated);
	EXPECT_TRUE(root.getData().worldTransformsUpdated);
	EXPECT_TRUE(it->getData().animated);
	EXPECT_TRUE(it->getData().worldTransformsUpdated);
	for (int j = 0; j < 3; ++j) {
		EXPECT_NEAR(it->getData().worldTransforms.position[j], expectedNodes[0].getData().worldTransforms.position[j], kTolerance);
	}
	for (int j = 0; j < 4; ++j) {
		EXPECT_NEAR(it->getData().worldTransforms.orientation[j], expectedNodes[0].getData().worldTransforms.orientation[j], kTolerance);
	}
	for (int j = 0; j < 3; ++j) {
		EXPECT_NEAR(it->getData().worldTransforms.scale[j], expectedNodes[0].getData().worldTransforms.scale[j], kTolerance);
	}
}
