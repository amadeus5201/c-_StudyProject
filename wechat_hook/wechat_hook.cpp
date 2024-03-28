#include "pch.h"
#include "wechat_hook.h"
#include <Windows.h>
#include "resource.h"
#include <TlHelp32.h>
#include <stdio.h>
#include <string>
#define WECHAT_PROCESSNAME  "WeChat.exe"

INT_PTR CALLBACK DialogP(HWND unnamedParam1, UINT unnamedParam2, WPARAM unnamedParam3, LPARAM unnamedParam4);
DWORD FindPIDByProcessName(LPCSTR processName);
VOID InjectDLL();
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {

	DialogBox(hInstance, MAKEINTRESOURCE(DIalog_1), NULL, &DialogP);

	return 0;
}

INT_PTR CALLBACK DialogP(HWND unnamedParam1, UINT unnamedParam2, WPARAM unnamedParam3, LPARAM unnamedParam4) {

	if (unnamedParam2 == WM_INITDIALOG) {
		MessageBox(NULL, "第一次加载", "标题", 0);
	}

	if (unnamedParam2 == WM_CLOSE) {
		MessageBox(NULL, "关闭", "关闭", 0);
		EndDialog(unnamedParam1, 0);
	}
	//界面上的按钮都会都这个事件
	if (unnamedParam2 == WM_COMMAND) {
		//unnamedParam3 == 点击的参数
		//注入
		if (unnamedParam3 == IDDLL) {
			MessageBox(NULL, "注入", "标题", 0);
			InjectDLL();
		}
		if (unnamedParam3 == UNDLL) {

		}
	}
	return FALSE;
}
// 1.读取微信句柄，有句柄才能操作微信内存
//		1)通过进程名，找到微信Pid,通过Pid找到微信句柄
// 
DWORD FindPIDByProcessName(LPCSTR processName) {
	//获取整个系统的内存快照
	HANDLE  processAll = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	PROCESSENTRY32 processInfo = { sizeof(PROCESSENTRY32) };	// 用来接收 hProcessSnap 的信息
	if (Process32First(processAll, &processInfo)) {
		do {
			//WeChat.exe
			//szExeFile进程名称

			if (strcmp(processName, processInfo.szExeFile) == 0) {
				return processInfo.th32ProcessID;
			}

		} while (Process32Next(processAll, &processInfo));
	}
	return 0;
}
// 
// 2.在微信内部申请内存，存放自己的DLL 路径

VOID InjectDLL() {
	CHAR dllPath[0x100] = { "E:\\Desktop\\aa.txt" };
	DWORD  PID = FindPIDByProcessName(WECHAT_PROCESSNAME);
	if (PID == 0) {
		MessageBox(NULL, "没找到微信进程", "error", 0);
		return;
	}
	HANDLE cProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	if (NULL == cProcess) {
		MessageBox(NULL, "进程打开失败", "error", 0);
		return;
	}
	//申请内存
	LPVOID baseAdd = VirtualAllocEx(cProcess, NULL, sizeof(dllPath), MEM_COMMIT, PAGE_READWRITE);
	if (baseAdd == NULL) {
		MessageBox(NULL, "申请内存失败", "error", 0);
		return;
	}
	//写入地址
	BOOL success = WriteProcessMemory(cProcess, baseAdd, dllPath, sizeof(dllPath), NULL);
	if (success) {
		MessageBox(NULL, "写入内存成功", "成功", MB_OK | MB_ICONINFORMATION);
	}
	char path[0x100] = { 0 };
	sprintf_s(path, "写入的地址为： %p", baseAdd);
	OutputDebugString(path);

	HMODULE k32 = GetModuleHandle("kernel32.dll");
	FARPROC fa = GetProcAddress(k32, "LoadLibraryW");






}
// 3.写入DLL路径。通过远程线程，执行loadLiaray，加载自己的DLL