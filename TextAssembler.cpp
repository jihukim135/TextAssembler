#pragma warning(disable: 4996)

#include <tchar.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define WRITE_LINE_BY_LINE
#define MAX_LINES 300

#define LOCATION_OPERATOR 11
#define LOCATION_STORAGE 8
#define LOCATION_OPERAND_LEFT 4
#define LOCATION_OPERAND_RIGHT 0

#define LOCATION_MODE 14
#define LOCATION_REGISTER 8
#define LOCATION_MAINMEMORY 0

#define MAX_TOKEN_LENGTH 15
#define MAX_TOKEN_STRING_LENGTH 300

typedef int16_t INSTRUCTION;

typedef struct _TOKEN_BASIC
{
	TCHAR name[MAX_TOKEN_LENGTH];
	INSTRUCTION binary;
} TOKEN_BASIC;

typedef struct _TOKEN_STRING
{
	TCHAR name[MAX_TOKEN_LENGTH];
	TCHAR str[MAX_TOKEN_STRING_LENGTH];
	DWORD numOfLines;
} TOKEN_STRING;

INSTRUCTION indirectMode = 0b11;

TOKEN_BASIC mathOps[] =
{
	{_T("ADD"), 0b001}, {_T("SUB"), 0b010}, {_T("MUL"), 0b011}, {_T("DIV"), 0b100},
};

TOKEN_BASIC loadStoreOps[] =
{
	{_T("LOAD"), 0b110}, {_T("STORE"), 0b111},
};

TOKEN_BASIC registers[] =
{
	{_T("r0"), 0b000}, {_T("r1"), 0b001}, {_T("r2"), 0b010}, {_T("r3"), 0b011},
	{_T("ir"), 0b100}, {_T("sp"), 0b101}, {_T("lr"), 0b110}, {_T("pc"), 0b111},
};

TOKEN_STRING pushOp =
{
	_T("PUSH"),
	_T("ADD r1, %s, 0\n")
	_T("STORE sp, 0x40\n")
	_T("STORE r1, [0x40]\n")
	_T("ADD sp, sp, 1\n"),
	4
};

TOKEN_STRING popOp =
{
	_T("POP"),
	_T("SUB sp, sp, 1\n"),
	1
};

DWORD mathOpsCnt = sizeof(mathOps) / sizeof(TOKEN_BASIC);
DWORD loadStoreOpsCnt = sizeof(loadStoreOps) / sizeof(TOKEN_BASIC);
DWORD registersCnt = sizeof(registers) / sizeof(TOKEN_BASIC);

DWORD TryGetBinary(TOKEN_BASIC* list, DWORD len, TCHAR* token, INSTRUCTION* output)
{
	for (DWORD i = 0; i < len; i++)
	{
		if (!_tcscmp(list[i].name, token))
		{
			*output = list[i].binary;
			return TRUE;
		}
	}

	return FALSE;
}

void BinaryToString(size_t size, void* ptr, TCHAR* output)
{
	unsigned __int8* b = (unsigned __int8*)ptr;
	unsigned __int8 byte;
	DWORD i, j;

	for (i = 0; i < size; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (byte = (b[size - 1 - i] >> (7 - j)) & 1)
				_stprintf(output + i * 8 + j, _T("1"));
			else
				_stprintf(output + i * 8 + j, _T("0"));
		}
	}

	_stprintf(output + i * 8, _T("\0"));
}

void Assemble(TCHAR* lines, INSTRUCTION output[], DWORD* bytesWritten)
{
	TCHAR* restLines = NULL;
	TCHAR* line = _tcstok_s(lines, _T("\r\n"), &restLines);
	DWORD lineIdx = 0;

	while (line && _tcscmp(line, _T("\n")))
	{
		TCHAR* restTokens = NULL;
		TCHAR* token = _tcstok_s(line, _T(", "), &restTokens);
		INSTRUCTION binaryToken = 0;

		if (TryGetBinary(mathOps, mathOpsCnt, token, &binaryToken)) // arithmetic operations
		{
			output[lineIdx] = 0;

			// operator
			output[lineIdx] |= binaryToken << LOCATION_OPERATOR;

			// storage
			token = _tcstok_s(NULL, _T(", "), &restTokens);
			TryGetBinary(registers, registersCnt, token, &binaryToken);
			output[lineIdx] |= binaryToken << LOCATION_STORAGE;

			// operands
			for (int i = LOCATION_OPERAND_LEFT; i >= LOCATION_OPERAND_RIGHT; i -= (LOCATION_OPERAND_LEFT - LOCATION_OPERAND_RIGHT))
			{
				token = _tcstok_s(NULL, _T(", "), &restTokens);
				if (TryGetBinary(registers, registersCnt, token, &binaryToken))
				{
					binaryToken |= 0b1000;
					output[lineIdx] |= binaryToken << i;
				}
				else // literal (non-negative)
					output[lineIdx] |= _ttoi(token) << i;
			}

			*bytesWritten += sizeof(INSTRUCTION);
			lineIdx++;
		}
		else if (TryGetBinary(loadStoreOps, loadStoreOpsCnt, token, &binaryToken)) // load/store operations
		{
			output[lineIdx] = 0;

			// operator
			output[lineIdx] |= binaryToken << LOCATION_OPERATOR;

			// register
			token = _tcstok_s(NULL, _T(", "), &restTokens);
			TryGetBinary(registers, registersCnt, token, &binaryToken);
			output[lineIdx] |= binaryToken << LOCATION_REGISTER;

			// main memory
			if (_tcsstr(restTokens, _T("[")))
			{
				output[lineIdx] |= indirectMode << LOCATION_MODE;
				token = _tcstok_s(NULL, _T(", []"), &restTokens);
			}
			else
				token = _tcstok_s(NULL, _T(", "), &restTokens);

			binaryToken = _tcstol(token, NULL, 16);
			output[lineIdx] |= binaryToken << LOCATION_MAINMEMORY;

			*bytesWritten += sizeof(INSTRUCTION);
			lineIdx++;
		}
		else if (!_tcscmp(token, pushOp.name)) // push operation
		{
			token = _tcstok_s(NULL, _T(", "), &restTokens);
			TCHAR* buffer = NULL;

			__try
			{
				buffer = (TCHAR*)malloc(sizeof(pushOp.str));
				_stprintf(buffer, pushOp.str, token);

				Assemble(buffer, output + lineIdx, bytesWritten);
				lineIdx += pushOp.numOfLines;
			}
			__finally
			{
				if (buffer)
					free(buffer);
			}
		}
		else if (!_tcscmp(token, popOp.name)) // pop operation
		{
			Assemble(popOp.str, &output[lineIdx], bytesWritten);
			lineIdx += popOp.numOfLines;
		}

		line = _tcstok_s(restLines, _T("\r\n"), &restLines);
	}
}

int _tmain(int argc, TCHAR* argv[])
{
	if (argc < 2)
	{
		_tprintf(_T("command-line argument does not contain a .asm file. \n"));
		_tprintf(_T("\npress enter to exit...\n"));
		_gettchar();

		return EXIT_FAILURE;
	}

	TCHAR* fileName = argv[1];
	TCHAR* baseName = NULL;

	TCHAR* destFileName = NULL;
	TCHAR destFileExtension[] = _T(".mac");

	FILE* readStream = NULL;
	FILE* writeStream = NULL;

	TCHAR* lines = NULL;

	INSTRUCTION instructions[MAX_LINES];

	__try
	{
		readStream = _tfopen(fileName, _T("r"));

		// get file size
		fseek(readStream, 0, SEEK_END);
		DWORD fileSize = ftell(readStream);
		lines = (TCHAR*)malloc((fileSize + 1) * sizeof(TCHAR));

		fseek(readStream, 0, SEEK_SET);
		DWORD ch;
		DWORD charsRead = 0;

		// read
		while ((ch = _fgettc(readStream)) != _TEOF && charsRead < fileSize)
		{
			lines[charsRead] = (TCHAR)ch;
			charsRead++;
		}

		lines[charsRead] = NULL;

		// assemble
		DWORD bytesWritten = 0;
		Assemble(lines, instructions, &bytesWritten);

		// get file name to write
		baseName = (TCHAR*)malloc((_tcslen(fileName) + 1) * sizeof(TCHAR));
		_tcscpy(baseName, fileName);
		TCHAR* dot = _tcsrchr(baseName, (TCHAR)'.');
		*dot = NULL;

		destFileName = (TCHAR*)malloc((_tcslen(baseName) + _tcslen(destFileExtension) + 1) * sizeof(TCHAR));
		_tcscpy(destFileName, baseName);
		_tcscat(destFileName, destFileExtension);

		writeStream = _tfopen(destFileName, _T("wb"));
		TCHAR buffer[20];

		// write
		for (int i = 0; i < bytesWritten / sizeof(INSTRUCTION); i++)
		{
			BinaryToString(sizeof(INSTRUCTION), instructions + i, buffer);
			_tprintf(_T("%s \n"), buffer);
#ifdef WRITE_LINE_BY_LINE
			_stprintf(buffer + _tcslen(buffer), _T("\n"));
#endif
			_fputts(buffer, writeStream);
		}
	}
	__finally
	{
		if (readStream)
			fclose(readStream);

		if (writeStream)
			fclose(writeStream);

		if (lines)
			free(lines);

		if (baseName)
			free(baseName);

		if (destFileName)
			free(destFileName);
	}
	
	_tprintf(_T("\npress enter to exit...\n"));
	_gettchar();

	return EXIT_SUCCESS;
}
