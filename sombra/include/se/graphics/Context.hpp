#ifndef GRAPHICS_CONTEXT_HPP
#define GRAPHICS_CONTEXT_HPP

namespace se::graphics {

	template <typename T>
	Context::TBindableRef<T>::TBindableRef(Context* parent, std::size_t index) :
		BindableRef()
	{
		if (parent && parent->checkType<T>(index)) {
			mParent = parent;
			mIndex = index;
			mParent->addUser(index);
		}
	}


	template <typename T, typename... Args>
	Context::TBindableRef<T> Context::create(Args&&... args)
	{
		std::scoped_lock lock(mCommandMutex);

		auto itBindable = mBindables.emplace();
		if (itBindable != mBindables.end()) {
			std::size_t index = itBindable.getIndex();
			itBindable->metadata = (1u << 31) | (getBindableTypeId<T>() << 24);

			mCommandQueue.push_back([=]() {
				auto bindable = std::make_unique<T>(args...);
				mBindables[index].bindable = std::move(bindable);
			});

			return TBindableRef<T>(this, index);
		}

		return TBindableRef<T>();
	}


	template <typename T>
	Context::TBindableRef<T> Context::insert(T* bindable)
	{
		std::scoped_lock lock(mCommandMutex);

		auto itBindable = mBindables.emplace();
		if (itBindable != mBindables.end()) {
			itBindable->bindable = std::unique_ptr<Bindable>(bindable);
			itBindable->metadata = getBindableTypeId<T>() << 24;
			return TBindableRef<T>(this, itBindable.getIndex());
		}

		return TBindableRef<T>();
	}


	template <typename T>
	template <typename F>
	Context::TBindableRef<T> Context::TBindableRef<T>::edit(F&& callback) const
	{
		mParent->execute([ref = *this, callback](Query& q) {
			T* tBindable = q.getTBindable<T>(ref);
			if (tBindable) {
				callback(*tBindable);
			}
		});
		return *this;
	}


	template <typename T>
	template <typename F>
	Context::TBindableRef<T> Context::TBindableRef<T>::qedit(F&& callback) const
	{
		mParent->execute([ref = *this, callback](Query& q) {
			T* tBindable = q.getTBindable<T>(ref);
			if (tBindable) {
				callback(q, *tBindable);
			}
		});
		return *this;
	}


	template <typename T>
	bool Context::checkType(std::size_t index)
	{
		if (mBindables.isActive(index)) {
			uint32_t bindableType = (mBindables[index].metadata >> 24) & 0x7Fu;
			uint32_t requestedType = getBindableTypeId<T>();
			return bindableType == requestedType;
		}
		return false;
	}


	template <typename T>
	uint32_t Context::getBindableTypeId()
	{
		static uint32_t sBindableId = sBindableTypeCount++;
		return sBindableId;
	}

}

#endif		// GRAPHICS_CONTEXT_HPP
