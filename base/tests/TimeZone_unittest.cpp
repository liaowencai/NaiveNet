#include "../TimeZone.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

using NaiveNet::TimeZone;

struct tm getTm(int year, int month, int day,
	int hour, int minute, int seconds)
{
	struct tm gmt;
	bzero(&gmt, sizeof gmt);
	gmt.tm_year = year - 1900;
	gmt.tm_mon = month - 1;
	gmt.tm_mday = day;
	gmt.tm_hour = hour;
	gmt.tm_min = minute;
	gmt.tm_sec = seconds;
	return gmt;
}

struct tm getTm(const char* str)
{
	struct tm gmt;
	bzero(&gmt, sizeof gmt);
	strptime(str, "%F %T", &gmt);
	return gmt;
}

time_t getGmt(int year, int month, int day,
	int hour, int minute, int seconds)
{
	struct tm gmt = getTm(year, month, day, hour, minute, seconds);
	return timegm(&gmt);
}

time_t getGmt(const char* str)
{
	struct tm gmt = getTm(str);
	return timegm(&gmt);
}

struct TestCase
{
	const char* gmt;
	const char* local;
	bool isdst;
};

void test(const TimeZone& tz, TestCase tc)
{
	time_t gmt = getGmt(tc.gmt);

	{
		struct tm local = tz.toLocalTime(gmt);
		char buf[256];
		strftime(buf, sizeof buf, "%F %T%z(%Z)", &local);

		if (strcmp(buf, tc.local) != 0 || tc.isdst != local.tm_isdst)
		{
			printf("WRONG: ");
		}
		printf("%s -> %s\n", tc.gmt, buf);
	}

	{
		struct tm local = getTm(tc.local);
		local.tm_isdst = tc.isdst;
		time_t result = tz.fromLocalTime(local);
		if (result != gmt)
		{
			struct tm local2 = tz.toLocalTime(result);
			char buf[256];
			strftime(buf, sizeof buf, "%F %T%z(%Z)", &local2);

			printf("WRONG fromLocalTime: %ld %ld %s\n",
				static_cast<long>(gmt), static_cast<long>(result), buf);
		}
	}
}

void testUtc()
{
	const int kRange = 100 * 1000 * 1000;
	for (time_t t = -kRange; t <= kRange; t += 11)
	{
		struct tm* t1 = gmtime(&t);
		struct tm t2 = TimeZone::toUtcTime(t, true);
		char buf1[80], buf2[80];
		strftime(buf1, sizeof buf1, "%F %T %u %j", t1);
		strftime(buf2, sizeof buf2, "%F %T %u %j", &t2);
		if (strcmp(buf1, buf2) != 0)
		{
			printf("'%s' != '%s'\n", buf1, buf2);
			assert(0);
		}
		time_t t3 = TimeZone::fromUtcTime(t2);
		if (t != t3)
		{
			printf("%ld != %ld\n", static_cast<long>(t), static_cast<long>(t3));
			assert(0);
		}
	}
}

void testFixedTimezone()
{
	TimeZone tz(8 * 3600, "CST");
	TestCase cases[] =
	{

		{ "2014-04-03 00:00:00", "2014-04-03 08:00:00+0800(CST)", false },

	};

	for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i)
	{
		test(tz, cases[i]);
	}
}

int main()
{
	testFixedTimezone();
	testUtc();

	return 0;
}