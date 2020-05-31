#pragma once
#include "stdafx.h"
#include "DatDecryptor.h"
#include "OggExtractor.h"
#include "ZipExtractor.h"
#include "WavExtractor.h"

#define BUFFER_SIZE (1 * 1024 * 1024)

inline void DoDataDecryption(TCHAR* input, TCHAR* output)
{
	const auto reader = File::CreateReader(input);
	const auto writer = File::CreateWriter(output);

	const auto szBuffer = new unsigned char[BUFFER_SIZE];
	auto decryptor = new DatDecryptor();

	while (true)
	{
		const DWORD bytesRead = reader->Read(szBuffer, BUFFER_SIZE);

		// eof 仅当读入字节为 0 的情况时产生
		if (reader->IsEndOfFile()) break;

		decryptor->DecryptBuffer(szBuffer, size_t{ bytesRead });

		writer->Write(szBuffer, bytesRead);
	}

	delete[] szBuffer;
	delete decryptor;
}

inline void DoOggExtraction(TCHAR* input, TCHAR* output)
{
	OggDecryptor * ogg = new OggDecryptor(input, output);
	ogg->Extract();
	delete ogg;
}

inline void DoUnzipAndDataDecryption(TCHAR* input, TCHAR* output)
{
	ZipExtractor * zip = new ZipExtractor(input, output);
	zip->Extract();
	delete zip;
}

inline void DoWavExtraction(TCHAR* input, TCHAR* output)
{
	WavExtractor * wav = new WavExtractor(input, output);
	wav->Extract();
	delete wav;
}
