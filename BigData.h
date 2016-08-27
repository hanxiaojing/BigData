#ifndef BIG_DATA_H
#define BIG_DATA_H
#include <string>
#include <iostream>

#define UN_INIT 0xcccccccccccccccc//?
#define MAX_INT64 0x7fffffffffffffff
#define MIN_INT64 0x8000000000000000

typedef long long INT64;

class BigData
{
public:
	BigData(INT64 data = UN_INIT);
	BigData(const char *pData);

	BigData operator+(BigData& bigData);
	BigData operator-(const BigData& bigData);
	BigData operator*(const BigData& bigData);
	BigData operator/(const BigData& bigData);
	BigData operator%(const BigData& bigData);

	//=======================================
	bool operator<(const BigData& bigData);
	bool operator>(const BigData& bigData);
	bool operator==(const BigData& bigData);
	
	std::ostream& operator<<(std::ostream& _cout);

	friend std::ostream& operator<<(std::ostream& _cout, const BigData& bigData);//由于自定义类型的自定义数据成员无法输出，所以要重载
	friend std::istream& operator>>(std::istream& _cin, BigData bigData);
	
private:
	std::string Add(std::string left, std::string right);
	std::string Sub(std::string left, std::string right);
	std::string Mul(std::string left, std::string right);
	std::string Div(std::string left, std::string right);
	void INT64ToString();
	bool IsINT64Owerflow()const;
	bool IsLeftStrBig(char *pLeft, size_t LSize, char *pRight, size_t RSize);
	char SubLoop(char *pLeft, size_t LSize, char *pRight, size_t RSize);

private:
	long long m_llValue;
	std::string m_strData;
};

#endif

