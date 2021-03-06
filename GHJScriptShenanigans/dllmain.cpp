// dllmain.cpp : Definiert den Einstiegspunkt für die DLL-Anwendung.
#include "stdafx.h"
#include <ChakraCommon.h>
#include <thread>
#include "Native.h"

JsContextRef scriptContext;

JCallback OnUiCallback(L"OnUi");

void OnUi()
{
	JsSetCurrentContext(scriptContext);

	JsValueRef undefined;
	JsGetUndefinedValue(&undefined);
	OnUiCallback.Call(scriptContext, { undefined });
}


DWORD HkUiCall = 0x00475CE0;
DWORD HkUiJmp = 0x00644280;

__declspec(naked) void HkUi()
{
	__asm
	{
		CALL[HkUiCall]
		PUSHAD;
		PUSHFD;
	}
	OnUi();
	__asm
	{
		POPFD;
		POPAD;
		JMP[HkUiJmp]
	}
}

DWORD WINAPI JsThread(LPVOID arg)
{
	MessageBoxA(NULL, "Injected", "Injected", 0);

	JsRuntimeHandle runtime;

	JsCreateRuntime(JsRuntimeAttributeNone, nullptr, &runtime);

	JsCreateContext(runtime, &scriptContext);

	JsSetCurrentContext(scriptContext);

	JExports::JInit(scriptContext);

	unsigned int scriptCookie = 0;

	MessageBoxW(NULL, JScriptManager::ReadJScript(L"app.js").c_str(), L"Script", 0);


	JsRunScript(JScriptManager::ReadJScript(L"app.js").c_str(), scriptCookie, L"", NULL);
	//JsValueRef exception;
	//JsGetAndClearException(&exception);

	//MessageBoxA(NULL, JString(exception).ToCharArray(), "", 0);
	//auto x = JsRunScript(JScriptManager::ReadJScript(L"app.js").c_str(), scriptCookie, L"", NULL);

	//if (x != JsNoError)
	//{
		//char buffer[64];
		//sprintf_s(buffer, "ERROR: %i", x);
		//MessageBoxA(NULL, buffer, "KANK", 0);
	//}

	JsSetCurrentContext(nullptr);

	Native::DetourFunction(reinterpret_cast<BYTE*>(0x0064427B), reinterpret_cast<BYTE*>(HkUi), 0x5);

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(NULL, NULL, &JsThread, NULL, NULL, NULL);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

