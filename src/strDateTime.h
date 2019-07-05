#ifndef _far
#define _far

class  strDateTime2 {
	public:
	byte hour;
	byte minute;
	byte second;
	int year;
	byte month;
	byte day;
	byte wday;

	String toString() {
		String dt = (String)hour+":"+(String)+ minute+":"+(String)second
           + "D" +(String)year+ "-" +(String)month+ "-" +(String)day;
      return dt;
	}
} ;

#endif