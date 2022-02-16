#pragma once

struct InputData
{
	//struct to hold data from inputs
	// prefixes:
	// k = keyboard
	// c = controller
	// m = mouse
	float kSpace, kShift, cLTrigger, cRTrigger; // up / down
	float kW, kS, cLYAxis; // forwards / backwards
	float kA, kD, cLXAxis; // right / left
	float mY, mX, cRYAxis, cRXAxis; // rotation
	float kCtrl, kO; // open file
	unsigned winHeight, winWidth; // window height and width
	float winFOV, winAR;  //fov / aspect ratio
};