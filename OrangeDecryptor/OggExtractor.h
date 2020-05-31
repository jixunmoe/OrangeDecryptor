#pragma once
#include "stdafx.h"
#include "Helper.h"

// 只有网络用?
#define OGG_HEADER_TYPE_FRESH_PACKET (1 << 0)

// 包头
#define OGG_HEADER_TYPE_FIRST_PACKET (1 << 1)
#define OGG_HEADER_TYPE_LAST_PACKET  (1 << 2)

#define OGG_MAGIC_OGGS (0x5367674F)

#define MAX_OGG_SEGMENT_LENGTH (0xFF)
#define MAX_OGG_SEGMENTS_COUNT (0xFF)

#include <pshpack1.h>
struct OggPageHeader
{
	DWORD CapturePattern; // 'OggS', 0x5367674F

	uint8_t Version;
	uint8_t HeaderType;

	uint64_t GranulePosition;
	uint32_t BitStreamSerial;
	uint32_t PageSequenceNumber;
	uint32_t Checksum;

	uint8_t PageSegments;

	// uint8_t SengmentsLength[PageSegments];

	// char Segment[SengmentsLength[0]]
	// ...

	bool IsNewPage() const
	{
		return PageSequenceNumber == 0 && (HeaderType & OGG_HEADER_TYPE_FIRST_PACKET) != 0;
	}
};
#include <poppack.h>


class OggDecryptor
{
public:
	OggDecryptor(TCHAR* lpInputPath, TCHAR* lpOutputDirectory);
	~OggDecryptor();

	void Extract();

private:
	void NextFile();

	std::shared_ptr<File> m_source;
	std::shared_ptr<File> m_writer;
	TCHAR* m_lpOutputDirectory;
	size_t m_fileNumber = 0;
};

