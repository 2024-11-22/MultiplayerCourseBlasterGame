// Fill out your copyright notice in the Description page of Project Settings.


#include "ElimAnnouncement.h"
#include "Components/TextBlock.h"

/**
 * 设置击杀公告文本
 * 生成并显示格式为"攻击者名称 elimmed 受害者名称!"的击杀信息
 * @param AttackerName - 攻击者名称
 * @param VictimName - 受害者名称
 */
void UElimAnnouncement::SetElimAnnouncementText(FString AttackerName, FString VictimName)
{
	// 格式化击杀公告文本，将攻击者和受害者名称插入到消息中
	FString ElimAnnouncementText = FString::Printf(TEXT("%s elimmed %s!"), *AttackerName, *VictimName);
	// 检查公告文本组件是否有效
	if (AnnouncementText)
	{
		// 设置文本内容，将FString转换为FText格式
		AnnouncementText->SetText(FText::FromString(ElimAnnouncementText));
	}
}