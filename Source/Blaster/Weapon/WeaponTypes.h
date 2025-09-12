#pragma once

/**
 * 射线追踪的最大长度（单位：厘米）
 * 用于武器的射线检测和命中判定
 */
#define TRACE_LENGTH 80000.f

/**
 * 自定义深度渲染的颜色值常量
 * 用于在场景中标识不同状态的武器或物体
 */
#define CUSTOM_DEPTH_PURPLE 250 // 紫色自定义深度值
#define CUSTOM_DEPTH_BLUE 251   // 蓝色自定义深度值
#define CUSTOM_DEPTH_TAN 252    // 棕色自定义深度值

/**
 * 武器类型枚举
 * 定义游戏中所有可用的武器类型
 * 可在蓝图中使用
 */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	//突击步枪
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	//火箭发射器
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	//手枪
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	//冲锋枪
	EWT_SubmachineGun UMETA(DisplayName = "Submachine Gun"),
	//霰弹枪
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	//狙击步枪
	EWT_SniperRifle UMETA(DisplayName = "Sniper Rifle"),
	//榴弹发射器
	EWT_GrenadeLauncher UMETA(DisplayName = "Grenade Launcher"),
	//旗帜
	// 特殊的"武器"类型，用于夺旗模式
	EWT_Flag UMETA(DisplayName = "Flag"),

	// 枚举边界值，用于迭代或验证
	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};