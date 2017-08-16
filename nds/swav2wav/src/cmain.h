/**
 * cmain.h: a skeleton for cui application
 */


#ifndef CMAIN_H
#define CMAIN_H


#include "cioutil.h"


void cappShowUsage(void);
bool cappDispatchOptionChar(const char optChar);
bool cappDispatchOptionStr(const char* optString);
bool cappDispatchFilePath(const char* path);
int main(int argc, char* argv[]);


#endif /* !CMAIN_H */
