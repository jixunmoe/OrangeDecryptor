#pragma once

extern const unsigned char szDecryptVector[];

class DatDecryptor
{
public:
	DatDecryptor();
	void Reset();
	void DecryptBuffer(LPBYTE lpBuffer, int64_t bufferSize);

private:
	BYTE GetNextKey();
	size_t m_index;
};