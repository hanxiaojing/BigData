#include "BigData.h"
#include <cassert>

BigData::BigData(INT64 data)
	: m_llValue(data)
	, m_strData("")
{
	INT64ToString();
}

BigData::BigData(const char *_pData)
{
	// "-12345789"  "1234567" "+" "12457aaa123" "000001234567"
	// "a23456789" 
	// atoi
	assert(NULL != _pData);

	char cSybom = _pData[0];
	char* pData = (char*)_pData;
	if ('+' == cSybom || '-' == cSybom)//统一符号先确定符号--》数值大小--》加上符号位
	{
		pData++;
	}
	else if (*pData >= '0' && *pData <= '9')
	{
		cSybom = '+';
	}
	else
	{
		m_llValue = 0;
		m_strData = "0";
		return;
	}

	// 去掉前置0
	while('0' == *pData)
		pData++;

	// "12457aaa123"
	m_strData.resize(strlen(pData)+1);
	m_llValue = 0;
	m_strData[0] = cSybom;
	int iCount = 1;
	while(pData)/////////////////////////////////////
	{
		if (*pData >= '0' && *pData <= '9')
		{
			m_llValue = m_llValue*10 + *pData - '0';
			m_strData[iCount++] = *pData++;
		}
		else
		{
			break;
		}
	}

	m_strData.resize(iCount);

	if ('-' == cSybom)
	{
		m_llValue = 0 - m_llValue;
	}
}

BigData BigData::operator+(BigData& bigData)//同号都未溢出：同号结果不溢出直接加	溢出Add()，异号直接加返回		有一个溢出，两个溢出:同号调Add(),异号Sub()
{
	// 8 + -2  10
	if (!IsINT64Owerflow() && !bigData.IsINT64Owerflow())
	{
		if (m_strData[0] != bigData.m_strData[0])
		{
			return BigData(m_llValue+bigData.m_llValue);
		}
		else
		{
			// 2 + 8  10 - 6 > 2
			// -3 + -8  -10 - (-6) = -4 < -3
			if (('+' == m_strData[0] && MAX_INT64 - m_llValue >= bigData.m_llValue) ||
				('-') == m_strData[0] && MIN_INT64 - m_llValue <= bigData.m_llValue)
			{
				return BigData(m_llValue+bigData.m_llValue);
			}
		}
	}

	// 2 + 2 / -2 + -2 == -(2+2)
	// 2 + -1 
	// 至少有一个溢出
	// 计算结果溢出
	std::string strRet;
	if (m_strData[0] == bigData.m_strData[0])
	{
		strRet = Add(m_strData, bigData.m_strData);
	}
	else
	{
		strRet = Sub(m_strData, bigData.m_strData);
	}

	return BigData(strRet.c_str());
}

BigData BigData::operator-(const BigData& bigData)//两都未溢出：同号-》直接减，异号-》直接减	一个或两个溢出：同号-》Sub,异号-》Add
{
	if (!IsINT64Owerflow() && !bigData.IsINT64Owerflow())
	{
		if (m_strData[0] == bigData.m_strData[0])
		{
			return BigData(m_llValue - bigData.m_llValue);
		}
		else
		{
			// 10 + (-8) = 2 > 1// 3 - (-8); 1 - (-8) 
			// -10  -8  3    -8  2  -10 + 3 = -7 <= 
			if (('+' == m_strData[0] && MAX_INT64 + bigData.m_llValue >= m_llValue) ||
				('-' == m_strData[0] && MIN_INT64 + bigData.m_llValue <= m_llValue))
			{
				return BigData(m_llValue - bigData.m_llValue);
			}
		}
	}

	// 1、至少有一个操作数溢出
	// 2、相减的结果一定会溢出
	// "999999999" "-111111"  "-9999999" "1111111"
	std::string strRet;
	if (m_strData[0] != bigData.m_strData[0])
	{
		strRet = Add(m_strData, bigData.m_strData);
	}
	else
	{
		strRet = Sub(m_strData, bigData.m_strData);
	}
	return BigData(strRet.c_str());
}

BigData BigData::operator*(const BigData& bigData)//1.为0直接返回0 2.两个（同号、异号）都没有溢出：直接返回
{
	if (0 == m_llValue || 0 == bigData.m_llValue)//返回0
	{
		return BigData(INT64(0));
	}

	if (!IsINT64Owerflow() && !bigData.IsINT64Owerflow())
	{
		if (m_strData[0] == bigData.m_strData[0])//同号用MAX_INT64,异号用MIN_INT64，不能两个直接乘因为乘出来会溢出
		{
			// 10 /2 = 5 >= 1 2 3 4 5
			// 10 /-2 = -5 <= -5 -4 -3 -2 -1 
			if (('+' == m_strData[0] && MAX_INT64 / m_llValue >= bigData.m_llValue) ||
				('-' == m_strData[0] && MAX_INT64 / m_llValue <= bigData.m_llValue))
			{
				return BigData(m_llValue*bigData.m_llValue);
			}
		}
		else
		{
			// -10 /2 = -5 <= 
			// -10/-2 = 5 >
			if (('+' == m_strData[0] && MIN_INT64 / m_llValue <= bigData.m_llValue) ||
				('-' == m_strData[0] && MIN_INT64 / m_llValue >= bigData.m_llValue))
			{
				return BigData(m_llValue*bigData.m_llValue);
			}
		}
	}

	return BigData(Mul(m_strData, bigData.m_strData).c_str());
}

BigData BigData::operator/(const BigData& bigData)//1.除数不能为0 2.两个都不溢出，结果也不会溢出，直接除  3.有1个溢出，就Div()
{
	if (0 == bigData.m_llValue)
	{
		assert("除数不能为0！");
		return BigData(INT64(0));
	}

	if (!IsINT64Owerflow() && !bigData.IsINT64Owerflow())
	{
		return BigData(m_llValue/bigData.m_llValue);
	}

	return BigData(Div(m_strData, bigData.m_strData).c_str());
}

// +
// += 
std::string BigData::Add(std::string left, std::string right)//将第一个数作为大的数，开辟Lsize+1大小,填写符号位，计算left[7]+right[7]->ret[8](有符号位)，left[7]中7是left[LSize-1]
{
	int iLSize = left.size();
	int iRSize = right.size();
	if (iLSize < iRSize)
	{
		std::swap(left, right);
		std::swap(iLSize, iRSize);
	}

	std::string strRet;
	strRet.resize(iLSize+1);
	strRet[0] = left[0];
	char cStep = 0;

	//left = "+9999999"  size = 9 
	// right="1"   "+10000000" 
	for (int iIdx = 1; iIdx < iLSize; ++iIdx)//计算left[7]+right[0]，放入ret[8]
	{
		char cRet = left[iLSize - iIdx] - '0' + cStep;

		if (iIdx < iRSize)//right位数没有了就不加
		{
			cRet += (right[iRSize - iIdx] - '0');
		}

		strRet[iLSize - iIdx + 1] = (cRet % 10 + '0');
		cStep = cRet/10;
	}

	strRet[1] = (cStep + '0');

	return strRet;
}

std::string BigData::Sub(std::string left, std::string right)//1.第一个数作为大的2.确定符号位3.从右往左直接只要right还有位数直接减4.若减出负的，前一位去借1（减掉1），该位+=10  5.保存结果
{
	// 1、左操作数 > 右操作数
	// 2、确定符号位
	int iLSize = left.size();
	int iRSize = right.size();
	char cSymbol = left[0];
	if (iLSize < iRSize || 
		(iLSize == iRSize && left < right))
	{
		std::swap(left, right);
		std::swap(iLSize, iRSize);
		if ('+' == cSymbol)
		{
			cSymbol = '-';
		}
		else
		{
			cSymbol = '+';
		}
	}

	std::string strRet;
	strRet.resize(iLSize);
	strRet[0] = cSymbol;

	// 逐位相减
	// 1、取left每一位，从后往前取
	// 2、在right没有超出  取right每一位 从后往前取
	// 3、直接相减
	// 4、 保存结果
	for (int iIdx = 1; iIdx < iLSize; iIdx++)
	{
		char cRet = left[iLSize - iIdx] - '0';
		if (iIdx < iRSize)
		{
			cRet -= (right[iRSize - iIdx] - '0');
		}

		if (cRet < 0)
		{
			left[iLSize - iIdx - 1] -= 1;
			cRet += 10;
		}

		strRet[iLSize - iIdx] = (cRet + '0');
	}

	return strRet;
}

std::string BigData::Mul(std::string left, std::string right)//1.2.确定符号位3.开辟lSize+RSize-1，放好符号位4.从最后一位乘，大于10进位，如果left那一位为0，前移
{
	int iLSize = left.size();
	int iRSize = right.size();

	if (iLSize > iRSize)
	{
		std::swap(left, right);
		std::swap(iLSize, iRSize);
	}
	//right是小的，把它放下面
	char cSymbol = '+';
	if (left[0] != right[0])
	{
		cSymbol = '-';
	}
	//right:1111
	//left:101

	std::string strRet;
	//strRet.resize(iLSize + iRSize - 1);
	strRet.assign(iLSize + iRSize - 1, '0');
	strRet[0] = cSymbol;
	int iDataLen = strRet.size();
	int iOffset = 0;

	for (int iIdx = 1; iIdx < iLSize; ++iIdx)
	{
		char cLeft = left[iLSize - iIdx] - '0';
		char cStep = 0;
		if (0 == cLeft)
		{
			iOffset++;
			continue;
		}

		for (int iRIdx = 1; iRIdx < iRSize; ++iRIdx)
		{
			char cRet = cLeft*(right[iRSize - iRIdx] - '0');
			cRet += cStep;
			cRet += (strRet[iDataLen - iOffset - iRIdx] - '0');
			strRet[iDataLen - iOffset - iRIdx] = cRet%10 + '0';
			cStep = cRet/10;
		}

		strRet[iDataLen - iOffset - iRSize] += cStep;
		iOffset++;
	}

	return strRet;
}

std::string BigData::Div(std::string left, std::string right)//1.确定符号位2.left<right返回0；除数为+-1,返回 3.该位左为0追加0，左小于右
{
	char cSymbol = '+';
	if (left[0] != right[0])
	{
		cSymbol = '-';
	}

	int iLSize = left.size();
	int iRSize = right.size();
	if (iLSize < iRSize ||
		iLSize == iRSize && strcmp(left.c_str()+1, right.c_str()+1) < 0)
	{
		return "0";
	}
	else
	{
		if ("+1" == right || "-1" == right)
		{
			left[0] = cSymbol;
			return left;
		}
	}

	std::string strRet;
	strRet.append(1, cSymbol);
	char *pLeft = (char*)(left.c_str()+1);
	char *pRight = (char*)(right.c_str()+1);
	int iDataLen = 1;
	iLSize -= 1;
	// "2422222222"  33
	for (int iIdx = 0; iIdx <iLSize;)
	{
		if ('0' == *pLeft)
		{
			strRet.append(1, '0');
			pLeft++;
			iIdx++;
			
			continue;
		}

		if (!IsLeftStrBig(pLeft, iDataLen, pRight, iRSize-1))//不是左边大于右边
		{
			strRet.append(1, '0');
			iDataLen++;
			if (iIdx + iDataLen > iLSize)
			{
				break;
			}
			continue;
		}
		else
		{
			// 循环相减
			strRet.append(1, SubLoop(pLeft, iDataLen, pRight, iRSize - 1));
		
			 // pLeft
			while('0' == *pLeft && iDataLen > 0)
			{
				pLeft++;
				iIdx++;
				iDataLen--;
			}

			iDataLen++;
			if (iIdx + iDataLen > iLSize)
			{
				break;
			}
		}
	}

	return strRet;
}

bool BigData::IsLeftStrBig(char *pLeft, size_t LSize, char *pRight, size_t RSize)
{
	assert(NULL != pLeft && NULL != pRight);
	if (LSize > RSize ||
		LSize == RSize && strncmp(pLeft, pRight, LSize) >= 0)
	{
		return true;
	}

	return false;
}

char BigData::SubLoop(char *pLeft, size_t LSize, char *pRight, size_t RSize)
{
	assert(NULL != pLeft && NULL != pRight);

	char cRet = '0';
	while(true)
	{
		if (!IsLeftStrBig(pLeft, LSize, pRight, RSize))
		{
			break;
		}

		// 做-=
		int iLDataLen = LSize - 1;
		int iRDataLen = RSize - 1;
		while(iRDataLen >= 0 && iLDataLen >= 0)
		{
			if (pLeft[iLDataLen] < pRight[iRDataLen])
			{
				pLeft[iLDataLen - 1] -= 1;
				pLeft[iLDataLen] += 10;
			}

			pLeft[iLDataLen] = pLeft[iLDataLen] - pRight[iRDataLen] + '0';
			iLDataLen--;
			iRDataLen--;
		}

		// "990000000000000000000000000099"
		while('0' == *pLeft && LSize > 0)
		{
			pLeft++;
			LSize--;
		}

		cRet++;
	}

	return cRet;
}

void BigData::INT64ToString()
{
	//12345
	char cSymbol = '+';
	INT64 temp = m_llValue;
	if (temp < 0)
	{
		cSymbol = '-';
		temp = 0 - temp;
	}

	m_strData.append(1, cSymbol);
	int iCount = 1;
	// 54321
	while(temp)
	{
		m_strData.append(1, temp%10 + '0');
		temp /= 10;
	}

	char *pLeft = (char*)(m_strData.c_str()+1);
	char *pRight = (char*)(m_strData.c_str()+m_strData.size() - 1);//c_str()指针指向字符串首位置，此时想要置首，所以用m_strData保存，m_strData.size()是+12345长度
	while(pLeft < pRight)
	{
		char ctemp = *pLeft;
		*pLeft++ = *pRight;
		*pRight-- = ctemp;
	}

	// 1 符号位
	// 2 m_strData = 54321
}

bool BigData::IsINT64Owerflow()const
{
	std::string strTemp;
	if ('+' == m_strData[0])
	{
		strTemp = "+9223372036854775807";
	}
	else
	{
		strTemp = "-9223372036854775808";
	}

	if (m_strData.size() > strTemp.size())
	{
		return true;
	}
	else if (m_strData.size() == strTemp.size() && m_strData > strTemp)
	{
		return true;
	}

	return false;
}

// std::ostream& operator<<(std::ostream& _cout, const BigData& bigData)
// {
// 	if (!bigData.IsINT64Owerflow()) // 没有溢出
// 	{
// 		_cout<<bigData.m_llValue;
// 	}
// 	else
// 	{
// 		char* pData = (char*)bigData.m_strData.c_str();
// 		if (pData[0] == '+')
// 		{
// 			pData++;
// 		}
// 		_cout<<pData;
// 	}
// 	return _cout;
// }

std::ostream& BigData::operator<<(std::ostream& _cout)
{
	_cout<<10;
	return _cout;
}
