#include "Timer.h"

namespace sisskey
{
	float Timer::Tick(void)
	{
		if (m_Stopped)
			return 0.0f;

		m_CurrTime = std::chrono::high_resolution_clock::now();

		float DeltaTime = std::chrono::duration<float>(m_CurrTime - m_PrevTime).count();

		m_PrevTime = m_CurrTime;

		// Force clamp the value??
		// return std::clamp(DeltaTime, 0.0f, 0.0003f);
		return DeltaTime;
	}

	void Timer::Stop()
	{
		if (!m_Stopped)
		{
			m_StopTime = std::chrono::high_resolution_clock::now();
			m_Stopped = true;
		}
	}

	void Timer::Start()
	{
		if (m_Stopped)
		{
			m_CurrTime = m_PrevTime = std::chrono::high_resolution_clock::now();

			m_PausedTime += std::chrono::duration<float>(m_CurrTime - m_StopTime).count();

			m_Stopped = false;
		}
	}

	void Timer::Reset()
	{
		m_BaseTime = m_CurrTime = m_PrevTime = std::chrono::high_resolution_clock::now();
		m_PausedTime = 0.0f;
		m_Stopped = false;
	}

	float Timer::TotalTime()
	{
		if (m_Stopped)
			return std::chrono::duration<float>(m_StopTime - m_BaseTime).count() - m_PausedTime;
		else
			return std::chrono::duration<float>(m_CurrTime - m_BaseTime).count() - m_PausedTime;
	}
}