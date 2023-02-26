/**************************************************
 ** GL Implementation of GUI rendering fuctions
 **
 **/
#include "precompiled.h"

#include "GUI.h"
#include "GLGeneral.h"
#include "Console.h"
#include "GLRenderer.h"
using namespace GUISystem;
//using namespace Render;

//TODO this turns off the conversion ( From int to float ) warning
//this should be fixed correctly
#if _MSC_VER > 1000
#pragma warning( disable : 4244 )
#endif

Console::ConsoleSystem *console;
Render::Renderer *renderSystem;
using namespace Console;

ConsoleVariable< float > *opacity;
/*
==============================================
GUI::render()

  renders ui
==============================================
*/
void GUI::render() {
	zdrawpos = 0.0001f;
	
	console = &FrontEnd::SystemMain::getInstance()->consoleSystem;
	renderSystem = &FrontEnd::SystemMain::getInstance()->renderer;
	
	if( opacity == NULL ) {
		opacity = console->variables.getFloatVariable( &std::string("guiOpacity") );
	}

	//render gui elements
	renderElements( elements );
}


/*
==============================================
void GUI::renderDebugLines()

  renders lines around a gui element for debugging
==============================================
*/
void GUI::renderDebugLines( GUIElement *element ) {
	glDisable( GL_TEXTURE_2D );
	
	GuiDim yres = renderSystem->getyRes();

	glBegin( GL_LINE_STRIP );
		glColor4f( 255, 255, 255, 255 );     
  		glVertex3f( element->getX(), yres - element->getY(), zdrawpos );
		glVertex3f( element->getX() + ( element->getWidth() ), yres - element->getY(), zdrawpos );
		glVertex3f( element->getX() + element->getWidth(), yres - ( element->getY() + element->getHeight() ), zdrawpos );
		glVertex3f( element->getX(), yres - ( element->getY() + element->getHeight() ), zdrawpos );
		glVertex3f( element->getX(), yres - element->getY(), zdrawpos );
	glEnd();

	glEnable( GL_TEXTURE_2D );
}

/*
==============================================
GUI::renderDrawList( list< GEDrawElement* > drawList )

  renders drawList
==============================================
*/
void GUI::renderDrawList( std::list< GEDrawElement* > drawList ) {
	if( !drawGUI ) return;

	std::list< GEDrawElement* >::iterator iter;
	GEDrawElement *curr;
	
	GuiDim posx;
	GuiDim posy;
	GuiDim yres = renderSystem->getyRes();
	
	//go through drawList and draw all of the elements contained within
	for( iter = drawList.begin(); iter != drawList.end(); ++iter ) {
		curr = ( GEDrawElement* )( *iter );

		posx = curr->x;
		posy = curr->y;

		glBindTexture( GL_TEXTURE_2D, curr->imageid );
		glColor4f( 1.0f, 1.0f, 1.0f, *opacity * curr->opacity );			

		glBegin( GL_POLYGON );
		switch( curr->stretchType ) {
		case ST_X:
				glTexCoord2f( 0.0, 0.0 ); glVertex3f( posx, yres - posy - curr->image->sizeY, zdrawpos );
				glTexCoord2f( 0.0, 1.0 ); glVertex3f( posx, yres- posy, zdrawpos );
				glTexCoord2f( 1.0, 1.0 ); glVertex3f( posx + curr->stretchFactorx, yres - posy, zdrawpos );
				glTexCoord2f( 1.0, 0.0 ); glVertex3f( posx + curr->stretchFactorx, yres - posy - curr->image->sizeY, zdrawpos );
			break;
		case ST_Y:
				glTexCoord2f( 0.0, 0.0 ); glVertex3f( posx, yres - ( posy + curr->stretchFactory ), zdrawpos );
				glTexCoord2f( 0.0, 1.0 ); glVertex3f( posx,  yres - posy, zdrawpos );
				glTexCoord2f( 1.0, 1.0 ); glVertex3f( posx + curr->image->sizeX, yres - posy, zdrawpos );
				glTexCoord2f( 1.0, 0.0 ); glVertex3f( posx + curr->image->sizeX, yres - posy - curr->image->sizeY - curr->stretchFactory, zdrawpos );
			break;
		case ST_XY:
				glTexCoord2f( 0.0, 0.0 ); glVertex3f( posx, yres - ( posy + curr->stretchFactory ), zdrawpos );
				glTexCoord2f( 0.0, 1.0 ); glVertex3f( posx,  yres - posy, zdrawpos );
				glTexCoord2f( 1.0, 1.0 ); glVertex3f( posx + curr->stretchFactorx,yres - posy, zdrawpos );
				glTexCoord2f( 1.0, 0.0 ); glVertex3f( posx + curr->stretchFactorx, yres - ( posy + curr->stretchFactory ), zdrawpos );
			break;
		case ST_NONE:
				glTexCoord2f( 0.0, 0.0 ); glVertex3f( posx, yres - ( posy + curr->image->sizeY ), zdrawpos );
				glTexCoord2f( 0.0, 1.0 ); glVertex3f( posx, yres - posy, zdrawpos );
				glTexCoord2f( 1.0, 1.0 ); glVertex3f( posx + curr->image->sizeX, yres - posy, zdrawpos );
				glTexCoord2f( 1.0, 0.0 ); glVertex3f( posx + curr->image->sizeX, yres - ( posy + curr->image->sizeY ), zdrawpos );
		}	
		glEnd();
		
		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );			
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );		
	}
}

/*
==============================================
GUI::renderTextLabel( TextLabel *fs )
	
  text labels must be drawn differently from drawList based gui elements
==============================================
*/
void GUI::renderTextLabel( TextLabel *fs ) {
	if( !drawGUI ) return;
	
	Font *font = fs->getFont();
	int x = fs->getX();	//x must move every time a letter is printed
	int yres = renderSystem->getyRes();

	glBindTexture( GL_TEXTURE_2D, font->geImageid() );

	//prevent outright crash if the font is not inialized
	if( font->geImage() == NULL ) {
		throw GUIRunException( "GUI::renderTextLabel", "Font not intialized!" );
	}
	
	//get console opacity value
	glColor4f( 1.0f, 1.0f, 0.5f, *opacity * (*console->variables.getFloatVariable( &std::string("fontOpacity"))));
	
	unsigned int length = fs->getString().length();
	
		for( unsigned int i = 0 ; i < length; i++ ) {
			glBegin( GL_QUADS );
			TextureCoord *c = font->getCoord( fs->getString()[ i ] );
			
			glTexCoord2f( c->x0, c->y0 ); 
			glVertex3f( x, yres - fs->getY() - font->getFontHeight(), zdrawpos );
			
			glTexCoord2f( c->x0, c->y1 );
			glVertex3f( x, yres - fs->getY(), zdrawpos );
			
			glTexCoord2f( c->x1, c->y1 ); 
			glVertex3f( x + font->getFontWidth(), yres - fs->getY(), zdrawpos );
			
			glTexCoord2f( c->x1, c->y0 );
			glVertex3f( x + font->getFontWidth(), yres - fs->getY() - font->getFontHeight(), zdrawpos );

			x += font->getFontWidth();
			glEnd();
		}
		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );			
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );			
}
