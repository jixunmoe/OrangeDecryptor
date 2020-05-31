#include "stdafx.h"
#include "OggExtractor.h"
#include <string>
#include <sstream>
#include <iomanip>

OggDecryptor::OggDecryptor(TCHAR* lpInputPath, TCHAR* lpOutputDirectory)
{
	m_source = File::CreateReader(lpInputPath);
	m_lpOutputDirectory = lpOutputDirectory;
	CreateDirectory(lpOutputDirectory, nullptr);
}

OggDecryptor::~OggDecryptor()
{
	if (m_source && m_source->Opened())
		m_source->Close();

	if (m_writer && m_writer->Opened())
		m_writer->Close();
}

void OggDecryptor::Extract()
{
	OggPageHeader pageHeader;
	uint8_t segmentLengths[MAX_OGG_SEGMENTS_COUNT];
	uint8_t segment[MAX_OGG_SEGMENT_LENGTH];

	while(m_source->FindU32(OGG_MAGIC_OGGS))
	{
		// printf("Dumping file (%d)\n", m_fileNumber);
		if (m_source->IsEndOfFile()) break;

		m_source->Read(&pageHeader, sizeof(pageHeader));
		const auto segmentsCount = pageHeader.PageSegments;

		if (pageHeader.IsNewPage())
		{
			NextFile();
		} else if (!m_writer && !m_writer->Opened())
		{
			printf("WARN: Expecting new header, discard this page...\n");
		}

		m_source->Read(segmentLengths, segmentsCount);

		m_writer->Write(&pageHeader, sizeof(pageHeader));
		m_writer->Write(segmentLengths, segmentsCount);

		// 处理各个区段
		for (int i = 0; i < segmentsCount; i++)
		{
			// printf("Dumping segment %d/%d...", i, segmentsCount);
			auto size = segmentLengths[i];
			m_source->Read(segment, size);
			m_writer->Write(segment, size);
			// printf("ok!\n");
		}
	}

	LogProgress::Done();
}

void OggDecryptor::NextFile()
{
	if (m_writer && m_writer->Opened())
		m_writer->Close();

	m_fileNumber++;


	std::wstringstream outputFileName;
	outputFileName
		<< std::setw(3) << std::setfill(L'0') << m_fileNumber
		<< L".ogg";

	std::wstringstream outputFile;
	outputFile << m_lpOutputDirectory << L"\\" << outputFileName.str();


	std::wstringstream outputLog;
	outputLog << L"Dumping " << outputFileName.str() << "...";
	LogProgress::UpdateProgress(outputLog.str().c_str());
	m_writer = File::CreateWriter(outputFile.str().c_str());
}
