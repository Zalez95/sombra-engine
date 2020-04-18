#include <mutex>
#include <gtest/gtest.h>
#include <se/utils/TaskSet.h>

TEST(TaskSet, taskSet1)
{
	std::mutex mMutex;
	std::vector<std::string> mTaskOrder;

	se::utils::TaskManager tmana(1024);
	se::utils::TaskSet set(tmana);
	auto A = set.createTask([&]() { std::unique_lock lock(mMutex); mTaskOrder.emplace_back("A"); });
	auto B = set.createSubTaskSet([&](se::utils::SubTaskSet& sset1) {
		sset1.createTask([&]() { std::unique_lock lock(mMutex); mTaskOrder.emplace_back("B1"); });
		sset1.createSubTaskSet([&](se::utils::SubTaskSet& sset2) {
			auto B2_1 = sset2.createTask([&]() { std::unique_lock lock(mMutex); mTaskOrder.emplace_back("B2_1"); });
			auto B2_2 = sset2.createTask([&]() { std::unique_lock lock(mMutex); mTaskOrder.emplace_back("B2_2"); });
			sset2.depends(B2_2, B2_1);
		});
	});
	auto C = set.createTask([&]() { std::unique_lock lock(mMutex); mTaskOrder.emplace_back("C"); });
	auto D = set.createTask([&]() { std::unique_lock lock(mMutex); mTaskOrder.emplace_back("D"); });
	auto E = set.createTask([&]() { std::unique_lock lock(mMutex); mTaskOrder.emplace_back("E"); });

	set.depends(B, A);
	set.depends(C, A);
	set.depends(D, B);
	set.depends(D, C);
	set.depends(E, D);

	set.submitAndWait();

	EXPECT_EQ(static_cast<int>(mTaskOrder.size()), 7);
	int iB2_1 = -1;
	for (int i = 0; i < 7; ++i) {
		if (mTaskOrder[i] == "A") {
			EXPECT_EQ(i, 0);
		}
		else if (mTaskOrder[i] == "B1") {
			EXPECT_TRUE((i >= 1) && (i <= 4));
		}
		else if (mTaskOrder[i] == "B2_1") {
			EXPECT_TRUE((i >= 1) && (i <= 3));
			iB2_1 = i;
		}
		else if (mTaskOrder[i] == "B2_2") {
			EXPECT_TRUE((i >= 2) && (i <= 4) && (i > iB2_1));
		}
		else if (mTaskOrder[i] == "C") {
			EXPECT_TRUE((i >= 1) && (i <= 4));
		}
		else if (mTaskOrder[i] == "D") {
			EXPECT_EQ(i, 5);
		}
		else if (mTaskOrder[i] == "E") {
			EXPECT_EQ(i, 6);
		}
		else {
			EXPECT_TRUE(false);
		}
	}
}
