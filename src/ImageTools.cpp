#include "precompiled.h"

#include "Image.h"
#include "ImageTools.h"

void DrawComponentBase::checkInitialized() {
	if( image == 0 ) {
		throw ImageToolsException( "DrawComponent", "Image not yet set" );
	}
}

//dotted lines won't look all that great on 
void LineDraw::draw() {
	try {
		checkInitialized();
	} catch( exception e ) {
		throw e;
	}

	_ASSERT( pos.x < image->sizeX );
	_ASSERT( pos.y < image->sizeY );
	_ASSERT( end.x < image->sizeX );
	_ASSERT( end.y < image->sizeY );

	//check to see if the line is straight verticle
	if( pos.x == end.x ) {
		drawVerticle( pos.y < end.y ? &pos : &end, abs( pos.y - end.y ), &color );
		return;
	}

	//determine line eqn
	slope = (float)( end.y - pos.y ) / (float)( end.x - pos.x );
	offset	= end.y - (int)( slope * end.x );

	int x;
	int start_x;
	int runLength;
	int y;
	Vec2d lastPos( 0, 0 );	//pos last loop
	int dotPos = 0;	//for dotted lines
	int dotPatternLength = dotSpacing * 2;
	bool draw;
	if( dotted ) {
		dotPos = 0;
	}
	
	//start from point that is on the left side of the other
	if( end.x > pos.x ) {
		start_x = pos.x;
		runLength = end.x - pos.x;
	} else {
		start_x = end.x;
		runLength = pos.x - end.x;
	}

	//run through all x points on the line and plot them on image
	for( x = start_x; x < runLength; ++x ) {
		draw = true;
		y = calc_y( x );
		Vec2d pos( x, y );
		
		//handle dotted lines
		if( dotted ) {
			dotPos++;
			if( dotPos > dotSpacing ) {
				draw = false;
			}
			if( dotPos == dotPatternLength ) {
				dotPos = 0;
			}
		}
		
		//now plot pixel
		if( draw ) {
			image->plotPixel( &pos, &color );
		}
		
		//now connect last pixel with this pixel to avoid gaps in line
		//this divides up the line into multiple verticle lines and draws them
		if( x != start_x ) {
			int xdelta = pos.x - lastPos.x;
			int ydelta = pos.y - lastPos.y;

			int vertLength = ydelta / xdelta;

			int ypos = lastPos.y;
			for( int line = 0; line < xdelta; ++line ) {
				drawVerticle( &Vec2d( lastPos.x + line, ypos ), vertLength, &color );
				ypos += vertLength;
				
				//last line (special case)
				if( line == ( xdelta - 1 ) ) {

				}
			}

		}

		lastPos = pos;
	}
}

//draw a completely verticle line
void LineDraw::drawVerticle( const Vec2d *pos, const uword length, const Pixel3Byte *color ) {
	Vec2d x( *pos );
	for( int y = pos->y; y < pos->y + length; ++y ) {
		image->plotPixel( &x, color );
		++x.y;
	}
}

//draw a perfectly horizontal line
void LineDraw::drawHorizontal( const Vec2d *pos, const uword length, const Pixel3Byte *color ) {
	Vec2d p = *pos;
	for( int x = 0; x < length; ++x ) {
		image->plotPixel( &p, color );
		++p.x;
	}
}

void PointDraw::draw() {
	checkInitialized();

	_ASSERT( pos.x < image->sizeX );
	_ASSERT( pos.y < image->sizeY );

	int size = (int)((float)radius / 2.0f);

	if( radius == 1 ) {
		image->plotPixel( &pos, &color );
		return;
	}

	int xstart	= pos.x - size;
	int xend	= pos.x + size;
	int ystart	= pos.y - size;
	int yend	= pos.y + size;

	//ensure values are in range
	xstart	= FORCE_LOW<int>( xstart, 0 );
	xend	= FORCE_HIGH<int>( xend, image->sizeX - 1 );
	ystart  = FORCE_LOW<int>( ystart, 0 );
	yend	= FORCE_HIGH<int>( yend, image->sizeY - 1 );
	
	for( int x = xstart; x < xend ; ++x ) {
		for( int y = ystart; y < yend; ++y ) {
			image->plotPixel( &Vec2d( x, y ), &color );
		}
	}
}


