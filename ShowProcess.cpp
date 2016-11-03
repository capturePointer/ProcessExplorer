// ProcessFinder.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#include <stdio.h>

#include <tchar.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <set>


using namespace std;
typedef unsigned long PID;
//typedef const string& PR_PATH;

//This is procedure for starting the child process
bool launch_process(string &path, STARTUPINFO &info, PROCESS_INFORMATION &processInfo, HANDLE &hProcess, PID &pid)
{
	//setlocale(LC_ALL, "en_US.utf8");
	wstring widestr = wstring(path.begin(), path.end());
	const wchar_t *widecstr = widestr.c_str();
	LPTSTR szCmdline = _tcsdup(widecstr);
	if (CreateProcess(szCmdline, NULL, NULL, NULL, false, NULL, NULL, NULL, &info, &processInfo)){
		hProcess = processInfo.hProcess;
		pid = processInfo.dwProcessId;
		return true;
	}
	return false;
}

int PrintProcessList(HANDLE CONST hStdOut)
{
	PROCESSENTRY32 pe32;//structure for processes
	unordered_map<string, string> process_list_first;//map for the first snapshot
	unordered_map<string, string> process_list_second;//map for the second snapchot
	unordered_map<string, string>::iterator it1;//for the first map
	unordered_map<string, string>::iterator it2;//for the second map
	TCHAR szBuff[1024];//for reading pid and name of the processes from process structure
	char PID_M[1024];
	char NAME_M[1024];
	string PID_M_Str;//string equivalent of PID_M
	string NAME_M_Str;//string equivalent of NAME_M
	//wchar_t name_m[20];
	//wchar_t pid_m[20];
	const wchar_t *file_name;//file for opennig of deleting
	string file_name_str;//string equivalent of the thing above
	DWORD dtemp;
	//variables for program control
	bool flag = 1;
	int action = 0;
	int number;
	bool result = false;
	int map_size = 1;
	bool key = 1;

	//Process32First(hSnapShot, &pe32);
	while (1){
		cout << "Choose the action: " << endl;
		cout << "1 ----------- create the process with certain name" << endl;
		cout << "2 ----------- close the process with certain name" << endl;
		cout << "3 ----------- list all processes in system" << endl;
		cout << "4 ----------- terminate the program" << endl;
		//cout << "5 ----------- show child process" << endl;
		cin >> number;
		switch (number){
		case 1: {
			STARTUPINFO info = { sizeof(info) };
			PROCESS_INFORMATION processInfo;
			HANDLE hProcess;
			PID pid = 0;
			DWORD Parent_id;
			cout << "Enter the full path to file:" << endl;
			cin >> file_name_str;
			//result = launch_process("C:\\WINDOWS\\System32\\notepad.exe", info, processInfo, hProcess, pid);
			result = launch_process(file_name_str, info, processInfo, hProcess, pid);
			hProcess = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, false, static_cast<DWORD> (pid));

			//now create snapchot and show child and parent processes
			HANDLE CONST hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
			if (hSnapShot == INVALID_HANDLE_VALUE){
				return 0;
			}
			pe32.dwSize = sizeof(PROCESSENTRY32);
			//find the chald and parent ID
			if (Process32First(hSnapShot, &pe32) == TRUE){
				do{
					wsprintf(szBuff, L"%08X   %08X     %s\r\n", pe32.th32ProcessID, pe32.th32ParentProcessID, pe32.szExeFile);
					if (pe32.th32ProcessID == static_cast<DWORD> (pid)){
						Parent_id = pe32.th32ParentProcessID;
						cout << "This is the child process" << endl;
						cout << "Process ID  ParentProcess ID  Process Name" << endl;
						WriteConsole(hStdOut, szBuff, lstrlen(szBuff), &dtemp, NULL);
					}
				} while (Process32Next(hSnapShot, &pe32));
			}
			//find the parent
			if (Process32First(hSnapShot, &pe32) == TRUE){
				do{
					wsprintf(szBuff, L"%08X    %s\r\n", pe32.th32ProcessID, pe32.szExeFile);
					if (pe32.th32ProcessID == Parent_id){
						cout << "This is the parent process" << endl;
						cout << "Process ID          Process Name" << endl;
						WriteConsole(hStdOut, szBuff, lstrlen(szBuff), &dtemp, NULL);
					}
				} while (Process32Next(hSnapShot, &pe32));
			}


			CloseHandle(hSnapShot);
			CloseHandle(hProcess);
			break;
		}
		case 2:{
			cout << "Enter the name(without full path to the file): " << endl;
			//cin.getline(name,256);
			//file_name_str.assign(name);
			cin >> file_name_str;
			wstring wfile_name_str = wstring(file_name_str.begin(), file_name_str.end());
			file_name = wfile_name_str.c_str();
			HANDLE CONST hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
			if (hSnapShot == INVALID_HANDLE_VALUE){
				return 0;
			}
			pe32.dwSize = sizeof(PROCESSENTRY32);
			if (Process32First(hSnapShot, &pe32) == TRUE){
				do{
					wsprintf(szBuff, L"%08X   %s\r\n", pe32.th32ProcessID, pe32.szExeFile);
					if (_tcsicmp(pe32.szExeFile, file_name) == 0){
						cout << "Process ID     Process Name" << endl;
						WriteConsole(hStdOut, szBuff, lstrlen(szBuff), &dtemp, NULL);
						HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
						TerminateProcess(hProc, 0);
						CloseHandle(hProc);
						cout << file_name_str << " is closed by user" << endl;
					}
				} while (Process32Next(hSnapShot, &pe32));
			}
			CloseHandle(hSnapShot);
			break;
		}
		case 3:{
			//PID_M = new char[20];
			//NAME_M = new char[20];
			HANDLE CONST hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
			if (hSnapShot == INVALID_HANDLE_VALUE){
				return 0;
			}
			pe32.dwSize = sizeof(PROCESSENTRY32);
			if (Process32First(hSnapShot, &pe32) == TRUE){
				cout << "The list of processes: " << endl;
				cout << "Process ID     Process Name" << endl;
				do{
					wsprintf(szBuff, L"%08X     %s\r\n", pe32.th32ProcessID, pe32.szExeFile);
					sprintf(PID_M, "%08X", pe32.th32ProcessID);
					//wcstombs(PID_M, pe32.th32ProcessID, sizeof(pe32.szExeFile));
					//strcpy_s(NAME_M, 20, (char*)pe32.szExeFile);
					wcstombs(NAME_M, pe32.szExeFile, sizeof(pe32.szExeFile));
					//sprintf(NAME_M, "%s",pe32.szExeFile);
					//					name_m=wstring(PID_M).c_str;
					PID_M_Str = (string)PID_M;
					NAME_M_Str = (string)NAME_M;
					if (action == 2){//if we already have two maps, we can rotate them and fill the second map
						if (key){
							process_list_first.clear();
							process_list_first = process_list_second;
							process_list_second.clear();
							process_list_second.insert(pair<string, string>(PID_M_Str, NAME_M_Str));
							key = 0;
						}
						else
						{
							process_list_second.insert(pair<string, string>(PID_M_Str, NAME_M_Str));
						}

					}
					else{
						if (flag){//for the first map
							process_list_first.insert(pair<string, string>(PID_M_Str, NAME_M_Str));

						}
						else{//for the second map
							process_list_second.insert(pair<string, string>(PID_M_Str, NAME_M_Str));

						}
					}
					WriteConsole(hStdOut, szBuff, lstrlen(szBuff), &dtemp, NULL);


				} while (Process32Next(hSnapShot, &pe32));
				key = 1;//for next iteration

			}
			CloseHandle(hSnapShot);
			if (flag == 0){//in the first iteration we need to fill the map #1
				//map_size = 0;
				//find new process in the system
				if (process_list_first.size() < process_list_second.size()){//if we have new process
					for (it1 = process_list_second.begin(); it1 != process_list_second.end(); it1++){
						it2 = process_list_first.find(it1->first);
						if (it2 == process_list_first.end()){//if find some process which is not in previous snapshot
							cout << "The new process in the system is: " << endl;
							cout << it1->first << "    " << it1->second << endl;
						}
					}
					/*
					for (it1 = process_list_first.begin(); it1 != process_list_first.end(); it1++){
					map_size++;
					for (it2 = process_list_second.begin(); it2 != process_list_second.end(); it2++){

					if (it2->first == it1->first) {
					//process_list_first.erase(it1->first);
					//continue;
					break;
					}
					if (it2->first != it1->first && (process_list_first.size()-1)==map_size){
					cout << "The new process in the system is: " << endl;
					cout << it2->first << "    " << it2->second << endl;
					}

					}
					}
					*/
				}
				//if there are some deleted processes
				if (process_list_first.size() > process_list_second.size()){
					for (it1 = process_list_first.begin(); it1 != process_list_first.end(); it1++){
						it2 = process_list_second.find(it1->first);
						if (it2 == process_list_second.end()){
							cout << "The old process in the system is: " << endl;
							cout << it1->first << "    " << it1->second << endl;
						}
					}
				}

				/*
				if (process_list_first.size() > process_list_second.size()){
				for (it1 = process_list_second.begin(); it1 != process_list_second.end(); it1++){
				for (it2 = process_list_first.begin(); it2 != process_list_first.end(); it2++){
				map_size++;
				if (it2->first == it1->first) continue;
				if (it2->first != it1->first && map_size == process_list_second.size()){
				cout << "The terminated process in the system is: " << endl;
				cout << it2->first << "    " << it2->second << endl;
				}
				}
				}
				}
				*/
			}
			/*
			cout << "//----------------------//--------------------------//" << endl;
			cout << "Processes from the map" << endl;
			for (it1 = process_list_first.begin(); it1 != process_list_first.end(); it1++){
			cout << it1->first << "    " << it1->second << endl;
			}
			*/
			cout << "//----------------------//--------------------------//" << endl;
			if (action<2) action++;
			if (flag) {
				flag = 0;
			}
			else flag = 1;
			if (action == 2) flag = 0;
			//action = 0;
			//else flag = 1;
			//for (int i = 0; i < 20;i++)
			//	PID_M[i] = 0 ;
			//delete []PID_M;
			//delete []NAME_M;
			break;
		}
		case 4:{
			return 0;
			break;

		}
		}



		/*


		if (_tcsicmp(pe32.szExeFile, str) == 0){

		WriteConsole(hStdOut, szBuff, lstrlen(szBuff), &dtemp, NULL);

		HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
		//flag = 0;
		cout << "Notepad is opened" << endl;
		if (action == 0){
		TerminateProcess(hProc, 0);
		CloseHandle(hProc);
		cout << "Notepad is closed by user" << endl;
		}
		}

		}
		}
		//---------------------------------------//
		wsprintf(szBuff, L"----%08X %s ----\r\n", pe32.th32ProcessID, pe32.szExeFile);
		if (_tcsicmp(pe32.szExeFile, str) == 0){

		WriteConsole(hStdOut, szBuff, lstrlen(szBuff), &dtemp, NULL);

		HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
		//flag = 0;
		cout << "Notepad is opened" << endl;
		if (action == 0){
		TerminateProcess(hProc, 0);
		CloseHandle(hProc);
		cout << "Notepad is closed by user"<<endl;
		}
		//cout << pe32.szExeFile << endl;
		//cout << "Other processes" << endl;

		//cout << "Notepad is closed" << endl;
		}
		else
		{
		if (flag == 0){
		cout << "Notepad is closed" << endl;
		flag = 1;
		}
		}


		//} while (Process32Next(hSnapShot, &pe32));


		CloseHandle(hSnapShot);
		cin >> action;
		Sleep(5000);
		cout << "The last line" << endl;
		//---------------------//
		}
		*/

	}



}


int _tmain(int argc, _TCHAR* argv[])
{
	//wchar_t *str = _T("notepad++.exe");

	HANDLE CONST hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (PrintProcessList(hStdOut) == 0){
		CloseHandle(hStdOut);
		ExitProcess(0);
	}
	/*
	HANDLE hSnap = NULL;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(entry);
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hSnap != NULL){
	if (Process32First(hSnap, &entry) == TRUE){
	do{

	//HANDLE hHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ParentProcessID);
	cout << entry.th32ProcessID << endl;


	} while (Process32Next(hSnap, &entry)==TRUE);
	}

	}
	CloseHandle(hSnap);
	return 0;
	*/
	/*STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	if (argc != 2){
	cout << "Too few arguments";
	return 0;
	}
	if (!CreateProcess(NULL,
	argv[1],
	NULL,
	FALSE,
	0,
	NULL,
	NULL,
	&si,
	&pi)){
	cout << "Can not create the process";
	return 1;
	}
	WaitForSingleObject(pi.hProcess);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	*/
}


