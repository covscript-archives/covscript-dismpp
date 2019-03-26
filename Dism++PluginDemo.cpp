// Dism++PluginDemo.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

#include <Windows.h>

#include "Dism++API.h"
#include "Plugin.h"
#include <covscript/covscript.hpp>
#ifdef _AMD64_
#pragma comment(lib,"Dism++x64.lib")
#pragma comment(lib,"CovScript_x64.lib")
#else
#pragma comment(lib,"Dism++x86.lib")
#pragma comment(lib,"CovScript_x86.lib")
#endif

// ���ַ�ʽ��һ�����Լ���ת��������һ�־�������ת������
// �������������Ϊ��һЩû�õĲ������ͱ���ת���ˣ������ʾ���Զ�ת������ı�д

// ȫ���Զ�ת������
namespace cs_impl {
	// CovScript->C++ת��
	// ���Զ��C++���Ͷ�Ӧһ��CovScript����
	template<>
	struct type_conversion_cs<const char*> {
		using source_type = cs::string;
	};

	// C++->CovScriptת��
	// ���Զ��C++���Ͷ�Ӧһ��CovScript����
	template<>
	struct type_conversion_cpp<const char*> {
		using target_type = cs::string;
	};

	// �Զ���ת������
	// һ���ܹ�static_cast��ת��CovScript�ܹ��Զ���⵽
	// �����convert������Ҫ��ͳһ����Ҫ��ʱ�������Universal Reference
	template<>
	struct type_convertor<cs::string, const char*> {
		static const char* convert(const cs::string& val) {
			return val.c_str();
		}
	};
}

void message_box(const char* title, const char* text)
{
	MessageBox(nullptr, text, title, MB_ICONINFORMATION);
}

// һ�����������
HRESULT WINAPI HelloWorldCleanup(
	_In_ DismSession Session,
	_Reserved_ DWORD Flags,
	_In_ UINT64 *CleanUpSpace,
	_In_ DismCallBack CallBack,
	_In_ LPVOID UserData)
{
	cs::context_t cxt = cs::create_context({});
	// �������ƿռ�
	cs::namespace_t dismpp_ns = cs::make_shared_namespace<cs::name_space>();
	/*
	1. ��ѭȫ���Զ�ת������+CovScript�Զ����ת������
	2. �Զ���ת������
	3. ͬ��1��������ͨ��Lambda���ʽ�Լ�����ת����һ���Ƽ�������
	*/
	(*dismpp_ns)
	.add_var("message_box_1", cs::make_cni(message_box))
	.add_var("message_box_2", cs::make_cni(message_box, cs::cni_type<void(cs::string, cs::string)>()))
	.add_var("message_box_3", cs::make_cni([](const cs::string& title, const cs::string& text) {
		MessageBox(nullptr, text.c_str(), title.c_str(), MB_ICONINFORMATION);
	}));
	// ����ǰ�����ƿռ����洢��
	cxt->instance->storage.add_buildin_var("dismpp", dismpp_ns);
	cxt->instance->compile("D:\\hello_world_cleanup.csc");
	cxt->instance->interpret();
	cs::function_invoker<void()> cs_main(cs::eval(cxt, "main"));
	cs_main();
	return S_OK;
}