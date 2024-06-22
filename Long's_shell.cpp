#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <windows.h>
using namespace std;

typedef struct process{
	string name;
	string status;
	PROCESS_INFORMATION pi;
}process;
vector <process> running(0);
vector <string> history(0);
vector <string> waiting_queries(0);
vector <string> paths={"","Built-in apps\\"};
map <string, string> variables;
int querynum;
string query[128],batch="Allow",prompt=variables["prompt"]="Long's_shell\\>",path=variables["path"]="";
//Processes attached to console
DWORD processList[100];
DWORD processCount = GetConsoleProcessList(processList, 100);
DWORD defaultProcessCount = processCount;
		
//Extract parameters from input
void parameters(string s, char delimiter){
	for (int i=0; i<128; i++) query[i]="";
	int quotesmode=0,num=0;
	for (int k=0; k<s.length(); k++){
		if ((s[k]!=delimiter&&s[k]!='"'&&s[k]!='\\')||(s[k]==delimiter&&quotesmode==1)) query[num].push_back(s[k]);
		else if (s[k]==delimiter&&(!quotesmode)){
			if (k>0) num++;
			while(s[k]==delimiter) k++;
			k--;
		}
		else if (s[k]=='"') quotesmode=1-quotesmode;
		else if (s[k]=='\\'){
			int l=0;
			while (s[k]=='\\'&&k<s.length()) k++,l++;
			if (k>=s.length()) for (int i=0; i<l; i++) query[num].push_back('\\');
			else if (s[k]!='"'){
				for (int i=0; i<l; i++) query[num].push_back('\\');
				query[num].push_back(s[k]);
			}
			else{
				for (int i=0; i<l/2; i++) query[num].push_back('\\');
				if (l%2==1) query[num].push_back('"');
				else quotesmode=1-quotesmode;
			}
		}
	}
	querynum=num;
}

//cls
void cls(HANDLE hConsole){
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    SMALL_RECT scrollRect;
    COORD scrollTarget;
    CHAR_INFO fill;
    // Get the number of character cells in the current buffer.
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;
    // Scroll the rectangle of the entire buffer.
    scrollRect.Left = 0;
    scrollRect.Top = 0;
    scrollRect.Right = csbi.dwSize.X;
    scrollRect.Bottom = csbi.dwSize.Y;
    // Scroll it upwards off the top of the buffer with a magnitude of the entire height.
    scrollTarget.X = 0;
    scrollTarget.Y = (SHORT)(0 - csbi.dwSize.Y);
    // Fill with empty spaces with the buffer's default text attribute.
    fill.Char.UnicodeChar = TEXT(' ');
    fill.Attributes = csbi.wAttributes;
    // Do the scroll
    ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, scrollTarget, &fill);
    // Move the cursor to the top left corner too.
    csbi.dwCursorPosition.X = 0;
    csbi.dwCursorPosition.Y = 0;
    SetConsoleCursorPosition(hConsole, csbi.dwCursorPosition);
}

//color
int color(string query){
	if (!query.length()) query="07";
	int q,q1,q2;
	try{
		q=stoi(query,0,16);
		if(q<0||q>255) q=-1;
	} catch(exception &e){
		q=-1;
	}
	if (q==-1){
		cout << "0 = Black       8 = Gray\n"
   				"1 = Blue        9 = Light Blue\n"
    			"2 = Green       A = Light Green\n"
    			"3 = Aqua        B = Light Aqua\n"
    			"4 = Red         C = Light Red\n"
    			"5 = Purple      D = Light Purple\n"
   				"6 = Yellow      E = Light Yellow\n"
    			"7 = White       F = Bright White\n";
	}
	else{
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		COORD coord = {0,0};
		DWORD dwWritten;
		if (q/16!=q%16){
			SetConsoleTextAttribute(hConsole,q);
			::FillConsoleOutputAttribute(hConsole,q,INFINITE,coord,&dwWritten);
		}
		else cout << "Cannot set foreground and background to same color\n";
	}
}

//date, time
string DateAndTime(char query){
	string weekday[7]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
	time_t now = time(0);
	tm *ltm = localtime(&now);
	if(query=='y') return to_string(1900+ltm->tm_year);
	else if(query=='m') return (ltm->tm_mon<9)? ("0"+to_string(1+ltm->tm_mon)) : to_string(1+ltm->tm_mon);
	else if(query=='d') return (ltm->tm_mday<9)? ("0"+to_string(ltm->tm_mday)) : to_string(ltm->tm_mday);
	else if(query=='w') return weekday[ltm->tm_wday];
	else if(query=='H') return (ltm->tm_hour<10)? ("0"+to_string(ltm->tm_hour)) : to_string(ltm->tm_hour);
	else if(query=='M') return (ltm->tm_min<10)? ("0"+to_string(ltm->tm_min)) : to_string(ltm->tm_min);
	else if(query=='S') return (ltm->tm_sec<10)? ("0"+to_string(ltm->tm_sec)) : to_string(ltm->tm_sec);
}

//fg
void fg(string x){
	auto it=running.begin();
	try{
		int pid=stoi(x);
		while (it!=running.end()&&it->pi.dwProcessId!=pid) it++;
		if(it==running.end()) cout << "Process with ID " << x << " not found\n";
		else{
			if (!it->status.compare("running")){
				it->status="running in foreground";
				WaitForSingleObject(it->pi.hProcess, INFINITE);
			}
		}
	}catch(exception &e){
		cout << "Process with ID " << x << " not found\n";
	}
}

//help, type
void printfile(string filepath, string prefix){ //prefix!="": only prints lines with prefix
	ifstream file(filepath.c_str());
	string ln,check="0";
	if (!file.is_open()) cout << "Cannot find the file '" << filepath << "'\n";
	else{
		if (!prefix.compare("")) while(getline(file, ln)) cout << ln << "\n";
		else{
			while(getline(file, ln)){
				if (!ln.rfind(prefix,0)){
					cout << ln << "\n";
					check="1";
				}
			}
			if (!check.compare("0")) cout << "Cannot find the line '" << prefix << "'\n";
		}
	}
	file.close();
}

//history
void History(string x){
	if (!x.compare("")){
		if (!history.size()) cout << "History empty\n";
		else for (int i=0; i<history.size(); i++) cout << i << ": " << history[i] << "\n";
	}
	else if(!x.compare("/clear")){
		history.clear();
		cout << "History cleared\n";
	}
	else{
		cout << "Incorrect syntax\n";
		printfile("help.txt","history");
	}
}

//manage processes
void manageprocess(){
	auto it=running.begin();
	while(it!=running.end()){
		DWORD exitCode=0;
		if (GetExitCodeProcess(it->pi.hProcess,&exitCode)) (exitCode!=STILL_ACTIVE)? running.erase(it) : it++;
		else it++;
	}
}

//path
void Path(string x){
	x.erase(0,5);
	int check=0;
	for (int i=1; i<x.length(); i++) if (x[i]!=32) check=1;
	if (!x.compare("")) cout << "path=" << path << "\n";
	else if (x[0]==59&&(!check)){
		paths.clear();
		paths.push_back("");
		paths.push_back("Built-in apps\\");
		path=variables["path"]="";	
	}
	else{
		string x1=path;
		for (int k=0; k<x.length(); k++) if(k<=x.length()-6&&!x.compare(k,6,"%PATH%")) x.replace(k,6,x1);
		path=variables["path"]=x;
		parameters(x,';');
		paths.clear();
		paths.push_back("");
		paths.push_back("Built-in apps\\");
		for (int i=0; i<=querynum; i++) paths.push_back(query[i]+"\\");
	}
}

//run
void run(string x, string mode){
	string ln, extention = (x.length()>=4)? x.substr(x.length()-4,4) : x;
	if(!extention.compare(".bat")){
		ifstream file(x.c_str());
		if (!file.is_open()) cout << "Cannot find the file '" << x << "'\n";
		else{
			int num=0;
			while(getline(file, ln)){
				waiting_queries.push_back(ln);
				num++;
			}
			reverse(waiting_queries.end()-num,waiting_queries.end());
		}
		file.close();
	}
	else if(!extention.compare(".exe")){
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb=sizeof(si);
		auto it=paths.begin();
		while(it!=paths.end()&&!CreateProcess((*it+x).c_str(),NULL,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi)) it++;
		if (it==paths.end()) cout << "Failed to open " << x << "\n";
		else{
			Sleep(10);
			processCount = GetConsoleProcessList(processList, 100);
			if (processCount>defaultProcessCount) mode="/wait";
			process p;
			p.name=x;
			p.pi=pi;
			running.push_back(p);
			running.back().status= (!mode.compare("/wait"))? "running in foreground" : "running";
			if (!mode.compare("/wait")) WaitForSingleObject(pi.hProcess, INFINITE);
		}
	}
	else cout << x << " is not an operable program or batch file\n";
}

//set
void set(string x, string y){
	if (!x.compare("")) for (auto &it:variables) cout << it.first << "=" << it.second << "\n";
	else if (!x.compare("/p")){
		int found = y.find("=");
		if (found==-1){
			cout << "Incorrect syntax\n";
			printfile("help.txt","set");
		}
		else{
			int check=(y[0]>=65&&y[0]<=90)||y[0]==95||(y[0]>=97&&y[0]<=122);
			for (int i=1; i<found; i++) check=(y[0]>=48||y[0]<=57)||(y[0]>=65&&y[0]<=90)||y[0]==95||(y[0]>=97&&y[0]<=122);
			if (!check){
				cout << "Incorrect syntax\n";
				printfile("help.txt","set");
			}
			else{
				string key=y.substr(0,found),value,prompt=y.substr(found+1,y.length());
				cout << prompt;
				getline(cin,value);
				if (cin.fail()||cin.eof()){
					cout << "Incorrect syntax\n";
					printfile("help.txt","set");
				}
				else{
					variables[key]=value;
					if (!key.compare("prompt"))	prompt=value;
				}
			}
		}
	}
	else{
		int found = x.find("="),check=0;
		if (found==-1){
			for (auto &it:variables){
				if (!it.first.rfind(x,0)){
					cout << it.first << "=" << it.second << "\n";
					check=1;
				}
			}
			if (!check) cout << "Environment variable " << x << " not defined\n";
		}
		else{
			string key=x.substr(0,found),value=x.substr(found+1,x.length());
			if (!value.compare("")){
				variables.erase(key);
				if (!key.compare("path")) path="";	
			}
			else variables[key]=value;
		}
	}
}

//start
void start(string x, string mode){
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb=sizeof(si);
	auto it=paths.begin();
	while(it!=paths.end()&&!CreateProcess((*it+x).c_str(),NULL,NULL,NULL,FALSE,CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi)) it++;
	if (it==paths.end()) cout << "Failed to open " << x << "\n";
	else{
		process p;
		p.name=x;
		p.pi=pi;
		running.push_back(p);
		running.back().status= (!mode.compare("/wait"))? "running in foreground" : "running";
		if (!mode.compare("/wait")) WaitForSingleObject(pi.hProcess, INFINITE);
	}
}

//taskkill, taskpause, taskresume
void task(string query, string x, string y){
	auto it=running.begin();
	int pid,check=0;
	for (int i=0; i<x.length(); i++) if(x[i]>=65&&x[i]<=90) x[i]+=32;
	if(!y.compare("")||(x.compare("/pid")&&x.compare("/im"))){
		cout << "Incorrect syntax\n";
		printfile("help.txt","task"+query);
	}
	else if (!x.compare("/pid")){
		try{
			pid=stoi(y);
			while (it!=running.end()&&it->pi.dwProcessId!=pid) it++;
			if(it==running.end()) cout << "Process with ID " << y << " not found\n";
			else{
				if (!query.compare("kill")){
					TerminateProcess(it->pi.hProcess,0);
					CloseHandle(it->pi.hProcess);
    				CloseHandle(it->pi.hThread);
				}
				else if(!query.compare("pause")){
					SuspendThread(it->pi.hThread);
					it->status="paused";
				}
				else if(!query.compare("resume")){
					ResumeThread(it->pi.hThread);
					it->status="running";
				}
    			cout << "Process '" << it->name << "' with ID " << y << " " << query <<  "ed\n";
    			if (!query.compare("kill")) running.erase(it);
			}
		}catch(exception &e){
			cout << "Process with ID " << y << " not found\n";
		}
	}
	else if(!x.compare("/im")){
		while (it!=running.end()){
			if(!it->name.compare(y)){
				if (!query.compare("kill")){
					TerminateProcess(it->pi.hProcess,0);
					CloseHandle(it->pi.hProcess);
    				CloseHandle(it->pi.hThread);
    				running.erase(it);
				}
				else if(!query.compare("pause")){
					SuspendThread(it->pi.hThread);
					it->status="paused";
					it++;
				}
				else if(!query.compare("resume")){
					ResumeThread(it->pi.hThread);
					it->status="running";
					it++;
				}
   				check=1;
			}
			else it++;
		}
		if(!check) cout << "Process with name '" << y << "' not found\n";
		else cout << "Processes with name '" << y << " " << query <<  "ed\n";
	}
}

//tasklist
void tasklist(string stat){
	cout << "Process Name                   PID Status\n"
			"========================= ======== =====================\n";
	for (int i=0; i<stat.length(); i++) if(stat[i]>=65&&stat[i]<=90) stat[i]+=32;
	for (int i=0; i<running.size(); i++){
		if ((!stat.compare(""))||(!running[i].status.compare(stat))){
			cout << running[i].name;
			if (running[i].name.length()<25) for (int j=0; j<25-running[i].name.length(); j++) cout << " ";
			else for (int j=0; j<running[i].name.length()-25; j++) cout << "\b";
			for (int j=0; j<9-to_string(running[i].pi.dwProcessId).length(); j++) cout << " ";
			cout << running[i].pi.dwProcessId << " " << running[i].status << "\n";
		}
	}
}

//exit
void quit(string x){
	if (!x.compare("/t")) while(!running.empty()) task("kill","/pid", to_string(running.begin()->pi.dwProcessId));
	exit(0);
}

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType){
	auto it=running.begin();
	switch (fdwCtrlType){
    case CTRL_C_EVENT:
    	if (!waiting_queries.empty()){
    		string inp;
			batch="Not allow";
			Sleep(10);
    		cout << "Terminate batch job (Y/N)?";
    		getline(cin,inp);
    		if (!inp.compare("Y")||!inp.compare("y")) waiting_queries.clear();
    		batch="Allow";
		}
		while(it!=running.end()){
			if(!(it->status).compare("running in foreground")) task("kill","/pid", to_string(it->pi.dwProcessId));
			else it++;
		}
    	return TRUE;
    default:
        return FALSE;
    }
}

int main(){
	cout << "Long's shell [Version 1.0.0.0]\n";
	string inp,echo="on";
	SetConsoleCtrlHandler(CtrlHandler, TRUE);
	while(1){
		if (variables.find("prompt")!=variables.end()) prompt=variables["prompt"];
		if (waiting_queries.empty()){
			if (!echo.compare("on")) cout << prompt;
			getline(cin, inp);
			manageprocess();
			if (cin.fail()||cin.eof()) quit("");
		}
		else{
			while (batch.compare("Allow"));
			if (waiting_queries.empty()){
				inp="rem";
				manageprocess();
			}
			else{
				inp=waiting_queries.back();
				waiting_queries.pop_back();
				if (!echo.compare("on")&&(inp.substr(0,4)).compare("rem ")) cout << prompt << inp << "\n";
				manageprocess();
			}
		}
		//Validate and extract parameters from input
		if (inp.length()>256){
			cout << "Not recognized as a command, operable program or batch file\n";
			continue;
		}
		parameters(inp,' ');
		for (int i=0; i<query[0].length(); i++) if(query[0][i]>=65&&query[0][i]<=90) query[0][i]+=32;
		
		//cls (Clears screen)
		if(!query[0].compare("cls")) cls(GetStdHandle(STD_OUTPUT_HANDLE));
		
		//color (Sets console background, foreground colors)
		else if(!query[0].compare("color")) color(query[1]);
		
		//date (Displays date in dd/mm/yyyy format)
		else if(!query[0].compare("date")) cout << "The current date (dd/mm/yyyy) is: " << DateAndTime('w') << " " << DateAndTime('d') << "/" << DateAndTime('m') << "/" << DateAndTime('y') << "\n";
		
		//echo (Shows echo status, turns command echoing on/off or displays messages)
		else if(!query[0].compare("echo")){
			if(!query[1].compare("")) cout << "Echo is " << echo << "\n";
			else if(!query[1].compare("on")||!query[1].compare("off")) echo=query[1];
			else cout << query[1] << "\n";
		}

		//exit (Quits shell. /t quits child processes)
		else if (!query[0].compare("exit")) quit(query[1]);
		
		//fg (Moves a background process to foreground)
		else if (!query[0].compare("fg")) fg(query[1]);
			
		//help (Prints Help information)
		else if(!query[0].compare("help")) printfile("help.txt",query[1]);
		
		//history (Displays 69 recent commands, excluding history. /clear to clear history)
		else if(!query[0].compare("history")) History(query[1]);
		
		//pause (Suspends processing of a batch file and displays a message)
		else if(!query[0].compare("pause")){
			cout << "Press Enter to continue...";
			getline(cin,inp);
		}
		
		//path (Displays or sets a search path for executable files)
		else if(!query[0].compare("path")) Path(inp);
		
		//prompt (Changes Shell command prompt)
		else if(!query[0].compare("prompt")) prompt=variables["prompt"]=(!query[1].compare(""))? "Long's_shell\\>" : query[1];
		
		//rem (Indicates a comment line in a batch file)
		else if(!query[0].compare("rem"));
		
		//run (Runs a program or batch (.bat) file)
		else if(!query[0].compare("run")){
			if(!query[1].compare("")) query[1]="Long's_shell.exe";
			run(query[1],query[2]);	
		}
		
		//set (Displays, sets environmental variable var. /p gets user input, empty /text removes var)
		else if(!query[0].compare("set")) set(query[1],query[2]);
		
		//start (Starts a process in a new console. /wait starts in foreground mode)
		else if(!query[0].compare("start")){
			if(!query[1].compare("")) query[1]="Long's_shell.exe";
			start(query[1],query[2]);
		}
		
		//taskkill (Stops a process. /pid for process ID, /im for process name)
		else if(!query[0].compare("taskkill")) task("kill",query[1],query[2]);
		
		//taskpause (Pauses a process. /pid for process ID, /im for process name)
		else if(!query[0].compare("taskpause")) task("pause",query[1],query[2]);
		
		//taskresume (Resumes a process. /pid for process ID, /im for process name)
		else if(!query[0].compare("taskresume")) task("resume",query[1],query[2]);
		
		//tasklist (Displays all running and paused processes)
		else if (!query[0].compare("tasklist")) tasklist(query[1]);
		
		//time (Displays time in 24-hour format)
		else if(!query[0].compare("time")) cout << "The current time is: " << DateAndTime('H') << ":" << DateAndTime('M') << ":" << DateAndTime('S') << "\n";
		
		//title (Creates title for Shell window)
		else if (!query[0].compare("title")){
			if (!SetConsoleTitle(query[1].c_str())) cout << "Failed to set title to " <<  query[1] << "\n";
			else cout << "Console title set to " << query[1] << "\n";
		}
		
		//type (Displays contents of a text file)
		else if(!query[0].compare("type")){
			if(!query[1].compare("")){
				cout << "Incorrect syntax\n";
				printfile("help.txt","type");
			}
			else printfile(query[1],"");
		}
		
		//bad command
		else cout << "'" << query[0] << "' is not recognized as a command, operable program or batch file\n";
		
		//Update history
		if (query[0].compare("history")) history.push_back(inp);
		if(history.size()>69) history.erase(history.begin());
	}
}
