#ifndef LICENCE_H
#define LICENCE_H

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "IPHLPAPI.lib")

#include <iostream>
#include <stdio.h>
#include <string>
//#include <windows.h>
//#include <iphlpapi.h>
//#include <Wincrypt.h>
#include <fstream>
#define MY_ENCODING_TYPE  (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING)

#include "Helper.h"

namespace FEA
{
class Licence
{
public:
	Licence();
	~Licence();

	RESULT check_licence();

private:
	std::string getmachineinfo();
	void writeRecord(bool is_infinite);
};
}


#endif