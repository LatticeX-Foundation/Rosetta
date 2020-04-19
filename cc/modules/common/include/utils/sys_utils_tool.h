// ==============================================================================
// Copyright 2020 The LatticeX Foundation
// This file is part of the Rosetta library.
//
// The Rosetta library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The Rosetta library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with the Rosetta library. If not, see <http://www.gnu.org/licenses/>.
// ==============================================================================
#pragma once

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <string>

#if defined(WIN32) || defined(__WIN32__) || defined(_WIN32) || defined(_WIN64) || \
    defined(__WINDOWS__)
#include <Windows.h>
#include <process.h>
#include <time.h>
#else
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif
#include <stdint.h>
#include <string>
#define MAX_PATH_LEN 256

#ifdef _WIN32
#define ACCESS(fileName, accessMode) _access(fileName, accessMode)
#define MKDIR(path) _mkdir(path)
#else
#define ACCESS(fileName, accessMode) access(fileName, accessMode)
#define MKDIR(path) mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif

class SysUtilsTool {
 public:
  static void ms_sleep(unsigned long milisec) {
#if defined(WIN32) || defined(__WIN32__) || defined(_WIN32) || defined(_WIN64) || \
    defined(__WINDOWS__)
    Sleep(milisec);
#else
    usleep(1000 * milisec);
#endif
  }

  static int getTid() {
#if defined(_WIN32) || defined(_WIN64)
    return (long)GetCurrentThreadId();
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__ANDROID__)
    void* ptr;
    int nBits = sizeof(ptr);
    if (nBits == 4)
      return (long int)syscall(224);
    else // 64bits
      return (long int)syscall(186);
#else
    return (long)getpid();
#endif //
  }

  static int getPid() {
#if defined(_WIN32) || defined(_WIN64)
    return _getpid();
#else
    return (int)getpid();
#endif //
  }

  static char* getErrStr(int error_no) {
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
    static char lpMsgBuf[128];
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPTSTR)&lpMsgBuf, sizeof(lpMsgBuf), NULL);

    return lpMsgBuf;
#else
    return strerror(error_no);
#endif
  }

  static char* getErrStr() {
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
    static char lpMsgBuf[128];
    FormatMessage(
        /*FORMAT_MESSAGE_ALLOCATE_BUFFER | */ FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPTSTR)&lpMsgBuf, sizeof(lpMsgBuf), NULL);

    return lpMsgBuf;
#else
    return strerror(errno);
#endif
  }

  static int getErrno() {
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
    return GetLastError();
#else
    return errno;
#endif //
  }

  static uint64_t getSystemTime(struct timeval* tp, void* tzp) {
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;

    GetLocalTime(&wtm);
    tm.tm_year = wtm.wYear - 1900;
    tm.tm_mon = wtm.wMonth - 1;
    tm.tm_mday = wtm.wDay;
    tm.tm_hour = wtm.wHour;
    tm.tm_min = wtm.wMinute;
    tm.tm_sec = wtm.wSecond;
    tm.tm_isdst = -1;
    clock = mktime(&tm);
    tp->tv_sec = (long)clock;
    tp->tv_usec = (long)wtm.wMilliseconds * 1000;
    return 0;
#else
    return gettimeofday(tp, NULL);
#endif //
  }

  static uint64_t getSysCurrSec() {
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;

    GetLocalTime(&wtm);
    tm.tm_year = wtm.wYear - 1900;
    tm.tm_mon = wtm.wMonth - 1;
    tm.tm_mday = wtm.wDay;
    tm.tm_hour = wtm.wHour;
    tm.tm_min = wtm.wMinute;
    tm.tm_sec = wtm.wSecond;
    tm.tm_isdst = -1;
    clock = mktime(&tm);

    return clock;
#else
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec;
#endif //
  }

  static uint64_t getFileSize(const char* szFile) {
    if (!checkFileExists(szFile))
      return -1;

#if defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
    return getAppleFileSize(szFile);
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (TRUE != GetFileAttributesExA(szFile, GetFileExInfoStandard, (void*)&fileInfo))
      return -1;

    LARGE_INTEGER size;
    size.HighPart = fileInfo.nFileSizeHigh;
    size.LowPart = fileInfo.nFileSizeLow;
    return size.QuadPart;

#else // linux, android
    struct stat stFile;
    if (stat(szFile, &stFile) != 0) {
      return -1;
    }

    return stFile.st_size;
#endif
  }

  static uint64_t getFileUpdateTime(const char* szFile) {
    if (!checkFileExists(szFile))
      return -1;

#if defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
    return getAppleFileUpdateTime(szFile);
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (TRUE != GetFileAttributesExA(szFile, GetFileExInfoStandard, (void*)&fileInfo))
      return -1;

    SYSTEMTIME objSysTime;
    FileTimeToSystemTime(&fileInfo.ftLastWriteTime, &objSysTime);

    time_t nTimeStamp;
    struct tm tm;

    GetLocalTime(&objSysTime);
    tm.tm_year = objSysTime.wYear - 1900;
    tm.tm_mon = objSysTime.wMonth - 1;
    tm.tm_mday = objSysTime.wDay;
    tm.tm_hour = objSysTime.wHour;
    tm.tm_min = objSysTime.wMinute;
    tm.tm_sec = objSysTime.wSecond;
    tm.tm_isdst = -1;
    nTimeStamp = mktime(&tm);

    return nTimeStamp;
#else // linux, android
    struct stat stFile;
    if (stat(szFile, &stFile) != 0) {
      return -1;
    }

    return stFile.st_mtime;
#endif //
  }

  static bool checkFileExists(const std::string& filePath) {
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
    DWORD fileType = GetFileAttributesA(filePath.c_str());
    if (fileType == INVALID_FILE_ATTRIBUTES)
      return false; // something is wrong with your path!

    return true;
#elif defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
    int ret = checkAppleFileExists(filePath.c_str());
    return ret == 1 ? true : false;

#else
    int iRet;
    struct stat stDir;

    iRet = stat(filePath.c_str(), &stDir);
    if (iRet != 0) {
      return false;
    }

    if (stDir.st_mode & S_IFREG) //
    {
      return true;
    }

    return false;
#endif //
  }

  static bool checkDirExists(const std::string& dirPath) {
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
    DWORD fileType = GetFileAttributesA(dirPath.c_str());
    if (fileType == INVALID_FILE_ATTRIBUTES)
      return false; // something is wrong with your path!

    if (fileType & FILE_ATTRIBUTE_DIRECTORY)
      return true;

    return false; // this is not a directory!

#elif defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
    int ret = checkAppleDirExists(dirPath.c_str());
    return ret == 1 ? true : false;

#else
    int ret;
    struct stat stDir;

    ret = stat(dirPath.c_str(), &stDir);
    if (ret != 0) {
      return false;
    }

    if (stDir.st_mode & S_IFDIR) {
      return true;
    }

    return false;
#endif //
  }

  static bool createFileDir(const std::string& dirPath) {
    if (dirPath.size() <= 0)
      return false;

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
    if (FALSE == CreateDirectoryA(dirPath.c_str(), NULL))
      return false;
#elif defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
    if (0 != createAppleMsDir(dirPath.c_str()))
      return false;
#else // android,  linux
    if (0 != mkdir(dirPath.c_str(), 0755))
      return false;
#endif

    return true;
  }

  // createDirectories("/aa/bb/cc/")
  static int32_t createDirectories(const std::string& directoryPath) {
    uint32_t dirPathLen = directoryPath.length();
    if (dirPathLen > MAX_PATH_LEN) {
      return -1;
    }
    char tmpDirPath[MAX_PATH_LEN] = {0};
    for (uint32_t i = 0; i < dirPathLen; ++i) {
      tmpDirPath[i] = directoryPath[i];
      if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/') {
        if (ACCESS(tmpDirPath, 0) != 0) {
          int32_t ret = MKDIR(tmpDirPath);
          if (ret != 0) {
            return ret;
          }
        }
      }
    }
    return 0;
  }
};
