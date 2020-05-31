#include "stdafx.h"
#include "WavExtractor.h"
#include "Helper.h"
#include <sstream>



WavExtractor::WavExtractor(TCHAR* lpInputPath, TCHAR* lpOutputDirectory)
{
	m_source = File::CreateReader(lpInputPath);
	m_lpOutputDirectory = lpOutputDirectory;
	CreateDirectory(lpOutputDirectory, nullptr);
}


WavExtractor::~WavExtractor()
{
	m_source->Close();
}

#define WAV_BUFFER_SIZE (2 * 1024 * 1024)

void WavExtractor::Extract()
{
	const auto lpWavBuffer = new char[WAV_BUFFER_SIZE];

	while(!m_source->IsEndOfFile())
	{
		const auto dataSize = m_source->Read<uint32_t>();
		std::stringstream ss;

		while(true)
		{
			const char c = m_source->Read<char>();
			if (c == 0 || m_source->IsEndOfFile()) break;
			ss << c;
		}

		if (m_source->IsEndOfFile()) break;

		std::string file(ss.str());
		std::wstring fileW;
		A2W(file.c_str(), fileW);

		const auto fmtSize = m_source->Read<uint32_t>();
		if (fmtSize != 0x12)
		{
			printf("File %s contains invalid format size.\n", file.c_str());
			break;
		}

		{
			std::wstringstream progress;
			progress << "Extracting " << fileW << "...";
			LogProgress::UpdateProgress(progress.str().c_str());
		}

		std::wstringstream outputPath;
		outputPath << m_lpOutputDirectory << "\\" << fileW;
		auto writer = File::CreateWriter(outputPath.str().c_str());

		// Header
		writer->Write("RIFF", 4);
		writer->Write<uint32_t>(4 + 24 + (8 + dataSize));

		// Start (4)
		writer->Write("WAVE", 4);

		// Format Chunk (4 + 4 + 16 = 24)
		writer->Write("fmt ", 4);
		writer->Write<uint32_t>(16);

		// 结构体最后两个字节是 WAVEFORMATEX 的大小，因此舍弃
		m_source->Read(lpWavBuffer, 0x10);
		writer->Write(lpWavBuffer, 0x10);
		m_source->Read<uint16_t>();

		// Data Chunk (4 + 4 + dataSize = 8 + dataSize)
		writer->Write("data", 4);
		writer->Write<uint32_t>(dataSize);

		auto bytesLeft = dataSize;
		while(bytesLeft > 0)
		{
			if (m_source->IsEndOfFile())
			{
				printf("Unexpected EOF when reading %s.\n", file.c_str());
				break;
			}

			const auto toRead = min(bytesLeft, WAV_BUFFER_SIZE);
			const auto bytesRead = m_source->Read(lpWavBuffer, toRead);
			writer->Write(lpWavBuffer, bytesRead);
			bytesLeft -= bytesRead;
		}

		writer->Close();
	}

	delete[] lpWavBuffer;

	LogProgress::Done();
}
