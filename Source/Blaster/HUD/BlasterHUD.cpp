// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"
#include "Announcement.h"
#include "ElimAnnouncement.h"
#include "Components/HorizontalBox.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"

/**
 * 游戏开始时的初始化函数
 * 重写父类的BeginPlay函数，执行HUD相关的初始化操作
 */
void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
}

/**
 * 添加击杀公告信息
 * 创建并显示一个击杀公告UI，显示攻击者和受害者信息
 * @param Attacker - 攻击者名称
 * @param Victim - 受害者名称
 */
void ABlasterHUD::AddElimAnnouncement(FString Attacker, FString Victim)
{
	// 获取或更新玩家控制器引用
	OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayerController() : OwningPlayer;
	// 检查玩家控制器和公告类是否有效
	if (OwningPlayer && ElimAnnouncementClass)
	{
		// 创建击杀公告UI实例
		UElimAnnouncement* ElimAnnouncementWidget = CreateWidget<UElimAnnouncement>(OwningPlayer, ElimAnnouncementClass);
		if (ElimAnnouncementWidget)
		{
			// 设置公告文本，显示击杀信息
			ElimAnnouncementWidget->SetElimAnnouncementText(Attacker, Victim);
			// 将公告添加到视口
			ElimAnnouncementWidget->AddToViewport();

			// 调整已有的所有公告位置，使新公告显示在最上方
			for (UElimAnnouncement* Msg : ElimMessages)
			{
				if (Msg && Msg->AnnouncementBox)
				{
					// 获取画布插槽，用于调整位置
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
					if (CanvasSlot)
					{
						// 计算新位置，将现有公告向上移动
						FVector2D Position = CanvasSlot->GetPosition();
						FVector2D NewPosition(
							CanvasSlot->GetPosition().X,
							Position.Y - CanvasSlot->GetSize().Y
						);
						// 设置新位置
						CanvasSlot->SetPosition(NewPosition);
					}
				}
			}

			// 将新公告添加到消息数组
			ElimMessages.Add(ElimAnnouncementWidget);

			// 设置计时器，在指定时间后移除公告
			FTimerHandle ElimMsgTimer;
			FTimerDelegate ElimMsgDelegate;
			// 绑定计时器回调函数
			ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncementWidget);
			GetWorldTimerManager().SetTimer(
				ElimMsgTimer,        // 计时器句柄
				ElimMsgDelegate,     // 回调委托
				ElimAnnouncementTime,// 计时时间
				false                // 不循环
			);
		}
	}
}

/**
 * 击杀公告计时器完成回调函数
 * 当公告显示时间结束时，移除公告UI
 * @param MsgToRemove - 要移除的公告实例
 */
void ABlasterHUD::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove)
{
	if (MsgToRemove)
	{
		// 从父组件中移除公告UI
		MsgToRemove->RemoveFromParent();
	}
}

/**
 * 添加角色状态叠加层
 * 创建并显示角色状态UI，用于显示生命值、弹药等信息
 */
void ABlasterHUD::AddCharacterOverlay()
{
	// 获取玩家控制器
	APlayerController* PlayerController = GetOwningPlayerController();
	// 检查玩家控制器和叠加层类是否有效
	if (PlayerController && CharacterOverlayClass)
	{
		// 创建角色状态叠加层实例
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		// 添加到视口
		CharacterOverlay->AddToViewport();
	}
}

/**
 * 添加公告UI
 * 创建并显示游戏公告UI，用于显示游戏状态信息
 */
void ABlasterHUD::AddAnnouncement()
{
	// 获取玩家控制器
	APlayerController* PlayerController = GetOwningPlayerController();
	// 检查玩家控制器和公告类是否有效
	if (PlayerController && AnnouncementClass)
	{
		// 创建公告UI实例
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		// 添加到视口
		Announcement->AddToViewport();
	}
}

/**
 * 绘制HUD元素
 * 重写父类的DrawHUD函数，绘制游戏中的准星等HUD元素
 */
void ABlasterHUD::DrawHUD()
{
	// 调用父类的DrawHUD函数
	Super::DrawHUD();

	// 获取视口大小
	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		// 计算视口中心位置
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		// 计算准星扩散距离
		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		// 绘制中心准星
		if (HUDPackage.CrosshairsCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		// 绘制左侧准星
		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		// 绘制右侧准星
		if (HUDPackage.CrosshairsRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		// 绘制上方准星
		if (HUDPackage.CrosshairsTop)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		// 绘制下方准星
		if (HUDPackage.CrosshairsBottom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
	}
}

/**
 * 绘制单个准星元素
 * @param Texture - 准星纹理
 * @param ViewportCenter - 视口中心位置
 * @param Spread - 准星扩散偏移量
 * @param CrosshairColor - 准星颜色
 */
void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	// 获取纹理尺寸
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	// 计算纹理绘制位置（考虑纹理中心点和扩散偏移）
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
	);

	// 绘制纹理
	DrawTexture(
		Texture,           // 纹理资源
		TextureDrawPoint.X, // X坐标
		TextureDrawPoint.Y, // Y坐标
		TextureWidth,       // 宽度
		TextureHeight,      // 高度
		0.f,                // U坐标起始点
		0.f,                // V坐标起始点
		1.f,                // U坐标结束点
		1.f,                // V坐标结束点
		CrosshairColor      // 颜色
	);
}