#if !defined( FpsTimer__H )
#define FpsTimer__H

namespace FrontEnd {
	class FPSTimer {
	  public:
		float getAvgFps();
		
		//used to update timer at end of frame
		//also caps framerate
		void updateTimer( float elapsedTime );
		
		//clears timer for start of frame
		void clearTimer();

		FPSTimer();
	  private:
		float	elapsedTime;
		float	currTime;
		
		float	totalElapsedTime;
		float	frameNumber;

		//frames per used to calulate average fps
		static const int FRAMES_PER_SAMPLE = 60;

		//how many frames between counter update
		static const int COUNTER_SEND_RATE = 60*2;
		int		counterFrame;

		//fps of last frames
		float	frameFPS[ FRAMES_PER_SAMPLE ];
		
		//fps over last number of samples
		float	averagedFPS;
	};
}
#endif