#pragma once
#include <stdio.h>
#include <iostream>
#include <vector>
#include <math.h>

# define M_PI 3.14159265358979323846  /* pi */
using namespace std;

class TabishUtility
{
private:
    TabishUtility()
{

}
public:

	static void DegreeToRadian(double degree, double & radian)
	{
		radian = degree * M_PI / 180;
	}
static void Split(string str, char del,vector<string> & vec)
{
    char* worker = new char[str.size()];
    int tempCharCount=0;
    int i = 0;
    bool condition;
    do
    {
        if(str[i]==del || str[i] == '\0')
        {
            //store everything uptil now as one character, and clear working stack
            if(tempCharCount!=0)
            {
                worker[tempCharCount] = '\0';
                vec.push_back(worker);
                tempCharCount=0;
                worker[tempCharCount] = '\0';
            }
            continue;
        }
        else
        {
            worker[tempCharCount] = str[i];
            tempCharCount++;
        }
        condition = str[i]!='\0';
        i++;

    }while(condition);


    delete[] worker;

}

static void StringToInt(string str,int & in)
{
    in = 0;
    int factor = 1;
    for(int i=(str.size()-1);i>=0;i--)
    {
         if(int(str[i])>=48 && int(str[i])<=57)
        {
            in = in + (int(str[i])-48) * factor;
            factor = factor * 10;
        }
        else
        {

            throw invalid_argument("Int value not pure");
        }

    }
}

static void StringToFloat(string str,float & fl)
{
    fl = 0;
    float factor = 1;
    float in = 0;
    float saveFactor = 1;
    bool dotFound = false;
    for(int i=(str.size()-1);i>=0;i--)
    {
        if(str[i]=='.')
        {  //deal with dot
            if(!dotFound)
            {
                saveFactor = factor;
                dotFound = true;
            }
            else
            {
                throw invalid_argument("Float value not pure");
            }
        }
        else
        {
            if(int(str[i])>=48 && int(str[i])<=57)
            {
                in = in + (int(str[i])-48) * factor;
                factor = factor * 10;

            }
            else
            {

                throw invalid_argument("Float value not pure");
            }
        }

    }

    fl = in/saveFactor;
}

static void StringToDouble(string str,double & dl)
{
    dl = 0;
    double factor = 1;
    double in = 0;
    double saveFactor = 1;
    bool dotFound = false;
    for(int i=(str.size()-1);i>=0;i--)
    {
        if(str[i]=='.')
        {  //deal with dot
            if(!dotFound)
            {
                saveFactor = factor;
                dotFound = true;
            }
            else
            {
                throw invalid_argument("Float value not pure");
            }
        }
        else
        {
            if(int(str[i])>=48 && int(str[i])<=57)
            {
                in = in + (int(str[i])-48) * factor;
                factor = factor * 10;

            }
            else
            {

                throw invalid_argument("Float value not pure");
            }
        }

    }

    dl = in/saveFactor;
}


static void IntToString(int in, string & str)
{

    int len=0;
    int save = in;

    while((in/10)!=0)
    {
        in = in / 10;
        len++;
    }
    char* strSaver = new char[len+1];
    double greatestDig = pow(10,len);

    int minVal = 0;
    int counter = 0;
    bool condition = false;
    do
    {
        int sepDigit  = (save - (minVal*(greatestDig*10))) / greatestDig;
        strSaver[counter] = char(sepDigit+48);

        minVal= (minVal*10) +sepDigit;
        condition = (greatestDig ==1);
        greatestDig = greatestDig / 10;
        counter++;

    }
    while(!condition);
    strSaver[counter] = '\0';
    str = strSaver;
    delete[] strSaver;


}

static void FloatToString(float in, string & str,int prec=3)
{
    int numberMaker = in;
    double precision = pow(10,prec);

    string str1;
    IntToString(numberMaker,str1);


    float temp = (in - numberMaker)*precision;

    string str2;
    IntToString(ceil(temp),str2);

    while(str2.size()<prec)
    {
        str2 += "0";
    }
    str = str1 + "." + str2;

}






};
