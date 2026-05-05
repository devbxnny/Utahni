// Fill out your copyright notice in the Description page of Project Settings.


#include "Dialog/HintDialogWidget.h"
#include "Components/TextBlock.h"

void UHintDialogWidget::SetHintMessage(const FString& NewMessage)
{
	if (HintText)
	{
		HintText->SetText(FText::FromString(NewMessage));
	}
}