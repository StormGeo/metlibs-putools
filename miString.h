/*
  libpuTools - Basic types/algorithms/containers
  
  $Id$

  Copyright (C) 2006 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: diana@met.no
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __dnmi_miString__
#define __dnmi_miString__

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <math.h>
#include <limits.h> 
#include "puAlgo.h"

namespace miutil{

class miString : public std::string {
  static const char whitespaces[];

public:
  miString()
    : std::string() {}
  miString(const char* s);
  miString(const std::string& s)
    : std::string(s) {}
  explicit miString(const int    d, const int =0, const char ='0');
  explicit miString(const double d, const int prec =-1);
  explicit miString(const float  d, const int prec =-1);

  const char* cStr() const
  { return c_str(); }

  bool contains(const miString& s) const
  { return find(s)==npos ? false: true; }

  bool exists() const { return !empty(); }

  void trim(bool =true, bool =true, const miString =whitespaces);
  void rtrim(const miString ws=whitespaces) { trim(false, true, ws); }
  void ltrim(const miString ws=whitespaces) { trim(true, false, ws); }

  int countChar(const char c) const
  { return std::count(begin(), end(), c); }

  void     remove( const char                      );
  miString replace(const char, const char          ) const;
  void     replace(const miString&, const miString&);

  std::vector<miString> split(const miString =whitespaces, const bool =true) const;
  std::vector<miString> split(const char, const bool =true) const;

  // nos = max number of splits, 0 = split all elements

  std::vector<miString> split(int nos, const miString =whitespaces, const bool =true) const;
  std::vector<miString> split(int nos, const char, const bool =true) const;

  // left/right border to protect strings inside the string () "" etc.

  std::vector<miString> split(const char leftborder, const char rightborder,
			      const miString =whitespaces, 
			      const bool =true) const;


  template< template< typename T, 
  typename ALLOC = std::allocator<T>  
  > class C, typename T>
  void join(const C<T>& j , const miString delimiter=" ")
  {
    bool first=true;
    std::ostringstream ost;
    typename C<T>::const_iterator itr= j.begin();
    for(;itr!=j.end();itr++){
      ost << (first ? "" : delimiter ) << *itr;
      first = false;
    }
    *this=ost.str();
  }

  template< template< typename T, 
  typename  COMPARE = std::less<T>,
  typename ALLOC = std::allocator<T>  
  > class C, typename T>
  void join(const C<T>& j , const miString delimiter=" ")
  {
    bool first=true;
    std::ostringstream ost;
    typename C<T>::const_iterator itr= j.begin();
    for(;itr!=j.end();itr++){
      ost << (first ? "" : delimiter ) << *itr;
      first = false;
    }
    *this=ost.str();
  }

  miString upcase(  int start=0, int len=0) const;
  miString downcase(int start=0, int len=0) const;

  bool     isNumber() const;
  bool     isInt()    const;

  int    toInt(    int    undefined=INT_MIN  ) const;
  float  toFloat(  float  undefined=NAN      ) const;
  double toDouble( double undefined=NAN      ) const;
  long   toLong(   long   undefined=LONG_MIN ) const;

};

}

#endif
