// FrameRate.h: interface for the FrameRate class.
//
//////////////////////////////////////////////////////////////////////

#if !defined( AFX_FRAMERATE_H__CDD8F8CF_64A9_4AF0_9F5A_4E149426BCA2__INCLUDED_ )
#define AFX_FRAMERATE_H__CDD8F8CF_64A9_4AF0_9F5A_4E149426BCA2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CGTSingleton.h"

using namespace CGTSingleton;

namespace FrontEnd {

	/*
	================================================================
	================================================================
	Class Timer 

	================================================================
	================================================================
	*/
	class Timer : public Singleton< Timer > {
	  public:
		friend Singleton< Timer >;
		virtual ~Timer() {		}
		
		void  init();
		//float getElapsedTime();
		float getCurrTime();
		float getAbsoluteTime();
		void stopTimer();
		void startTimer();
		void reset();
	  private:
		Timer() {}
	};
}

#endif // !defined( AFX_FRAMERATE_H__CDD8F8CF_64A9_4AF0_9F5A_4E149426BCA2__INCLUDED_ )
