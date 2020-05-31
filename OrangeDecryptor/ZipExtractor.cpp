#include "stdafx.h"
#include "ZipExtractor.h"
#include "DatDecryptor.h"
#include "Helper.h"

#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

#define ZIP_BUFFER_SIZE (512 * 1024)

ZipExtractor::ZipExtractor(TCHAR* lpInputPath, TCHAR* lpOutputDirectory)
	:m_zipSource(nullptr), m_zip(nullptr)
{
	zip_error_t error;
	m_zipSource = zip_source_win32w_create(lpInputPath, 0, 0, &error);
	if (!m_zipSource)
	{
		zip_error_strerror(&error);
		printf("Failed to open zip file as source with error %s\n", error.str);
		zip_error_fini(&error);
		return;
	}
	m_zip = zip_open_from_source(m_zipSource, ZIP_CHECKCONS | ZIP_RDONLY, &error);
	if (!m_zipSource)
	{
		zip_error_strerror(&error);
		printf("Failed to open zip source with error %s\n", error.str);
		zip_error_fini(&error);
		return;
	}

	CreateDirectory(lpOutputDirectory, nullptr);
	m_lpOutputDirectory = lpOutputDirectory;
}

ZipExtractor::~ZipExtractor()
{
	if (m_zip) zip_close(m_zip);
	if (m_zipSource) zip_source_close(m_zipSource);
}

void ZipExtractor::Extract()
{
	const auto szBuffer = new unsigned char[ZIP_BUFFER_SIZE];
	auto decryptor = new DatDecryptor();

	auto entries = zip_get_num_entries(m_zip, ZIP_FL_UNCHANGED);
	int fill_char = static_cast<int>(std::ceill(log10(entries)));

	std::string fileName;
	std::wstring filenameUnicode;
	std::wstring ext(L"");
	for (int64_t i = 0; i < entries; i++)
	{
		fileName.assign(zip_get_name(m_zip, i, ZIP_FL_ENC_RAW));
		A2W(fileName.c_str(), filenameUnicode);

		std::wstringstream outputFile;
		outputFile << m_lpOutputDirectory << L"\\" << filenameUnicode;

		zip_file_t* file = zip_fopen_index(m_zip, i, NULL);
		decryptor->Reset();

		if (HasEnding<std::string>(fileName, "/"))
		{
			CreateDirectory(outputFile.str().c_str(), nullptr);
			continue;
		}

		std::wstring nameLower(filenameUnicode);
		std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
		const bool doDecrypt = HasEnding<std::wstring>(nameLower, L".dat");

		if (doDecrypt)
		{
			// 探测文件类型
			if (zip_fread(file, szBuffer, 4) != 4)
			{
				printf("File %s is less than 4 bytes, skip...\n", fileName.c_str());
				continue;
			}

			decryptor->DecryptBuffer(szBuffer, 4);
			const auto fileMagic = LPDWORD(LPVOID(szBuffer))[0];
			if (fileMagic == 0x474E5089)
			{
				ext.assign(L".png");
			}
			else if (fileMagic == 0x20534444)
			{
				ext.assign(L".dds");
			}
			else
			{
				ext.assign(L"");
				printf("Unknown magic: 0x%08x (%s)\n", fileMagic, fileName.c_str());
			}
			outputFile << ext;
		}

		const auto writer = File::CreateWriter(outputFile.str().c_str());

		// 因为探测过才需要写出之前读的 4 个字节备份
		if (doDecrypt) writer->Write(szBuffer, 4);

		std::wstringstream progress;
		progress
			<< (doDecrypt ? L"Decrypting " : L"Extracting ")
			<< "[" << std::setw(fill_char) << i << "/" << entries << "] "
			<< filenameUnicode << ext << "...   ";
		LogProgress::UpdateProgress(progress.str());

		while(true)
		{
			const auto bytesRead = zip_fread(file, szBuffer, ZIP_BUFFER_SIZE);
			if (bytesRead == 0) break;

			if (doDecrypt) decryptor->DecryptBuffer(szBuffer, bytesRead);

			writer->Write(szBuffer, static_cast<size_t>(bytesRead));
		}

		zip_fclose(file);
	}

	LogProgress::Done();

	delete[] szBuffer;
	delete decryptor;
}
