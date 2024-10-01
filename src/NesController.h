#pragma once

#include "Input.h"

auto constexpr NUM_NES_BUTTONS = 8;

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
	class NesController : public FrontEnd::InputSystem::Controllable {
	  public:
		NesController( FrontEnd::InputSystem::Input *inputSystem );

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

		FrontEnd::InputSystem::Input* inputSystem;

		FrontEnd::InputSystem::ControllableButton buttons[ NUM_NES_BUTTONS ];

		enum {
			NES_BUTTON_A,		NES_BUTTON_B, 
			NES_BUTTON_SELECT,	NES_BUTTON_START, 
			NES_BUTTON_UP,		NES_BUTTON_DOWN,
			NES_BUTTON_LEFT,	NES_BUTTON_RIGHT
		};
		
		ubyte connected;

		ubyte strobeCount;
	};

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
		NesControllerSystem( FrontEnd::InputSystem::Input* inputSystem );
	    NesController controller[ 2 ];

	  private:
		FrontEnd::InputSystem::Input* inputSystem;
	};
}