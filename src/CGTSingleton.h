#pragma once

namespace CGTSingleton {
	/*
	================================================================
	================================================================
	Class Singleton
	  
	  "there can be only one"
	================================================================
	================================================================
	*/
	template < class T >
	class Singleton {
	public:
		//get singleton instance of this class
		static T *getInstance() {
			static T singleton;
			return &singleton;
		}

	protected:
		Singleton() {}

		// non-copyable
		Singleton(const Singleton&) = delete;
		Singleton& operator=(const Singleton&) = delete;

		// non-movable
		Singleton(Singleton&&) = delete;
		Singleton& operator=(Singleton&&) = delete;
	};
}