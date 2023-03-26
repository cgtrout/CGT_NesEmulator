#pragma once

#include "Input.h"

#define NUM_NES_BUTTONS 8
using namespace FrontEnd::InputSystem;
namespace NesEmulator {
	/*
	================================================================
	================================================================
	Class
	NesController

	  represents one nes controller
	================================================================
	================================================================
	*/
	class NesController : public Controllable {
	  public:
		NesController( Input* inputSystem );

		//clear all button states
		void clear();
		
		//get next strobe value from controller
		ubyte getNextStrobe();

		//resets strobe to 1
		void resetStrobe() { strobeCount = 1; }
		
		//write value to controller
		void write( ubyte val );

		//is this controller connected?
		ubyte isConnected() { return connected; }
		
		//connect controller to system
		void connect() { connected = true; }
		
	  private:

		Input* inputSystem;

		ControllableButton buttons[ NUM_NES_BUTTONS ];

		enum {
			NES_BUTTON_A,		NES_BUTTON_B, 
			NES_BUTTON_SELECT,	NES_BUTTON_START, 
			NES_BUTTON_UP,		NES_BUTTON_DOWN,
			NES_BUTTON_LEFT,	NES_BUTTON_RIGHT
		};
		
		ubyte connected;

		ubyte strobeCount;
	};

#include "CGTSingleton.h"
	enum {
		CONTROLLER01,
		CONTROLLER02,
	};
	/*
	================================================================
	================================================================
	Class
	NesControllerSystem

	  this class contains all of the controllers used by the nes
	================================================================
	================================================================
	*/
	class NesControllerSystem {
	  public:	
		NesControllerSystem( Input* inputSystem );
	    NesController controller[ 2 ];

	  private:
		Input* inputSystem;
	};
}