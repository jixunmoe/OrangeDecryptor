#pragma once
#include "stdafx.h"
#include <memory>
#include <sstream>
#include <vector>

template<class T>
bool HasEnding(T const &fullString, T const &ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	else {
		return false;
	}
}

inline void A2W(const char* input, std::wstring& output)
{
	const auto size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input, -1, nullptr, 0);
	const auto buffer = new TCHAR[size];
	MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input, -1, buffer, size);
	output.assign(buffer);
}

class File
{
public:
	File(HANDLE hFile)
	{
		m_hFile = hFile;
		m_eof = false;
	}

	~File()
	{
		Close();
	}

	bool Opened() const
	{
		return m_hFile != INVALID_HANDLE_VALUE;
	}

	void Close()
	{
		if (Opened())
		{
			CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
		}
	}

	void MoveTo(int64_t position, DWORD method = FILE_BEGIN)
	{
		LARGE_INTEGER pos;
		pos.QuadPart = position;
		SetFilePointerEx(m_hFile, pos, nullptr, FILE_CURRENT);
	}

	void Forward(int64_t bytes)
	{
		MoveTo(bytes, FILE_CURRENT);
	}

	void Backward(int64_t bytes)
	{
		Forward(-bytes);
	}

	template<typename T>
	T Read()
	{
		T szBuffer;
		Read(&szBuffer, sizeof(T));
		return szBuffer;
	}

	DWORD Read(LPVOID buffer, size_t size)
	{
		DWORD bytesRead;
		ReadFile(m_hFile, buffer, size, &bytesRead, nullptr);
		SetEndOfFile(bytesRead == 0);
		return bytesRead;
	}

	void ReadNullTerminated(std::string& str)
	{
		std::stringstream ss;
		while(true)
		{
			const char c = ReadByte();
			if (c == 0 || IsEndOfFile()) break;
			ss << c;
		}

		str.assign(ss.str());
	}

	void Write(LPVOID buffer, size_t size) const
	{
		DWORD bytesWritten;
		WriteFile(m_hFile, buffer, size, &bytesWritten, nullptr);
	}

	void Write(const char* buffer, size_t size) const
	{
		DWORD bytesWritten;
		WriteFile(m_hFile, buffer, size, &bytesWritten, nullptr);
	}

	template<typename T>
	void Write(T buffer)
	{
		Write(&buffer, sizeof(T));
	}

	template<typename T>
	void Write(std::vector<T> buffer)
	{
		Write(buffer.begin(), buffer.size());
	}

	bool FindU32(uint32_t expected)
	{
		while (Read<uint32_t>() != expected)
		{
			Read<uint8_t>();
			if (IsEndOfFile()) return false;
			Backward(sizeof(uint32_t));
		}

		Backward(sizeof(uint32_t));
		return true;
	}

	bool IsEndOfFile()
	{
		return m_eof;
	}

	void SetEndOfFile(bool isEndOfFile)
	{
		m_eof = isEndOfFile;
	}

	uint8_t ReadByte()
	{
		uint8_t szBuffer[1];
		DWORD bytesRead;
		ReadFile(m_hFile, szBuffer, 1, &bytesRead, nullptr);
		SetEndOfFile(bytesRead == 0);
		return szBuffer[0];
	}

	static std::shared_ptr<File> CreateReader(const TCHAR* lpPath)
	{
		return std::make_shared<File>(CreateFile(lpPath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
	}

	static std::shared_ptr<File> CreateWriter(const TCHAR* lpPath)
	{
		return std::make_shared<File>(CreateFile(lpPath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr));
	}

private:
	bool m_eof;
	HANDLE m_hFile;
};

class LogProgress
{
public:
	static void UpdateProgress(const std::wstring& str)
	{
		auto hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(hConsole, &info);
		wprintf(L"%s    ", str.c_str());
		fflush(stdout);
		SetConsoleCursorPosition(hConsole, info.dwCursorPosition);
	}

	static void Done()
	{
		printf("Done.");
		for (int i = 0; i < 40; i++)
		{
			printf(" ");
		}
		printf("\n");
	}
};