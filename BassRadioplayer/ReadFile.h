#pragma once

class ReadFile
{
public:
	static bool readFile(CFile& file, std::wstring& content);
	static bool readUTF8OrANSI(CFile& file, std::wstring& content);
	static bool readANSI(CFile& file, std::wstring& content);
	static bool readUTF8(CFile& file, std::wstring& content, bool skipBOM = true);
	static bool readUnicode(CFile& file, std::wstring& content);
	static bool readUnicodeBigEndian(CFile& file, std::wstring& content);
	static const ULONGLONG FILE_SIZE_LIMIT;
};


