/*
  Scene is a custom 3d scene parser intended for use with graphical demos.
  
  Copyright (C) 2013, Daniel Green

  Scene is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Scene is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Scene.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __StringUtils__
#define __StringUtils__

#include <vector>
#include <string>
#include <fstream>
#include <sstream>

namespace strutils {
  static std::string removeSpaces( const std::string& str ) {
    int begin       = 0;
    int end         = str.size()-1;
    bool beginFound = false;
    bool endFound   = false;
    while( !beginFound || !endFound ) {
      // Safety check.
      if( begin >= end ) {
        break;
      }

      // If the begin character is a space, increment.  Otherwise, we found the first character.
      if( !beginFound ) {
        const char beginChar = str[begin];
        if( beginChar == ' ' ) {
          begin += 1;
        } else {
          beginFound = true;
        }
      }

      // If the end character is a space, decrement.  Otherwise, we found the last character.
      if( !endFound ) {
      const char endChar = str[end];
        if( endChar == ' ' ) {
          end -= 1;
        } else {
          endFound = true;
        }
      }
    }

    // Size safety check.  Add 1 to end as it catches the first non-space character.
    const int size = (end+1) - begin;
    if( size <= 0 ) {
      return str;
    }

    return std::string(&str[begin], size);
  }

  static void splitString( const char* const str, const int size, const char delim, bool keepSpaces, std::vector<std::string>* outStr ) {
    // Safety check.
    if( outStr == nullptr ) {
      return;
    }

    long index = 0;
    long end   = 0;
    char c = ' ';
    for( ;; ) {
      // Bounds check of index.
      if( index > size ) {
        break;
      }

      // Read current character.
      c = str[index];

      // EoF check.
      if( c == '\0' ) {
        break;
      }

      // Update the end to the current index.
      end = index;

      // Read until we find the next delim character.
      for( ;; ) {
        // Read next character.
        c = str[end];
        // If this character is the newline, break out.
        // Also check EoF for safety.
        if( c == delim || c == '\0' ) {
          break;
        }
        // Increment end as we're not at the delim or EoF yet.
        end += 1;
      }

      // Check the size of the bounds we have in case they are invalid.
      // If they are, advance and continue on.
      const long len = end - index;
      if( len <= 0 ) {
        index += 1;
        continue;
      }

      // Read the current range of characters into a string.
      std::string currRange(&str[index], len);
      // Trim prefix and postfix whitespace, if required.
      if( !keepSpaces ) {
        currRange = strutils::removeSpaces(currRange);
      }
      // Add the string to the vector.
      outStr->push_back(currRange);

      // Set the new index to the end + 1 to read the next character.
      index = end + 1;
    }
  }
}

#endif /* __StringUtils__ */
