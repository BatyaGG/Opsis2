#include "Licence.h"

std::string junk1 = "BGKA13A5CF";
std::string junk2 = "4DE6QAB603";

//#include "stdafx.h"

using namespace std;
using namespace FEA;


//Expiry check
string key = "snldkjfhbweiufhoijdop123098377wqjdemcjdasidjqoijeiojud1982u38oiu4j";
//string key = "abcdefghijklmno";

struct tm expiry;
std::string mId;

//LPCSTR licence_filename = "licence_adsc";


Licence::Licence()
{
}


Licence::~Licence()
{
}

std::string Licence::getmachineinfo()
{
	return("");
}


//Write
void Licence::writeRecord(bool is_infinite)
{
}


FEA::RESULT Licence::check_licence()
{
	return OK;
}
