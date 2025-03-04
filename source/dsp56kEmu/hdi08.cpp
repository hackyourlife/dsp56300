#include "dsp.h"
#include "interrupts.h"
#include "hdi08.h"

namespace dsp56k
{
	void HDI08::exec()
	{
		if (!bittest(m_hpcr, HPCR_HEN)) 
			return;

		if (m_pendingRXInterrupts > 0 && bittest(m_hcr, HCR_HRIE))
		{
			--m_pendingRXInterrupts;
			m_periph.getDSP().injectInterrupt(Vba_Host_Receive_Data_Full);
		}
		else if (bittest(m_hcr, HCR_HTIE))
		{
			m_periph.getDSP().injectInterrupt(Vba_Host_Transmit_Data_Empty);
		}
	}

	TWord HDI08::readRX()
	{
		if (m_data.empty()) {
			LOG("Empty read");
			return 0;
		}

		return m_data.pop_front() & 0xFFFFFF;
	}

	void HDI08::writeRX(const int32_t* _data, const size_t _count)
	{
		for (size_t i = 0; i < _count; ++i)
		{
			m_data.waitNotFull();
			m_data.push_back(_data[i] & 0x00ffffff);
			if (bittest(m_hpcr, HPCR_HEN) && bittest(m_hcr, HCR_HRIE)) {
				++m_pendingRXInterrupts;
			}
		}
	}

	void HDI08::clearRX()
	{
		m_data.clear();
	}

	void HDI08::setHostFlags(const char _flag0, const char _flag1)
	{
		dsp56k::bitset<TWord, HSR_HF0>(m_hsr, _flag0);
		dsp56k::bitset<TWord, HSR_HF1>(m_hsr, _flag1);
		LOG("Write HostFlags, HSR " << HEX(m_hsr));
	}

	uint32_t HDI08::readTX()
	{
		m_dataTX.waitNotEmpty();
		return m_dataTX.pop_front();
	}

	void HDI08::writeTX(TWord _val)
	{
		m_dataTX.waitNotFull();
		m_dataTX.push_back(_val);
		//LOG("Write HDI08 HOTX " << HEX(_val));
	}

	void HDI08::writeControlRegister(TWord _val)
	{
		//LOG("Write HDI08 HCR " << HEX(_val));
		m_hcr = _val;
	}
};
