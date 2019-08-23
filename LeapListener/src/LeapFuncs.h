#pragma once
#include <LeapC.h>

namespace LeapFuncs
{
	enum LeapGesture
	{
		NONE,
		SWIPE_LEFT,
		SWIPE_RIGHT,

		NUM_GESTURES
	};

	void SetGestureTimeLimit(float tl_sec);
	void SetAngularThreshold(float angle);
	void OutputErrorMessage(char const* msg, eLeapRS error);
	void InitContext();
	void HandleEvent(LEAP_CONNECTION_MESSAGE const* msg);
	void SendGesture(LeapGesture gst);
}
