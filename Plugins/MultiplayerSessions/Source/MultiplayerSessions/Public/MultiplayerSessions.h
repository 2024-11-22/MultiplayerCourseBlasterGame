// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * MultiplayerSessions模块的主要类，继承自IModuleInterface
 * 负责处理插件的启动和关闭
 */
class FMultiplayerSessionsModule : public IModuleInterface
{
public:

	/** IModuleInterface实现 */
	// 当模块被加载到内存时调用
	virtual void StartupModule() override;
	// 当模块被卸载时调用，用于清理资源
	virtual void ShutdownModule() override;
};
