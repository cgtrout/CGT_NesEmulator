#include "GUI.h"

#define CONSOLE_LINES 10
namespace GUISystem {
	class GUIConsole : public GUISystem::GUIElement, public Singleton< GUIConsole > {
	public:
		GUIConsole();
		GUIConsole( std::string guitextures );
		virtual ~GUIConsole();
		
		void onLeftMouseDown();
		void onLeftMouseRelease();
		void onMouseOver();
		void onRender();
		void onRightMouseDown();
		void onRightMouseRelease();

		void onKeyDown( SDL_Keycode key );

		void update();

		void setOpen( bool val );

	private:
		GUISystem::GEDrawElement background;
		std::array<GUISystem::TextLabel, CONSOLE_LINES> lines;
		
		class ConsoleEditBox : public GUISystem::EditBox {
			void onEnterKey();
		};

	public:
		ConsoleEditBox editLine;

	private:
		
		GUISystem::Font font;

		std::string consoleLog;	

		//position from back of history that we want to print from
		int offset;
		
		void initialize( std::string guitextures );

		//has the output changed since the last frame?
		bool changed;
	};
}
