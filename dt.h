#pragma once
#include <iostream>
#include <string>
#include <ctime>

using namespace std;

const int DTSize = 20;

string leadZero(const int n) {
	return n < 10 ? "0" + to_string(n) : to_string(n);
}

class DateTime : public tm {
private:
	tm t;
public:
	DateTime() {
		time_t timestamp = time(0);
		localtime_s(&t, &timestamp);
	}
	operator string() {
		const int year = 1900 + t.tm_year;
		const int* nums[] = {
			&t.tm_mday, &t.tm_mon, &year,
			&t.tm_hour, &t.tm_min, &t.tm_sec };
		const char* delims = ".. ::";
		string s;
		for (uint16_t i = 0; i < sizeof(nums) / sizeof(int*); i++)
			s += leadZero(*nums[i]) + delims[i];
		return s;
	}
	friend ostream& operator <<(ostream& out, DateTime& dt) {
		out << static_cast<string>(dt) << endl;
		return out;
	}
};
