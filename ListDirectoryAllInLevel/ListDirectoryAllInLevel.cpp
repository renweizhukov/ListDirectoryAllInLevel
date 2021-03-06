// ListDirectoryAllInLevel.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;

void Readme()
{
    _tprintf(TEXT("Usage: ListDirectoryAllInLevel [path-to-directory/file]\n"));
}

BOOL AddSubDirsAndFilesToSearchQ(
    array<TCHAR, MAX_PATH>& currPath,
    queue<array<TCHAR, MAX_PATH>>& searchQ)
{
    // Check if the input path plus 3 is not longer than MAX_PATH.
    // The three additional characters are for the wild card "\*" 
    // plus the NULL terminator appended below.
    size_t pathLen = 0;
    StringCchLength(currPath.data(), MAX_PATH, &pathLen);

    if (pathLen + 3 > MAX_PATH)
    {
        _tprintf(TEXT("Error: the directory path with the length %d is longer than MAX_PATH - 3 = %d.\n"),
            pathLen, MAX_PATH - 3);
        return FALSE;
    }

    // Obtain the path search pattern by appending the wild card "\*" to currPath.
    // This search pattern will be used by FindFirstFile().
    array<TCHAR, MAX_PATH> pathPattern = currPath;
    StringCchCat(pathPattern.data(), MAX_PATH, TEXT("\\*"));

    WIN32_FIND_DATA ffd = { 0 };
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;

    hFind = FindFirstFile(pathPattern.data(), &ffd);
    // For a directory, FindFirstFile should at least find two special files 
    // "." and "..". If it can't find anything, then it implies something 
    // wrong with the path.
    if (hFind == INVALID_HANDLE_VALUE)
    {
        dwError = GetLastError();
        _tprintf(TEXT("Error: FindFirstFile %d"), dwError);
        return FALSE;
    }

    do
    {
        // Ignore the first file "." and the second file ".."
        if ((wcscmp(ffd.cFileName, TEXT(".")) != 0) &&
            (wcscmp(ffd.cFileName, TEXT("..")) != 0))
        {
            // The result ffd.cFileName returned by FindFirstFile() and FindNextFile() 
            // is a relative path based on currPath and isn't a full path, so we append 
            // ffd.cFileName to currPath to obtain the full path.
            array<TCHAR, MAX_PATH> fileName;
            StringCchCopy(fileName.data(), MAX_PATH, currPath.data());
            StringCchCat(fileName.data(), MAX_PATH, TEXT("\\"));
            StringCchCat(fileName.data(), MAX_PATH, ffd.cFileName);
            searchQ.push(fileName);
        }
    } while (FindNextFile(hFind, &ffd) != 0);

    BOOL fRet = TRUE;
    dwError = GetLastError();
    // Only the error "ERROR_NO_MORE_FILES" is expected when FindNextFile() 
    // reaches its search end and other errors should be treated as failures.
    if (dwError != ERROR_NO_MORE_FILES)
    {
        _tprintf(TEXT("Error: FindNextFile %d"), dwError);
        fRet = FALSE;
    }

    FindClose(hFind);
    hFind = INVALID_HANDLE_VALUE;

    return fRet;
}

int _tmain(int argc, _TCHAR* argv[])
{
    if (argc != 2)
    {
        _tprintf(TEXT("Error: need to specify a path.\n"));
        Readme();
        return -1;
    }

    // Check if the input path is not longer than MAX_PATH.
    size_t argLen = 0;
    StringCchLength(argv[1], MAX_PATH, &argLen);

    if (argLen > MAX_PATH)
    {
        _tprintf(TEXT("Error: the input path with the length %d is longer than MAX_PATH = %d.\n"),
            argLen, MAX_PATH);
        return -1;
    }

    // Check if the input path exists in the system.
    BOOL fPathExists = PathFileExists(argv[1]);
    if (!fPathExists)
    {
        _tprintf(TEXT("Error: the input path %s doesn't exist.\n"), argv[1]);
        return -1;
    }

    // Set the Unicode mode in the console for displaying Unicode characters later.
    // Note that if you still see garbled characters in the console, you may need 
    // to change the console font to one which supports those Unicode characters, 
    // e.g., FangSong for Chinese.
    _setmode(_fileno(stdout), _O_U16TEXT);
    _tprintf(TEXT("INFO: the root path is %s.\n"), argv[1]);

    array<TCHAR, MAX_PATH> rootPath;
    StringCchCopy(rootPath.data(), MAX_PATH, argv[1]);
    
    // The STL containers queue and vector don't work with C arrays like TCHAR[MAX_PATH] 
    // since the C arrays don't have constructors and destructors, so we have to use the 
    // STL arrays here.
    queue<array<TCHAR, MAX_PATH>> searchQ;
    searchQ.push(rootPath);

    vector<array<TCHAR, MAX_PATH>> allSubDirsAndFiles;
    
    DWORD dwFileAttr = 0;
    
    // Perform a breadth-first search starting from the input path.
    while (!searchQ.empty())
    {
        array<TCHAR, MAX_PATH>& currProcessedPath = searchQ.front();
        allSubDirsAndFiles.push_back(currProcessedPath);

        // Get the file attributes of currProcessedPath.
        dwFileAttr = GetFileAttributes(currProcessedPath.data());

        // If currProcessedPath is a directory and is not a symbolic link,
        // then add its subdirectories and files to the search queue.
        if ((dwFileAttr & FILE_ATTRIBUTE_DIRECTORY) &&
            !(dwFileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
        {
            // Add the subdirectories and files of currProcessedPath to 
            // the search queue.
            if (!AddSubDirsAndFilesToSearchQ(currProcessedPath, searchQ))
            {
                // Here we treat the failure of AddSubDirsAndFilesToSearchQ 
                // as a fatal one and exit immediately. Depending on your 
                // situation, you may ignore this failure and continue.
                return -1;
            }
        }

        searchQ.pop();
    }

    // Print out all the subdirectories and files.
    _tprintf(TEXT("All the %d subdirectories and files are listed as below:\n"), 
        allSubDirsAndFiles.size());
    _tprintf(TEXT("================================================================\n"));
    for (auto& onePath : allSubDirsAndFiles)
    {
        _tprintf(TEXT("\t%s\n"), onePath.data());
    }
    _tprintf(TEXT("================================================================\n"));

    return 0;
}