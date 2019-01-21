#pragma once

#include<exception>
#include<string>
#include<array>
#include<functional>

namespace worm {
	template<class T>
	class Type {
	private:
		bool written = false;
		T value;
	public:
		Type(T val) {
			this->value = val;
			written = true;
		}
		Type() {}
		~Type() {}
		Type<T>& operator=(T val) {
			if (written) throw std::exception("Type can only be written once!");
			this->value = val;
			written = true;
			return *this;
		}
		bool isSet() { return written; }

		template<class U>
		const U& as(std::function<U(const Type<T>)> converter) {
			return converter((*this));
		}

		operator T() const { return value; }
	};

	
}
