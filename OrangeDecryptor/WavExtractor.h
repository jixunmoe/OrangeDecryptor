#pragma once
#include "stdafx.h"
#include "Helper.h"

class WavExtractor
{
public:
	WavExtractor(TCHAR* lpInputPath, TCHAR* lpOutputDirectory);
	~WavExtractor();

	void Extract();

private:
	std::shared_ptr<File> m_source;
	TCHAR* m_lpOutputDirectory;
};

