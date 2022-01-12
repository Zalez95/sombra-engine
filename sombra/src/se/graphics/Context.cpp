#include "se/utils/MathUtils.h"
#include "se/graphics/Context.h"

namespace se::graphics {

	uint32_t Context::sBindableTypeCount = 0;


	std::size_t Context::BindableRef::HashFunc::operator()(const BindableRef& ref) const
	{
		std::size_t ret = 0;
		utils::hash_combine(ret, ref.getParent());
		utils::hash_combine(ret, ref.getIndex());
		return ret;
	}


	Context::BindableRef::BindableRef(Context* parent, std::size_t index) :
		mParent(parent), mIndex(index)
	{
		if (mParent) {
			mParent->addUser(mIndex);
		}
	}


	Context::BindableRef::BindableRef(const BindableRef& other) :
		mParent(other.mParent), mIndex(other.mIndex)
	{
		if (mParent) {
			mParent->addUser(mIndex);
		}
	}


	Context::BindableRef::BindableRef(BindableRef&& other) :
		mParent(std::move(other.mParent)), mIndex(std::move(other.mIndex))
	{
		other.mParent = nullptr;
	}


	Context::BindableRef::~BindableRef()
	{
		if (mParent) {
			mParent->removeUser(mIndex);
		}
	}


	Context::BindableRef& Context::BindableRef::operator=(const BindableRef& other)
	{
		if (mParent) {
			mParent->removeUser(mIndex);
		}

		mParent = other.mParent;
		mIndex = other.mIndex;

		if (mParent) {
			mParent->addUser(mIndex);
		}

		return *this;
	}


	Context::BindableRef& Context::BindableRef::operator=(BindableRef&& other)
	{
		if (mParent) {
			mParent->removeUser(mIndex);
		}

		mParent = std::move(other.mParent);
		mIndex = std::move(other.mIndex);
		other.mParent = nullptr;
		return *this;
	}


	Context::~Context()
	{
		std::scoped_lock lock(mCommandMutex);

		for (auto it = mBindables.begin(); it != mBindables.end(); ++it) {
			remove(it.getIndex());
		}

		update();
	}


	Context& Context::update()
	{
		{
			std::scoped_lock lock(mCommandMutex);

			for (std::size_t i = 0; i < mCommandQueue.size(); ++i) {
				mCommandQueue[i]();
			}
			mCommandQueue.clear();
		}
		mCommandCV.notify_all();
		return *this;
	}


	Context& Context::wait()
	{
		std::unique_lock lck(mCommandMutex);
		mCommandCV.wait(lck);
		return *this;
	}


	Context& Context::execute(const std::function<void(Query&)>& command)
	{
		std::scoped_lock lock(mCommandMutex);
		mCommandQueue.push_back([=] {
			Query q(*this);
			command(q);
		});
		return *this;
	}


	Context::BindableRef Context::clone(std::size_t index)
	{
		std::scoped_lock lock(mCommandMutex);

		auto itBindable = mBindables.emplace();
		if (itBindable != mBindables.end()) {
			std::size_t indexClonned = itBindable.getIndex();
			itBindable->metadata = mBindables[index].metadata & 0xFF000000;

			mCommandQueue.push_back([=]() {
				if (mBindables.isActive(index)) {
					auto bindable = mBindables[index].bindable->clone();
					mBindables[indexClonned].bindable = std::move(bindable);
				}
			});

			return BindableRef(this, indexClonned);
		}

		return BindableRef();
	}

// Private functions
	void Context::addUser(std::size_t index)
	{
		std::scoped_lock lock(mCommandMutex);

		if (mBindables.isActive(index)) {
			mBindables[index].metadata++;
		}
	}


	void Context::removeUser(std::size_t index)
	{
		std::scoped_lock lock(mCommandMutex);

		if (mBindables.isActive(index)) {
			mBindables[index].metadata--;

			uint32_t numUsers = mBindables[index].metadata & 0x00FFFFFF;
			if (numUsers == 0) {
				remove(index);
			}
		}
	}


	void Context::remove(std::size_t index)
	{
		std::scoped_lock lock(mCommandMutex);

		mCommandQueue.push_back([=]() {
			if (mBindables.isActive(index)) {
				bool destroy = mBindables[index].metadata & (1u << 31);
				if (!destroy) {
					mBindables[index].bindable.release();
				}

				mBindables.erase(mBindables.begin().setIndex(index));
			}
		});
	}

}
