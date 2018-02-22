// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "BiciProtGameMode.h"
#include "BiciProtPawn.h"
#include "BiciProtHud.h"

ABiciProtGameMode::ABiciProtGameMode()
{
	DefaultPawnClass = ABiciProtPawn::StaticClass();
	HUDClass = ABiciProtHud::StaticClass();
}
