#include <gtest/gtest.h>
#include <se/animation/AnimationSystem.h>

using namespace se::animation;
static constexpr float kTolerance = 0.000001f;

TEST(AnimationSystem, update1)
{
	const std::vector<SceneNode> expectedNodes = {
		{{ "b0", { 4.445192337f, -4.281722545f, 2.230783700f }, { 0.634908735f, 0.734051764f, 0.169194266f, -0.171558305f }, { -0.616233110f, 0.166248172f, -0.367524087f } }},
		{{ "b1", { 0.905142105f, 1.988715789f, 0.0f }, { 0.860006511f, -0.472376585f, 0.024241564f, 0.191472634f }, { 0.332452088f, -0.111552506f, 1.410963535f } }},
		{{ "b2", { 0.841695f, 0.296882f, -0.450525f}, { 0.074382677f, -0.287267595f, 0.806229293f, 0.511799693f }, { 0.332452088f, -0.111552506f, 1.410963535f } }},
		{{ "b3", { 0.25f, 1.0f, 2.75f }, { 0.525976598f, -0.087920218f, 0.640181660f, 0.552979230f }, { -0.369545638f, 0.504002809f, -0.842678189f } }}
	};
	std::vector<SceneNode> originalNodes = {
		{{ "b0", { 4.445192337f, -4.281722545f, 2.230783700f }, { 0.634908735f, 0.734051764f, 0.169194266f, -0.171558305f }, { -0.616233110f, 0.166248172f, -0.367524087f } }},
		{{ "b1", { 0.905142105f, 1.988715789f, 0.0f }, { 0.860006511f, -0.472376585f, 0.024241564f, 0.191472634f }, { 0.332452088f, -0.111552506f, 1.410963535f } }},
		{{ "b2", { 0.841695f, 0.296882f, -0.450525f }, { 0.074382677f, -0.287267595f, 0.806229293f, 0.511799693f }, { 0.332452088f, -0.111552506f, 1.410963535f } }},
		{{ "b3", { 0.25f, 1.0f, 2.75f }, { 0.525976598f, -0.087920218f, 0.640181660f, 0.552979230f }, { -0.369545638f, 0.504002809f, -0.842678189f } }}
	};
	std::vector<Animation> animations = { { 7.5f, true }, { 0.0f, true }, { 2.0f, false }, { 4.5f, true } };

	AnimationSystem animationSystem;
	for (Animation& animation : animations) {
		animationSystem.addAnimation(&animation);
	}
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
		{{ "b0", { -3.798496723f, 8.799341201f, 7.647266387f }, { 0.332961529f, 0.848409652f, -0.193423464f, -0.363215088f }, { 5.456992626f, 22.498020172f, 18.062149047f } }},
		{{ "b1", { 0.332452088f, -0.111552506f, 1.410963535f }, { 0.860006511f, -0.472376585f, 0.024241564f, 0.191472634f }, { 0.905142105f, 1.988715789f, 0.0f } }},
		{{ "b2", { 0.884948372f, -0.937227845f, 1.525016903f }, { -0.850997865f, -0.043472886f, -0.517922818f, -0.075289249f }, { 0.610229551f, 0.198523178f, -0.074333041f } }},
		{{ "b3", { -0.430792808f, 0.950675666f, -0.635973989f }, { 0.098620802f, -0.588778734f, 0.738621354f, -0.313132643f }, { -0.033172354f, 0.462077677f, -0.595307648f } }},
	};
	std::vector<SceneNode> originalNodes = {
		{{ "b0", { -0.616233110f, 0.166248172f, -0.367524087f }, { 0.634908735f, 0.734051764f, 0.169194266f, -0.171558305f }, { 4.445192337f, -4.281722545f, 2.230783700f } }},
		{{ "b1", { 0.332452088f, -0.111552506f, 1.410963535f }, { 0.860006511f, -0.472376585f, 0.024241564f, 0.191472634f }, { 0.905142105f, 1.988715789f, 0.0f } }},
		{{ "b2", { 0.332452088f, -0.111552506f, 1.410963535f }, { 0.074382677f, -0.287267595f, 0.806229293f, 0.511799693f }, { 0.841695f, 0.296882f, -0.450525f } }},
		{{ "b3", { -0.369545638f, 0.504002809f, -0.842678189f }, { 0.525976598f, -0.087920218f, 0.640181660f, 0.552979230f }, { 0.25f, 1.0f, 2.75f } }}
	};
	std::vector<Animation> animations = { { 7.5f, true }, { 0.0f, true }, { 2.0f, false }, { 4.5f, true } };

	animations[0].addKeyFrame(&originalNodes[0], { { -3.182263720f, 8.633092795f, 8.014790691f }, { 0.863763153f, 0.388888984f, -0.300208389f, 0.112042762f }, { 1.227616805f, -5.254432319f, 8.096772882f }, 0.650173135f });
	animations[1].addKeyFrame(&originalNodes[1], { { 0.279904574f, -0.034162148f, -0.959419786f }, { -0.684891402f, -0.166260287f, 0.639585793f, 0.306938827f }, { 0.681886255f, 0.491552144f, 0.541671037f }, 2.993066462f });
	animations[2].addKeyFrame(&originalNodes[2], { { 0.552496254f, -0.825675308f, 0.114053405f }, { -0.506908714f, -0.452070087f, 0.691452503f, 0.246108666f }, { 0.725000798f, 0.668693900f, 0.164992049f }, 2.060792734f });
	animations[3].addKeyFrame(&originalNodes[3], { { -0.510563910f, -0.355170249f, 0.783057153f }, { 0.479462414f, 0.526573240f, 0.605637669f, -0.355020254f }, { -0.626821994f, -0.344191372f, -0.699018180f }, 2.211552098f });
	animations[3].addKeyFrame(&originalNodes[3], { { 0.226587504f, 0.960337698f, -0.162510499f }, { 0.334920555f, 0.151881396f, 0.693246245f, -0.619814455f }, { 0.183854460f, 0.978577852f, 0.092643812f }, 3.833204925f });

	AnimationSystem animationSystem;
	for (Animation& animation : animations) {
		animationSystem.addAnimation(&animation);
	}
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
