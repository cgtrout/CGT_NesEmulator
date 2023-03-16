#if !defined( NesSprite_INCLUDED )
#define NesSprite_INCLUDED

#include "CGTSingleton.h"
#include "NesMemory.h"

#include <vector>

using namespace CGTSingleton;
typedef unsigned char ubyte;
typedef unsigned short uword;
typedef short word;

namespace PpuSystem {
	struct SpriteData {
	  public:
		ubyte x, y;
		ubyte tileIndex;
		ubyte verticleFlip;
		ubyte horizontalFlip;
		ubyte backGroundPriority;
		ubyte upperColorBits;
		bool isSprite0;
	};
	
	//used to hold a list of sprites contained within a scanline
	class NesSpriteScanlineResults {
	  public:
		void addSprite( SpriteData *sprite );
		std::vector< SpriteData* > *getSpriteList();
		
		//erase all entries in this list
		void clearList();
		
		//how many sprites in this scanline?
		int	 getCount() { return count; }
	
	  private:
		
		
		//sprites contained on this scanline  
		std::vector< SpriteData* > sprites;
		
		//how many sprites are there in this list
		int count;
	};

	class NesSprite {
	  public:
		void loadSpriteInformation( ubyte *spriteRamPtr );
		SpriteData *getSprite( ubyte spriteToGet );

		//get a list of sprites in given scanline
		NesSpriteScanlineResults *getScanlineList( int scanline, ubyte spriteSize ) ;
		NesSprite();
	  private:
		SpriteData sprites[ 64 ];
		NesSpriteScanlineResults scanlineResults;
	};
}
#endif //NesSprite_INCLUDED
