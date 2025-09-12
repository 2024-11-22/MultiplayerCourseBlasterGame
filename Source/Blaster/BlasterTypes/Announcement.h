// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * 包含游戏中各种公告和消息的常量字符串
 * 用于UI显示、游戏通知和结果展示等场景
 */
namespace Announcement
{
	/** 新比赛开始倒计时提示文本 */
	const FString NewMatchStartsIn(TEXT("New match starts in:"));
	/** 无获胜者时的提示文本（例如平局） */
	const FString ThereIsNoWinner(TEXT("There is no winner."));
	/** 玩家获胜时的提示文本 */
	const FString YouAreTheWinner(TEXT("You are the winner!"));
	/** 多名玩家并列获胜时的提示文本 */
	const FString PlayersTiedForTheWin(TEXT("Players tied for the win:"));
	/** 多支队伍并列获胜时的提示文本 */
	const FString TeamsTiedForTheWin(TEXT("Teams tied for the win:"));
	/** 红队名称文本 */
	const FString RedTeam(TEXT("Red team"));
	/** 蓝队名称文本 */
	const FString BlueTeam(TEXT("Blue team"));
	/** 红队获胜时的提示文本 */
	const FString RedTeamWins(TEXT("Red team wins!"));
	/** 蓝队获胜时的提示文本 */
	const FString BlueTeamWins(TEXT("Blue team wins!"));
}