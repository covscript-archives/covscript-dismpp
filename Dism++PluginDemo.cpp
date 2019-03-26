// Dism++PluginDemo.cpp : 定义 DLL 应用程序的导出函数。
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

// 两种方式，一种是自己做转发，另外一种就是声明转发规则
// 不过这个函数因为有一些没用的参数，就必须转发了，这里仅示范自动转发规则的编写

// 全局自动转换规则
namespace cs_impl {
	// CovScript->C++转换
	// 可以多个C++类型对应一个CovScript类型
	template<>
	struct type_conversion_cs<const char*> {
		using source_type = cs::string;
	};

	// C++->CovScript转换
	// 可以多个C++类型对应一个CovScript类型
	template<>
	struct type_conversion_cpp<const char*> {
		using target_type = cs::string;
	};

	// 自定义转换函数
	// 一般能够static_cast的转换CovScript能够自动检测到
	// 这里对convert函数的要求不统一，需要的时候可以上Universal Reference
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

// 一个清理插件入口
HRESULT WINAPI HelloWorldCleanup(
	_In_ DismSession Session,
	_Reserved_ DWORD Flags,
	_In_ UINT64 *CleanUpSpace,
	_In_ DismCallBack CallBack,
	_In_ LPVOID UserData)
{
	cs::context_t cxt = cs::create_context({});
	// 设置名称空间
	cs::namespace_t dismpp_ns = cs::make_shared_namespace<cs::name_space>();
	/*
	1. 遵循全局自动转发规则+CovScript自动检测转发规则
	2. 自定义转发规则
	3. 同（1），但是通过Lambda表达式自己做了转发，一般推荐这样做
	*/
	(*dismpp_ns)
	.add_var("message_box_1", cs::make_cni(message_box))
	.add_var("message_box_2", cs::make_cni(message_box, cs::cni_type<void(cs::string, cs::string)>()))
	.add_var("message_box_3", cs::make_cni([](const cs::string& title, const cs::string& text) {
		MessageBox(nullptr, text.c_str(), title.c_str(), MB_ICONINFORMATION);
	}));
	// 编译前将名称空间加入存储器
	cxt->instance->storage.add_buildin_var("dismpp", dismpp_ns);
	cxt->instance->compile("D:\\hello_world_cleanup.csc");
	cxt->instance->interpret();
	cs::function_invoker<void()> cs_main(cs::eval(cxt, "main"));
	cs_main();
	return S_OK;
}