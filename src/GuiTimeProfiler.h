#include "GUI.h"

#define CONSOLE_LINES 10
namespace GUISystem {
	class GuiTimeProfiler : public GUISystem::GUIElement {
	  public:
		GuiTimeProfiler();
		GuiTimeProfiler( std::string guitextures );

		void initialize( );

		~GuiTimeProfiler();
		
		void onLeftMouseDown();
		void onLeftMouseRelease();
		void onMouseOver();
		void onRender();
		void onRightMouseDown();
		void onRightMouseRelease();

		void onKeyDown( unsigned char key );

		void update();

		void setX ( GuiDim val );
		void setY ( GuiDim val );

		void setReportString( const std::string &val );

	  private:
		
		GUISystem::MultiLineTextLabel lines;
		GUISystem::GEDrawElement background;
		
		//string with formatted profile data
		std::string reportString;

		GUISystem::Font font;

		std::string consoleLog;	

		//position from back of history that we want to print from
		int offset;

		float fps;
		
		void initialize( std::string guitextures );
	};
}
