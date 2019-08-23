#include <iostream>
#include "LeapFuncs.h"

static LEAP_CONNECTION connection;

namespace LeapFuncs
{
	extern float horzSwipeSensitivity;
}

void Closed()
{
	LeapDestroyConnection(connection);
}

int main(int argc, char* argv[])
{
	atexit(Closed);
	LeapFuncs::InitContext();
	if (argc > 1)
	{
		LeapFuncs::SetAngularThreshold(float(atof(argv[1])));
	}
	if (argc > 2)
	{
		LeapFuncs::SetGestureTimeLimit(float(atof(argv[2])));
	}

	connection = {};
	eLeapRS result = LeapCreateConnection(nullptr, &connection);
	if (result != eLeapRS_Success)
	{
		LeapFuncs::OutputErrorMessage("LeapCreateConnection", result);
		return 0;
	}
	result = LeapOpenConnection(connection);
	if (result != eLeapRS_Success)
	{
		LeapFuncs::OutputErrorMessage("LeapOpenConnection", result);
	}

	{
		LEAP_CONNECTION_MESSAGE msg;
		while (true)
		{
			result = LeapPollConnection(connection, 0, &msg);
			LeapFuncs::HandleEvent(&msg);
		}
	}

	return 0;
}
