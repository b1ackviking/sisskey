#pragma once

#include <chrono>

namespace sisskey
{
	class Timer
	{
	private:
		using tp = std::chrono::time_point<std::chrono::high_resolution_clock>;
		tp m_BaseTime;
		tp m_CurrTime;
		tp m_PrevTime;
		tp m_StopTime;

		float m_PausedTime{ 0.0f };
		bool m_Stopped{ false };

	public:
		Timer() { Reset(); }
		// TODO: move constructors??

		void Stop();
		void Start();
		void Reset();
		float Tick();

		float TotalTime();
	};
}