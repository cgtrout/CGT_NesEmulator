#if !defined( ImageTools_INCLUDED )
#define ImageTools_INCLUDED

//TODO somewhat messy / not very object oriented design


/*
================================================================
================================================================
Class DrawComponentBase

  all draw classes must extend from this class

  can not be instantiated
================================================================
================================================================
*/
class DrawComponentBase {
  public:
	virtual void draw() = 0;
	void setImage( Image *i ) { image = i; }
	
	Vec2d pos;
	Pixel3Byte color;

	DrawComponentBase(): 
		image( 0 ), pos( 0, 0 ) {		}

	//ensures image has been loaded else throws exception
	void checkInitialized();
  protected:
	Image *image;
};


/*
================================================================
================================================================
Class LineDraw : public DrawComponentBase

  draws lines with variable slope.  Automatically handles 
  verticle line special case.
================================================================
================================================================
*/
class LineDraw : public DrawComponentBase {
  public:
	bool dotted;
	int dotSpacing;
	Vec2d end;

	void draw();
	
	void drawVerticle( const Vec2d *pos, const uword length, const Pixel3Byte *color );
	void drawHorizontal( const Vec2d *pos, const uword length, const Pixel3Byte *color );

	LineDraw(): 
		dotted( true ), dotSpacing( 4 ), end( 0, 0 ) {}

  private:
	//for internal calculations
	float slope;
	int offset;
	int calc_y( int x ) { return (int)( slope * x ) + offset; }

};

//really just draws a box


/*
================================================================
================================================================
Class PointDraw : public DrawComponentBase

  draws a "point" which is really just a square with a specified 
  "radius" (length of one side)
================================================================
================================================================
*/
class PointDraw : public DrawComponentBase {
public:
	int radius;

	void draw();

	PointDraw(): radius( 2 ){};
};


class ImageToolsException : public CgtException {
  public:
	ImageToolsException( string header, string m, bool s = true ) {
		::CgtException( header, m, s );
	}			
};


#endif
