// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef NAIVENET_BASE_DATE_H
#define NAIVENET_BASE_DATE_H

#include <algorithm>
#include <string>

struct tm;

namespace NaiveNet
{

	///
	/// Date in Gregorian calendar.
	///
	/// This class is immutable.
	/// It's recommended to pass it by value, since it's passed in register on x64.
	///

	// ������������ά���ٿƣ�http://zh.wikipedia.org/wiki/%E5%84%92%E7%95%A5%E6%97%A5

	// �����գ�Julian day����ָ�ɹ�Ԫǰ4713��1��1�գ�Э������ʱ����12ʱ��ʼ����������������Ϊ����ѧ�Ҳ��ã�������Ϊ����ѧ�ĵ�һ�������Ѳ�ͬ���������ͳһ������

	// ��������һ�ֲ������µĳ��ڼ��շ�����дΪJD�����ɺ�������ѧ��ʷ�����գ�Joseph Justus Scliger 1540��-1609�꣩��1583����������������Ϊ�˼������ĸ��ס��������ѧ��Julius Caesar Scaliger��1484��-1558�꣩��

	// �������ռ�����Ϊ���������������Զ��ͬ���������¼��������������

	class Date 
		// public boost::less_than_comparable<Date>,
		// public boost::equality_comparable<Date>
	{
	public:

		// ��ʾ������
		struct YearMonthDay
		{
			int year; // [1900..2500]
			int month;  // [1..12]
			int day;  // [1..31]
		};

		// һ��������
		static const int kDaysPerWeek = 7;
		static const int kJulianDayOf1970_01_01;

		///
		/// Constucts an invalid Date.
		///
		Date()
			: julianDayNumber_(0)  // ����һ����Ч������
		{}

		///
		/// Constucts a yyyy-mm-dd Date.
		///
		/// 1 <= month <= 12
		Date(int year, int month, int day);

		///
		/// Constucts a Date from Julian Day Number.
		///
		explicit Date(int julianDayNum)
			: julianDayNumber_(julianDayNum)
		{}

		///
		/// Constucts a Date from struct tm
		///
		explicit Date(const struct tm&);

		// default copy/assignment/dtor are Okay

		void swap(Date& that)
		{
			std::swap(julianDayNumber_, that.julianDayNumber_);
		}

		bool valid() const { return julianDayNumber_ > 0; }

		///
		/// Converts to yyyy-mm-dd format.
		///
		std::string toIsoString() const;

		// ʹ��julianDayNumber_ ����һ�������ն����Ա�����ȡ���е�year month day
		struct YearMonthDay yearMonthDay() const;

		int year() const
		{
			return yearMonthDay().year;
		}

		int month() const
		{
			return yearMonthDay().month;
		}

		int day() const
		{
			return yearMonthDay().day;
		}

		// [0, 1, ..., 6] => [Sunday, Monday, ..., Saturday ]
		// һ�ܵĵڼ���
		int weekDay() const
		{
			return (julianDayNumber_ + 1) % kDaysPerWeek;
		}

		int julianDayNumber() const { return julianDayNumber_; }

	private:
		int julianDayNumber_; //�����ڶ�Ӧ��������
	};

	inline bool operator<(Date x, Date y)
	{
		return x.julianDayNumber() < y.julianDayNumber();
	}

	inline bool operator==(Date x, Date y)
	{
		return x.julianDayNumber() == y.julianDayNumber();
	}

}
#endif  // NAIVENET_BASE_DATE_H
