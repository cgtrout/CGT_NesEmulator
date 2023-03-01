#if !defined( FpsTimer__H )
#define FpsTimer__H

namespace FrontEnd {
	class FPSTimer {
	  public:
		double getAvgFps();
		
		//used to update timer at end of frame
		//also caps framerate
		void updateTimer( double elapsedTime );
		
		//clears timer for start of frame
		void clearTimer();

		FPSTimer();
	  private:
		double	elapsedTime;
		double	currTime;
		
		double 	totalElapsedTime;
		double frameNumber;

		//frames per used to calulate average fps
		static const int FRAMES_PER_SAMPLE = 60;

		//how many frames between counter update
		static const int COUNTER_SEND_RATE = 60*2;
		int		counterFrame;

		//fps of last frames
		std::array<double, FRAMES_PER_SAMPLE> frameFPS;
		
		//fps over last number of samples
		double averagedFPS;
	};
}
#endif