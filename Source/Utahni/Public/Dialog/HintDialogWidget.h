// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HintDialogWidget.generated.h"

class UTextBlock;

/**
 *
 */
UCLASS()
class UTAHNI_API UHintDialogWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HintText;

	UFUNCTION(BlueprintCallable)
	void SetHintMessage(const FString& NewMessage);

};