// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralSplineMesh.generated.h"

UCLASS()
class PROCEDURALMESH_API  AProceduralSplineMesh : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProceduralSplineMesh(const FObjectInitializer& ObjectInitializer);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

#ifdef WITH_EDITOR
	//AActor interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent) override;
#endif

	/**The spline that is used as the base path of the mesh*/
	UPROPERTY(BlueprintReadOnly, Category = "Procedural Spline Mesh")
		USplineComponent* Spline;

	/**Dynamic procedual mesh */
	UPROPERTY(BlueprintReadOnly, Category = "Procedural Spline Mesh")
		UProceduralMeshComponent* Mesh;

	/**Height that the mesh should be*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Spline Mesh")
		float MeshHeight;

	/**Width to use for the mesh*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Spline Mesh")
		float MeshWidth;

	/**Length of a segment, ie how often the spline should be sampled, higer is smoother but more performance hungry*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Spline Mesh")
		float SegmentLength;

	/**Caculated number of segments*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Procedural Spline Mesh")
		int32 NumberOfSegments;

	//FORTESTING
	/**Change the color*/
	UFUNCTION(BlueprintCallable, Category = "Procedural Spline Mesh")
		void ChangeColor(FLinearColor InColor, float Intensity);

private:

	bool CreateMesh(FProceduralMeshData& OutMesh);
};
