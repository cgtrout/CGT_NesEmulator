#include "GUI.h"

#define CONSOLE_LINES 10
namespace GUISystem {
	class GUIFPSCounter : public GUISystem::GUIElement {
	  public:
		GUIFPSCounter();
		GUIFPSCounter( std::string guitextures );

		virtual ~GUIFPSCounter();
		
		void onLeftMouseDown();
		void onLeftMouseRelease();
		void onMouseOver();
		void onRender();
		void onRightMouseDown();
		void onRightMouseRelease();

		void onKeyDown( unsigned char key );

		void update();

		void setFps( float f ) { fps = f; }
		void setX ( GuiDim val );
		void setY ( GuiDim val );

	  private:
		
		GUISystem::TextLabel fpsCounter;
		GUISystem::GEDrawElement background;
		
		GUISystem::Font font;

		std::string consoleLog;	

		//position from back of history that we want to print from
		int offset;

		float fps;
		
		void initialize( std::string guitextures );
	};
}
