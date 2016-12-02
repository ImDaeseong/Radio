#include "stdafx.h"
#include "ReadFile.h"

const ULONGLONG ReadFile::FILE_SIZE_LIMIT = 20 * (1 << 20);

bool ReadFile::readFile(CFile& file, wstring& content)
{
	try
	{		
		static const WORD SIG_UNICODE = 0xFEFF; // Unicode (little endian)
		static const WORD SIG_UNICODE_BIG_ENDIAN = 0xFFFE; // Unicode big endian
		static const DWORD SIG_UTF8 = 0xBFBBEF; /*EF BB BF*/ // UTF-8
		
		WORD signature = 0;
		if (file.GetLength() >= 2)
		{
			file.Read(&signature, 2);
			file.SeekToBegin();
		}
		
		bool success = false;
		if (signature == SIG_UNICODE)
			success = readUnicode(file, content);
		else if (signature == SIG_UNICODE_BIG_ENDIAN)
			success = readUnicodeBigEndian(file, content);
		else if(signature == (SIG_UTF8 & 0xffff))
		{
			if (file.GetLength() >= 3) 
			{
				DWORD sig = 0;
				file.Read(&sig, 3);
				file.SeekToBegin();
				if (sig == SIG_UTF8)
					success = readUTF8(file, content);
				else
				{
					success = readUTF8OrANSI(file, content);
				}
			} else success = readUTF8OrANSI(file, content);
		}
		else 
			success = readUTF8OrANSI(file, content);
		
		return success;
	}
	catch (...)
	{	
		return false;
	}
	return false;
}

bool ReadFile::readUTF8OrANSI(CFile& file, wstring& content)
{
	bool success = readUTF8(file, content, false);
	if (!success)
	{
		file.SeekToBegin();
		success = readANSI(file, content);
	}
	return success;
}

bool ReadFile::readANSI(CFile& file, wstring& content)
{
	ULONGLONG contentLength = file.GetLength();
	if (contentLength == 0)
	{
		content = L"";
		return true;
	}

	if (contentLength > FILE_SIZE_LIMIT) contentLength = FILE_SIZE_LIMIT;
	char* buffer = new char[(size_t)contentLength];
	
	file.Read(buffer, (unsigned int)contentLength);

	int newContentLength = MultiByteToWideChar(CP_ACP, 0, buffer, (int)contentLength, NULL, 0);
	if (!newContentLength)
	{
		delete[] buffer;
		return false;
	}
	content.resize(newContentLength);

	int size = MultiByteToWideChar(CP_ACP, 0, buffer, (int)contentLength, &content[0], newContentLength);
	delete[] buffer;
	return 0 != size;
}

bool ReadFile::readUTF8(CFile& file, wstring& content, bool skipBOM)
{
	ULONGLONG contentLength = (file.GetLength() - (skipBOM ? 3 : 0));
	if (contentLength == 0)
	{
		content = L"";
		return true;
	}

	if (contentLength > FILE_SIZE_LIMIT) contentLength = FILE_SIZE_LIMIT;
	if (skipBOM) file.Seek(3, CFile::begin);

	char* buffer = new char[(size_t)contentLength];
	file.Read(buffer, (unsigned int)contentLength);

	int newContentLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, buffer, (int)contentLength, NULL, 0);
	if (!newContentLength)
	{
		delete[] buffer;
		return false;
	}

	content.resize(newContentLength);
	int size = MultiByteToWideChar(CP_UTF8, 0, buffer, (int)contentLength, &content[0], newContentLength);
	delete[] buffer;
	return 0 != size;
}

bool ReadFile::readUnicode(CFile& file, wstring& content)
{
	_STATIC_ASSERT(sizeof(wchar_t) == 2); // required by UTF16

	ULONGLONG contentLength = (file.GetLength() - 2) / sizeof(wchar_t);
	if (contentLength <= 0)
	{
		content = L"";
		return true;
	}

	if (contentLength > FILE_SIZE_LIMIT) contentLength = FILE_SIZE_LIMIT;
	content.resize((size_t)contentLength);

	file.Seek(2, CFile::begin);
	file.Read(&content[0], (unsigned int)(contentLength * sizeof(wchar_t)));
	return true;
}

bool ReadFile::readUnicodeBigEndian(CFile& file, wstring& content)
{
	_STATIC_ASSERT(sizeof(wchar_t) == 2); // required by UTF16

	ULONGLONG contentLength = (file.GetLength() - 2) / sizeof(wchar_t);
	if (contentLength <= 0)
	{
		content = L"";
		return true;
	}
	
	if ( contentLength > FILE_SIZE_LIMIT ) contentLength = FILE_SIZE_LIMIT;
	content.resize((size_t)contentLength);
	
	file.Seek(2, CFile::begin);
	file.Read(&content[0], (unsigned int)(contentLength * sizeof(wchar_t)));

	for (int i = 0; i < (int)contentLength; i++)
	{
		unsigned int ch = content[i];
		content[i] = (wchar_t)((ch >> 8) | (ch << 8));
	}
	return true;
}