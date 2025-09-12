// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiplayerSessions.h"

#define LOCTEXT_NAMESPACE "FMultiplayerSessionsModule"

/**
 * 当模块被加载到内存时调用的函数
 * 此代码将在模块加载后执行，具体时间在.uplugin文件中为每个模块指定
 */
void FMultiplayerSessionsModule::StartupModule()
{
	// 模块启动逻辑，可以在这里进行初始化操作
}

/**
 * 当模块被卸载时调用的函数
 * 此函数可能在关闭期间调用，用于清理模块资源
 * 对于支持动态重新加载的模块，我们在卸载模块之前调用此函数
 */
void FMultiplayerSessionsModule::ShutdownModule()
{
	// 模块关闭逻辑，可以在这里进行资源清理
}

#undef LOCTEXT_NAMESPACE

// 实现MultiplayerSessions模块
IMPLEMENT_MODULE(FMultiplayerSessionsModule, MultiplayerSessions)