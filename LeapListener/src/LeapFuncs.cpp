#include "LeapFuncs.h"
#include <iostream>
#include <map>
#include <vector>
#include <Windows.h>
#include <tchar.h>
#define _USE_MATH_DEFINES
#include <math.h>

namespace LeapFuncs
{
	struct HandInfo
	{
		LEAP_VECTOR storedPos;
	};

	static std::map<uint32_t, HandInfo> g_handsTracked;
	static UINT _gesturesToKeyMappings[LeapGesture::NUM_GESTURES];
	static uint64_t gestureTimeLimit = 800000; // 0.8 sec, since LeapGetNow() deals with ns
	static float angularThreshold = 1.0f;

	void SetGestureTimeLimit(float tl_sec)
	{
		gestureTimeLimit = uint64_t(tl_sec * 1000000);
	}

	void SetAngularThreshold(float angle)
	{
		angularThreshold = float(std::tan(angle * (M_PI / 180.0)));
	}

	wchar_t const* GetGestureString(LeapGesture gst)
	{
		static wchar_t const* names[LeapGesture::NUM_GESTURES] = 
		{
			L"None",
			L"Swipe Left",
			L"Swipe Right"
		};

		return names[gst];
	}

	HandInfo* GetTrackedHand(uint32_t id)
	{
		auto it = g_handsTracked.find(id);
		if (it != g_handsTracked.end())
		{
			return &(it->second);
		}
		return nullptr;
	}

	inline bool IdIsInFrame(uint32_t id, uint32_t numHands, LEAP_HAND* hands)
	{
		for (uint32_t hi = 0; hi < numHands; ++hi)
		{
			if (hands[hi].id == id)
			{
				return true;
			}
		}
		return false;
	}

	inline void CleanupTrackedhands(uint32_t numHands, LEAP_HAND* hands)
	{
		std::vector<uint32_t> keys(g_handsTracked.size());
		for (auto& ht : g_handsTracked)
		{
			keys.push_back(ht.first);
		}
		for (auto& k : keys)
		{
			if (!IdIsInFrame(k,numHands,hands))
			{
				g_handsTracked.erase(k);
			}
		}
	}

	inline void DetermineGesture(HandInfo* hInfo, LEAP_HAND* frameHand)
	{
		LEAP_VECTOR* refPalmPos = &(hInfo->storedPos);
		float refTan = refPalmPos->x / refPalmPos->y;

		LEAP_VECTOR* palmPos = &(frameHand->palm.position);
		float tan = palmPos->x / palmPos->y;

		//std::cout << frameHand->visible_time << std::endl;
		if ((tan - refTan) < -angularThreshold)
		{
			SendGesture(LeapGesture::SWIPE_LEFT);
			hInfo->storedPos = *palmPos;
		}
		else if ((tan - refTan) > angularThreshold)
		{
			SendGesture(LeapGesture::SWIPE_RIGHT);
			hInfo->storedPos = *palmPos;
		}
	}

	void HandleHands(uint32_t numHands, LEAP_HAND* hands)
	{
		LEAP_HAND* currHand = nullptr;
		
		CleanupTrackedhands(numHands, hands);
		for (uint32_t h = 0; h < numHands; ++h)
		{
			currHand = &(hands[h]);
			LEAP_VECTOR* palmPos = &(currHand->palm.position);
			float tan = palmPos->x / palmPos->y;

			HandInfo* trackedHand = GetTrackedHand(currHand->id);
			if (trackedHand)
			{
				DetermineGesture(trackedHand, currHand);
			}
			else
			{
				HandInfo hi;
				hi.storedPos = *palmPos;
				g_handsTracked[currHand->id] = hi;
			}
		}
	}

	void HandleEvent(LEAP_CONNECTION_MESSAGE const* msg)
	{
		switch (msg->type)
		{
			case eLeapEventType_Tracking:
			{
				HandleHands(msg->tracking_event->nHands, msg->tracking_event->pHands);
				break;
			}
			case eLeapEventType_Connection:
			{
				std::cout << "Connection started" << std::endl;
				break;
			}
			case eLeapEventType_Device:
			{
				std::cout << "Device connected" << std::endl;
				break;
			}
			case eLeapEventType_DeviceLost:
			{
				std::cout << "Device disconnected" << std::endl;
				break;
			}
			case eLeapEventType_ConnectionLost:
			{
				std::cout << "Connection lost" << std::endl;
				break;
			}
		}
	}

	void OutputErrorMessage(char const* msg, eLeapRS error)
	{
		switch (error)
		{
			case eLeapRS_CannotOpenDevice:
			{
				std::cout << msg << " : Cannot open device" << std::endl;
				break;
			}
			case eLeapRS_NotConnected:
			{
				std::cout << msg << " : Device not connected" << std::endl;
				break;
			}
			case eLeapRS_NotAvailable:
			{
				std::cout << msg << " : Not available" << std::endl;
				break;
			}
			case eLeapRS_NotStreaming:
			{
				std::cout << msg << " : Not streaming" << std::endl;
				break;
			}
		}
	}

	void  InitContext()
	{
		_gesturesToKeyMappings[LeapGesture::NONE] = 0;
		_gesturesToKeyMappings[LeapGesture::SWIPE_LEFT] = VK_RIGHT;
		_gesturesToKeyMappings[LeapGesture::SWIPE_RIGHT] = VK_LEFT;

		SetGestureTimeLimit(0.8f);
		SetAngularThreshold(45.0f);
	}

	void SendGesture(LeapGesture gst)
	{
		static uint64_t lastTime = LeapGetNow();
		uint64_t now = LeapGetNow();
		if (now - lastTime < gestureTimeLimit)
		{
			return;
		}
		lastTime = now;
		auto hwnd = GetForegroundWindow();
		std::wcout << GetGestureString(gst) << std::endl;
		if (hwnd != NULL)
		{
			BOOL result = PostMessage(hwnd, WM_KEYDOWN, _gesturesToKeyMappings[gst], MapVirtualKey(_gesturesToKeyMappings[gst], MAPVK_VK_TO_VSC));
		}
	}
}
