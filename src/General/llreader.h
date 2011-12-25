/**
 * @file  llreader.h
 * @brief Header declaring a general-purpose array-to-file adapter.
 * 
 * This file is meant to provide a faster alternative to the standard C++
 * stream classes. It can work directly with files and strings alike.
 * 
 * @section License
 * 
 * Copyright (C) 2011 Josh Ventura
 * This file is part of JustDefineIt.
 * 
 * JustDefineIt is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3 of the License, or (at your option) any later version.
 * 
 * JustDefineIt is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * JustDefineIt. If not, see <http://www.gnu.org/licenses/>.
**/

/**
  @brief A low-level file reader structure for quick I/O.
  
  This structure guarantees that its data member represents
  the file as a string in memory, and that its length member
  gives the valid length of the array member. The implementation
  may in fact virtually map the file in memory or read the
  whole thing in at once.
  
  This class may also represent a simple string in memory
  if one is provided in place of a file.
**/

#ifndef _LLREADER__H
#define _LLREADER__H

#include <string>

class llreader {
  int mode; ///< What kind of stream we have open
  
  public:
    size_t pos; ///< Our position in the stream
    size_t length; ///< The length of the stream
    const char *data; ///< The data in the stream
    /**
      Open a file as an array.
      Length will reflect the length of the file, and
      data will mirror its contents.
      @param filename  The name of the file that will be opened.
    **/
    void open(const char* filename);
    /**
      Encapsulate a string for the duration of its life.
      
      Length will be set to code.length(), and data will be
      set to its c_str().
      
      @param contents  A string containing contents to be pointed to.
      @warning As this function only encapsulates the string,
               without copying it, its contents will become
               invalid once the encapsulated string leaves scope.
    **/
    void encapsulate(std::string &contents);
    /**
      Copies a string into a new buffer and uses it as the data.
      Length will be set to code.length(), and data will be set
      to a buffer containing a copy of its c_str().
      @param contents  A string containing contents to be mirrored.
    **/
    void copy(std::string contents);
    /**
      Close the open stream.
      This function will take care of any necessary memory frees
      or file close operations. It is called automatically by the
      destructor.
    **/
    void close();
    
    /**
      Overrides default operator= to create a copy of the stored data.
      This function behaves like the constructor.
    **/
    void operator=(const llreader& llr);
    
    /**
      Returns whether the stream is open.
    **/
    bool is_open();
    
    /**
      Default constructor.
      There's really no good way to make a constructor for each method above.
    **/
    llreader();
    /**
      The (const char*) constructor behaves like open().
      This is because knowing the length of the const char* won't help, anyway.
      @param filename  The name of the file that will be opened.
    **/
    llreader(const char* filename);
    /**
      The (string,bool) constructor behaves like either copy() or encapsulate().
      For both of these, it's good to know how big the string is.
      @param contents  A string containing contents to be mirrored.
      @param copy      True if the contents are to be copied, false if they are 
                       to simply be pointed to (see \c encapsulate).
    **/
    llreader(std::string contents, bool copy);
    /**
      Copy constructor.
      This constructor really shouldn't be used; it is provided to be compliant.
      This constructor will perform an O(N) copy of the entire contents to a string,
      resulting in the new class having a different storage mode.
    **/
    llreader(const llreader&);
    /**
      Default destructor. Closes open stream.
    **/
    ~llreader();
};

#endif
