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
		Singleton() {}

		//get singleton instance of this class
		static T *getInstance() {
			static T singleton;
			return &singleton;
		}
	};
}