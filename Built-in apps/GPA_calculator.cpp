#include <iostream>
#include <windows.h>
using namespace std;

float point(string grade){
	if (grade.compare("A+")==0) return 4;
	else if (grade.compare("A")==0) return 4;
	else if (grade.compare("B+")==0) return 3.5;
	else if (grade.compare("B")==0) return 3;
	else if (grade.compare("C+")==0) return 2.5;
	else if (grade.compare("C")==0) return 2;
	else if (grade.compare("D+")==0) return 1.5;
	else if (grade.compare("D")==0) return 1;
	else if (grade.compare("F")==0) return 0;
	else return -1;
}
string help=
"----GPA Calculator----\n"
"Syntax:\n"
"Line 1: <Unsigned integer n: Number of subjects>\n"
"Next n lines: <Unsigned integer: Number of credits> <String: Grade>\n\n"
"- Valid grades: A+, A, B+, B, C+, C, D+, D, F\n"
"- n=0: Clears all previous grades\n"
"- n=-1: Exits program\n"
"- Invalid input causes the program to terminate.\n\n";

int main(){
	cout << help; float pointsum=0,creditsum=0;
	while(1){
		int n; cin >> n;
		if (cin.fail()||cin.eof()) return 0;
		else if (n<-1){
			cout << "Invalid input\n"; Sleep(500); return 0;
		}
		else if (n==-1) return 0;
		else if (n==0){
			pointsum=0; creditsum=0; cout << "Score reset\n";
		}
		for (int i=0; i<n; i++){
			float credits; string grade;
			cin >> credits;
			if (cin.fail()||credits<0){
				cout << "Invalid input\n"; Sleep(500); return 0;
			}
			cin >> grade;
			if (point(grade)!=-1){
				creditsum+=credits;
				pointsum+=point(grade)*credits;
			}
			else{
				cout << "Invalid input\n"; Sleep(500); return 0;
			}
		}
		printf("GPA: %.2f\n", pointsum/creditsum);
		cout << "Total Credits: " << creditsum << "\n";
	}
}
