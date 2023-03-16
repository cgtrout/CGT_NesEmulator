#include "precompiled.h"

using namespace NesEmulator;
#include <sstream>

/* 
==============================================
NesController::NesController()
==============================================
*/
NesController::NesController() {
	static int controllerNum = 0;
	clear();
	connected = 1;

	//initialize buttons
	for( int i = 0; i < NUM_NES_BUTTONS; i++ ) {
		buttons[ i ].bindName = std::string();
		switch( i ) {
			case NES_BUTTON_A:
				buttons[ i ].name = "a";
				break;
			case NES_BUTTON_B:
				buttons[ i ].name = "b";
				break;
			case NES_BUTTON_SELECT:
				buttons[ i ].name = "select";
				break;
			case NES_BUTTON_START:
				buttons[ i ].name = "start";
				break;
			case NES_BUTTON_UP:
				buttons[ i ].name = "up";
				break;
			case NES_BUTTON_DOWN:
				buttons[ i ].name = "down";
				break;
			case NES_BUTTON_LEFT:
				buttons[ i ].name = "left";
				break;
			case NES_BUTTON_RIGHT:
				buttons[ i ].name = "right";
				break;
		}
		addButton( &buttons[ i ] );
	}
	name = "controller" + std::to_string(controllerNum + 1);

	//add this control to input system
	Input::getInstance()->addControl( dynamic_cast< Controllable* >( this ) );
	controllerNum++;
}
/* 
==============================================
void NesController::clear() 
==============================================
*/
void NesController::clear() { 
	for( int x = 0; x < NUM_NES_BUTTONS; x++ ) {
		buttons[ x ].setState( NOT_PRESSED ) ; 
	}
}
/*
==============================================
ubyte NesController::getNextStrobe()
==============================================
*/
ubyte NesController::getNextStrobe() {
	ubyte returnVal = 0x40;
	
	switch( strobeCount++ ) {
	case 1:
		returnVal += buttons[ NES_BUTTON_A ].getState();
		break;
	case 2:
		returnVal += buttons[ NES_BUTTON_B ].getState();
		break;
	case 3:
		returnVal += buttons[ NES_BUTTON_SELECT ].getState();
		break;
	case 4:
		returnVal += buttons[ NES_BUTTON_START ].getState();
		break;
	case 5:
		returnVal += buttons[ NES_BUTTON_UP ].getState();
		break;
	case 6:
		returnVal += buttons[ NES_BUTTON_DOWN ].getState();
		break;
	case 7:
		returnVal += buttons[ NES_BUTTON_LEFT ].getState();
		break;
	case 8:
		returnVal += buttons[ NES_BUTTON_RIGHT ].getState();
		break;
	case 9:
	case 10:
	case 11:
	case 12:
	case 13: 
	case 14:
	case 15:
	case 16:
		returnVal += 1;
		break;
	case 17:
		//FIXME temporary hack
		returnVal += 1;
		break;
	case 18:
		returnVal += 1;
		//returnVal += a;
		break;
	case 19:
		returnVal += 1;
		//returnVal += a;
		break;
	
	case 20:
		returnVal += 1;
		//returnVal += a;
		break;
	case 21:
	case 22:
	case 23:
	case 24:
		returnVal += 1;
	default:
		returnVal = 0;
	}
	return returnVal;
}
