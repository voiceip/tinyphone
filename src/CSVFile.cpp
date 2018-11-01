#include "StdAfx.h"

#include "CSVFile.h"

CCSVFile::CCSVFile():
	CStdioFileEx()
{
}

bool CCSVFile::ReadData(CStringArray &arr)
{
	ULONGLONG pos = GetPosition();

	// Read next line
	CString sLine;
	if (!ReadString(sLine))
		return false;
	sLine.Trim();
	LPCTSTR p = sLine;
	
	int nValue = 0;

	// Parse values in this line

	while (*p != '\0')
	{
		CString s;  // String to hold this value

		if (*p == '"')
		{
			// Bump past opening quote
			p++;
			// Parse quoted value
			while (*p != '\0')
			{
				// Test for quote character
				if (*p == '"')
				{
					// Found one quote
					p++;
					// If pair of quotes, keep one
					// Else interpret as end of value
					if (*p != '"')
					{
						p++;
						break;
					}
				}
				// Add this character to value
				s.AppendChar(*p++);
			}
		}
		else
		{
			// Parse unquoted value
			while (*p != '\0' && *p != ',')
			{
				s.AppendChar(*p++);
			}
			// Advance to next character (if not already end of string)
			if (*p != '\0')
				p++;
		}
		// Add this string to value array
		if (nValue < arr.GetCount())
			arr[nValue] = s;
		else
			arr.Add(s);
		nValue++;
	}
	// Trim off any unused array values
	if (arr.GetCount() > nValue)
		arr.RemoveAt(nValue, arr.GetCount() - nValue);
	// We return true if ReadString() succeeded--even if no values
	return true;
}

void CCSVFile::WriteData(CStringArray &arr)
{
	static TCHAR chQuote = '"';
	static TCHAR chComma = ',';

	// Loop through each string in array
	for (int i = 0; i < arr.GetCount(); i++)
	{
		// Separate this value from previous
		if (i > 0)
			WriteString(_T(","));
		// We need special handling if string contains
		// comma or double quote
		bool bComma = (arr[i].Find(chComma) != -1);
		bool bQuote = (arr[i].Find(chQuote) != -1);
		if (bComma || bQuote)
		{
			Write(&chQuote, sizeof(TCHAR));
			if (bQuote)
			{
				for (int j = 0; j < arr[i].GetLength(); i++)
				{
					// Pairs of quotes interpreted as single quote
					if (arr[i][j] == chQuote)
						Write(&chQuote, sizeof(TCHAR));
					TCHAR ch = arr[i][j];
					Write(&ch, sizeof(TCHAR));
				}
			}
			else
			{
				WriteString(arr[i]);
			}
			Write(&chQuote, sizeof(TCHAR));
		}
		else
		{
			WriteString(arr[i]);
		}
	}
	WriteString(_T("\n"));
}
