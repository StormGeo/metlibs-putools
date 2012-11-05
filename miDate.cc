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


// miDate.cc -- functions for class miDate

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "miDate.h"

using namespace std;
using namespace miutil;


miutil::miString  miutil::miDate::defaultLanguage;

static inline void warning(const miutil::miString& s)
{
  std::cerr << "Warning: miDate::" << s << std::endl;
}

static const long julianDayZero=1721425;

static const int monthLength[14]={
   0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0 };

static const int cum_ml[2][16]={
  { 0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365, 400, 0 },
  { 0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366, 400, 0 }};

static inline long lfloor(const long a, const long b) // assumes b positive
{ return a>=0? a/b: (a%b==0)-1-labs(a)/b; }

static inline int isLeap(const int y)
{ return ((y%4==0 && y%100!=0) || y%400==0); }

int
miutil::miDate::daysInYear() const
{ return 365+isLeap(Year); }

int
miutil::miDate::daysInMonth() const
{ return monthLength[Month]+(Month==2 && isLeap(Year)); }

int
miutil::miDate::dayOfYear() const
{ return cum_ml[isLeap(Year)][Month]+Day; }

/*
 * Constructors
 */

void
miutil::miDate::setDate(int y, int m, int d)
{
  if(!isValid(y,m,d) ) {
    miString w = miString("setDate: Illegal date! YYYY:MM:DD (") +
      miString(y) + miString(":") + miString(m) + miString(":") +miString(d)+miString(")");
    Year=Month=Day=jdn=0;
    return;
  }

  Year  = y;
  Month = m;
  Day   = d;

  jdn=(Year-1)*365 + lfloor(Year-1,4L) +
    lfloor(Year-1,400L) - lfloor(Year-1,100L) +
    cum_ml[isLeap(Year)][Month] + Day + julianDayZero;
}

void
miutil::miDate::setDate(const miString& str)
{
  int y=0, m=0, d=0;
  if(!isValid(str))
    warning( "setDate: Error in format. YYYY-MM-DD (" + str + ")" );
  else
    sscanf(str.cStr(), "%4d-%2d-%2d",&y, &m, &d);
  setDate(y,m,d);
}

bool
miutil::miDate::isValid(int y, int m, int d)
{

  if (m<=0 || m>12 || d<0 || d>31)
    return false;
  if ( d > ( monthLength[m]+(m==2 && isLeap(y))))
    return false;
  return true;
}

bool
miutil::miDate::isValid(const miString& str)
{
  int y=0, m=0, d=0;
  if(sscanf(str.cStr(), "%4d-%2d-%2d",&y, &m, &d)!=3)
    return false;
  return isValid(y,m,d);
}

/*
 * Arithmetic
 */

void
miutil::miDate::addDay(const long add)
{
  if (undef()) {
    warning("addDay: Can't add days. Date is undefined.");
    return;
  }

  jdn+=add;
  jdntodate(jdn);
}

/*
 * Conversion functions
 */

miutil::miDate&
miutil::miDate::jdntodate(long dn) // Convert Julian day number (dn) to date
{
  if (undef()) {
    warning("jdntodate: Date is undefined.");
    return *this;
  }

  int exception=0;

  const long y400=146097; // == 400*365.2425
  const long y100=36524;  // == round(100*365.2425)
  const long y4=1461;     // == round(4*365.2425)

  jdn=dn;
  dn-=julianDayZero+1;

  Year=400*lfloor(dn,y400);    // 400-year periods
  dn-=y400*lfloor(dn,y400);

  if (dn>0) {
    Year+=100*lfloor(dn,y100); // 100-year periods
    dn-=y100*lfloor(dn,y100);
    exception=(dn==0);         // Occurs once a century
    if (dn>0) {
      Year+=4*lfloor(dn,y4);   // 4-year periods
      dn-=y4*lfloor(dn,y4);
      if (dn>0) {
	int i=0;
	while (dn>365 && ++i<4) {
	  Year++;
	  dn-=365;
	}
      }
    }
  }

  if (exception)
    dn=366;
  else {
    Year++;
    dn++;
  }

  Month=1;
  while (cum_ml[isLeap(Year)][Month]<dn)
    Month++;
  Month--;
  dn-=cum_ml[isLeap(Year)][Month];
  if (Month==13) {
    Month=1;
    Year++;
  }

  Day=dn;

  return *this;
}

// Return a string with date formatted according to ISO
// standards (ISO 8601)
miutil::miString
miutil::miDate::isoDate() const
{
  if (undef())
    warning("isoDate: Date is undefined.");

  ostringstream ost;

  ost << setw(4) << setfill('0') << Year  << '-'
      << setw(2) << setfill('0') << Month << '-'
      << setw(2) << setfill('0') << Day;

  return ost.str();
}

// Returns the week number. Week 1 of a year is per definition the
// first week that contains a Thursday.
int
miutil::miDate::weekNo() const
{
  if (undef()) {
    warning("weekNo: Date is undefined. Can't compute week number.");
    return 0;
  }

  miDate mon(Year,1,1);
  int monD=(mon.intWeekday()+6)%7;

  if (monD<4)
    mon.addDay(-monD);
  else
    mon.addDay(7-monD);

  return (jdn-mon.jdn)/7+1;
}

miutil::miString
miutil::miDate::weekday(const lang l) const
{
  return weekday( (l== Norwegian ? "no" : "" ) );
}


miutil::miString
miutil::miDate::weekday(miString l) const
{
  if (undef()) {
    warning("weekday: Date is undefined. Can't find weekday.");
    return "";
  }

  if(!l.exists()) l=defaultLanguage;
  miString la = l.downcase();

  int a=intWeekday();

  static miString nameEN[]={ "Sunday",
			     "Monday",
			     "Tuesday",
			     "Wednesday",
			     "Thursday",
			     "Friday",
			     "Saturday" };

  static miString nameNO[]={ "S�ndag",
			     "Mandag",
			     "Tirsdag",
			     "Onsdag",
			     "Torsdag",
			     "Fredag",
			     "L�rdag" };

  static miString nameNN[]={ "S�ndag",
			     "M�ndag",
			     "Tysdag",
			     "Onsdag",
			     "Torsdag",
			     "Fredag",
			     "Laurdag" };

  static miString nameDE[]={ "Sonntag",
			     "Montag",
			     "Dienstag",
			     "Mittwoch",
			     "Donnerstag",
			     "Freitag",
			     "Samstag" };

  static miString nameSE[]={ "S�ndag",
			     "M�ndag",
			     "Tisdag",
			     "Onsdag",
			     "Torsdag",
			     "Fredag",
			     "L�rdag" };
  if (la=="no" || la=="nb")
    return nameNO[a];
  if (la=="nn")
    return nameNN[a];
  if (la=="de")
    return nameDE[a];
  if (la=="se" || la=="sv")
    return nameSE[a];

  return nameEN[a];
}

miutil::miString
miutil::miDate::shortweekday(const lang l) const
{
  return shortweekday( (l== Norwegian ? "no" : "" ) );
}



miutil::miString
miutil::miDate::shortweekday(miString l) const
{
  if (undef()) {
    warning("shortWeekday: Date is undefined. Can't find weekday.");
    return "";
  }

  if(!l.exists()) l=defaultLanguage;
  miString la = l.downcase();

  int a=intWeekday();

  static miString nameEN[]={ "Sun",
			     "Mon",
			     "Tue",
			     "Wed",
			     "Thu",
			     "Fri",
			     "Sat" };

  static miString nameNO[]={ "S�n",
			     "Man",
			     "Tir",
			     "Ons",
			     "Tor",
			     "Fre",
			     "L�r" };

  static miString nameNN[]={ "S�n",
			     "M�n",
			     "Tys",
			     "Ons",
			     "Tor",
			     "Fre",
			     "Lau" };

  static miString nameDE[]={ "So",
			     "Mo",
			     "Di",
			     "Mi",
			     "Do",
			     "Fr",
			     "Sa" };

  static miString nameSE[]={ "S�n",
			     "M�n",
			     "Tis",
			     "Ons",
			     "Tor",
			     "Fre",
			     "L�r" };
  if (la=="no" || la=="nb")
    return nameNO[a];
  if (la=="nn")
    return nameNN[a];
  if (la=="de")
    return nameDE[a];
  if (la=="se" || la=="sv")
    return nameSE[a];

  return nameEN[a];
}

miutil::miString
miutil::miDate::monthname(const lang l) const
{
  return monthname( (l== Norwegian ? "no" : "" ) );
}

miutil::miString
miutil::miDate::monthname(miString l) const
{
  if (undef()) {
    warning("monthname: Date is undefined. Can't return month name.");
    return "";
  }

  if(!l.exists()) l=defaultLanguage;
  miString la = l.downcase();

  static miString nameEN[]={ "January",
			     "February",
			     "March",
			     "April",
			     "May",
			     "June",
			     "July",
			     "August",
			     "September",
			     "October",
			     "November",
			     "December" };

  static miString nameNO[]={ "Januar",
			     "Februar",
			     "Mars",
			     "April",
			     "Mai",
			     "Juni",
			     "Juli",
			     "August",
			     "September",
			     "Oktober",
			     "November",
			     "Desember" };

  static miString nameDE[]={ "Januar",
			     "Februar",
			     "M�rz",
			     "April",
			     "Mai",
			     "Juni",
			     "Juli",
			     "August",
			     "September",
			     "Oktober",
			     "November",
			     "Dezember" };


  static miString nameSE[]={ "Januari",
			     "Februari",
			     "Mars",
			     "April",
			     "Maj",
			     "Juni",
			     "Juli",
			     "Augusti",
			     "September",
			     "Oktober",
			     "November",
			     "December" };

  if (la=="no" || la=="nb" || la=="nn")
    return nameNO[Month-1];
  if( la=="de")
    return nameDE[Month-1];
  if (la=="se" || la=="sv")
    return nameSE[Month-1];

  return nameEN[Month-1];
}

miutil::miString
miutil::miDate::shortmonthname(const lang l) const
{
  return shortmonthname( (l== Norwegian ? "no" : "" ) );
}


miutil::miString
miutil::miDate::shortmonthname(miString l) const
{
  if (undef()) {
    warning("monthShortname: Date is undefined. Can't return month name.");
    return "";
  }


  if(!l.exists()) l=defaultLanguage;
  miString la = l.downcase();

  static miString nameEN[]={ "Jan",
			     "Feb",
			     "Mar",
			     "Apr",
			     "May",
			     "Jun",
			     "Jul",
			     "Aug",
			     "Sep",
			     "Oct",
			     "Nov",
			     "Dec" };

  static miString nameNO[]={ "Jan",
			     "Feb",
			     "Mar",
			     "Apr",
			     "Mai",
			     "Jun",
			     "Jul",
			     "Aug",
			     "Sep",
			     "Okt",
			     "Nov",
			     "Des" };

  static miString nameDE[]={ "Jan",
			     "Feb",
			     "M�r",
			     "Apr",
			     "Mai",
			     "Jun",
			     "Jul",
			     "Aug",
			     "Sep",
			     "Okt",
			     "Nov",
			     "Dez" };

  static miString nameSE[]={ "Jan",
			     "Feb",
			     "Mar",
			     "Apr",
			     "Maj",
			     "Jun",
			     "Jul",
			     "Aug",
			     "Sep",
			     "Okt",
			     "Nov",
			     "Dec" };
  if (la=="no" || la=="nb" || la=="nn")
    return nameNO[Month-1];
  if (la=="de")
    return nameDE[Month-1];
  if (la=="se" || la=="sv")
    return nameSE[Month-1];

  return nameEN[Month-1];
}


miutil::miDate
miutil::miDate::easterSundayThisYear() const
{
  if (undef()) {
    warning("easterSundayThisYear: Date is undefined. Can't find easter.");
    return *this;
  }

  int g=Year%19;
  int c=Year/100;
  int h=(c - c/4 - (8*c+13)/25 + 19*g + 15)%30;
  int i=h - (h/28)*(1 - (h/28)*(29/(h+1))*((21-g)/11));
  int j=(Year + Year/4 + i + 2 - c + c/4)%7;
  int l=i-j;

  int eMonth=3+(l+40)/44;
  int eDay  =l+28-31*(eMonth/4);

  miDate easterSunday(Year,eMonth,eDay);

  return easterSunday;
}

miutil::miDate
miutil::miDate::today()
{
  time_t tp=time(0);
  struct tm *ts=gmtime(&tp);
  int y=ts->tm_year+1900;
  int m=ts->tm_mon+1;
  int d=ts->tm_mday;

  return miDate(y,m,d);
}


miutil::miString
miutil::miDate::format(miString newDate,const lang l) const
{
  return format(newDate, (l== Norwegian ? "no" : "" ) );
}

miutil::miString
miutil::miDate::format(miString newDate,miString l ) const
{
  if(undef())
    return newDate;

  newDate.replace("%y",miString(Year%100,2)); //!%y  last two digits of year (00..99)
  newDate.replace("%Y",miString(Year,4));     //!%Y  year (1970...)

  newDate.replace("%d",miString(Day,2));      //!%d  day of month (01..31)
  newDate.replace("%e",miString(Day));        //!%e  day of month ( 1..31)
  newDate.replace("%m",miString(Month,2));    //!%m  month (01..12)

  newDate.replace("%D",isoDate());            //!%D  date (yyyy-mm-dd)

  newDate.replace("%B",monthname(l));         //!%B  month  name,  (January..December)
  newDate.replace("%b",shortmonthname(l));    //!%b  short month  name,  (Jan..Dec)
  newDate.replace("%A",weekday(l));           //!%A  weekday name, (Sunday..Saturday)
  newDate.replace("%a",shortweekday(l));      //!%a  shortweekday name, (Sun..Sat)

  newDate.replace("%_B",monthname(l).downcase());      //!%B  month  name, lowercase
  newDate.replace("%_b",shortmonthname(l).downcase()); //!%b  short month  name, lowercase
  newDate.replace("%_A",weekday(l).downcase());        //!%A  weekday name,lowercase
  newDate.replace("%_a",shortweekday(l).downcase());   //!%a  shortweekday name,lowercase

  return newDate;
}
