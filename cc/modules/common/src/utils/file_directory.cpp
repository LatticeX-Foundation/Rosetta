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
#include "cc/modules/common/include/utils/file_directory.h"

int get_file_lines_fields(const string& file, char delimiter, int& lines, int& fields, bool ignore_blank_line)
{
  fields = 1;
  lines = 0;
  const int buf_size = 16*1024;
  char buf[buf_size] = {0};
  int fd = open(file.data(), O_RDONLY);
  if (fd == -1)
  {
    log_error << "open file: "  << file << "failed!";
    return -1;
  }

  // read the first line
  ssize_t bytes = 0;
  if (0 < (bytes = read(fd, buf, 4096))) {
    char* p = buf;
    while (*p != '\n' && p != buf+bytes) {
      if (*(p++) == ',')
        fields++;
    }
  }
  //reset file to begin
  lseek(fd, 0, SEEK_SET);

  int index = 0;
  while(0 < (bytes=read(fd, buf, buf_size))){
    //count lines for the buffer
    char* p = buf;
    if (ignore_blank_line) {
      char* pre = p;
      while (p != buf+bytes)
      {
        if(*(p++) == '\n')
        {
          if (p - pre == 1)
          {
            pre = p;
            continue;
          }

          lines++;
          pre = p;
        }
      }
    } else {
      while (p != buf+bytes)
      {
        if(*(p++) == '\n')
        {
          lines++;
        }
      }
    }

    if (bytes >=1 && bytes < buf_size &&  buf[bytes-1] !='\n')
      ++lines;
  }

  log_debug << "lines: "<< lines << ", fields: " << fields;
  return 0;
}
