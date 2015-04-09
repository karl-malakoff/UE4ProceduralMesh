// Fill out your copyright notice in the Description page of Project Settings.

#include "ProceduralMesh.h"
#include "ProceduralSplineMesh.h"

#include "Components/SplineComponent.h"
#include "ProceduralMeshComponent.h"


// Sets default values
AProceduralSplineMesh::AProceduralSplineMesh(const FObjectInitializer& ObjectInitializer)
	: MeshHeight(50.f)
	, MeshWidth(10.f)
	, SegmentLength(10.f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Spline = ObjectInitializer.CreateDefaultSubobject<USplineComponent>(this, TEXT("Spline"));
	RootComponent = Spline;

	Mesh = ObjectInitializer.CreateDefaultSubobject<UProceduralMeshComponent>(this, TEXT("Procedural Spline Mesh"));
	
	FProceduralMeshData Data;
	CreateMesh(Data);
	Mesh->SetMeshData(Data);

	Mesh->AttachTo(Spline);

	NumberOfSegments = FMath::FloorToInt(Spline->GetSplineLength() / SegmentLength);
}

// Called when the game starts or when spawned
void AProceduralSplineMesh::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProceduralSplineMesh::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

#ifdef WITH_EDITOR
void AProceduralSplineMesh::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	//TODO expand this for more stuff, for now don't care
	FProceduralMeshData Data;
	CreateMesh(Data);
	Mesh->SetMeshData(Data);

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

bool AProceduralSplineMesh::CreateMesh(FProceduralMeshData& OutMesh)
{
	//want to generate all points in local space but spline will give us world so lets cache it for quicker math
	FVector WorldLocaction = Spline->GetComponentLocation();

	NumberOfSegments = FMath::FloorToInt(Spline->GetSplineLength() / SegmentLength);

	//base vectors
	FVector v0(0.f, -(MeshWidth / 2.f), MeshHeight);
	FVector v1(0.f,   MeshWidth / 2.f,  MeshHeight);
	FVector v2(0.f, -(MeshWidth / 2.f),        0.f);
	FVector v3(0.f,   MeshWidth / 2.f,         0.f);

	float PositionOnSpline = 0.f;

	FProceduralMeshTriangle Tri;

	for (int32 Segment = 0; Segment < NumberOfSegments; Segment++)
	{
		//Do front(?) face of box
		if (Segment == 0)
		{
			Tri.Vertex0 = 0;
			Tri.Vertex1 = 2;
			Tri.Vertex2 = 1;
			OutMesh.Triangles.Add(Tri);

			Tri.Vertex0 = 1;
			Tri.Vertex1 = 2;
			Tri.Vertex2 = 3;
			OutMesh.Triangles.Add(Tri);
		}

		//Add vertices
		FVector SplineOffset = Spline->GetWorldLocationAtDistanceAlongSpline(PositionOnSpline);
		SplineOffset -= WorldLocaction;

		//also need to rotate by spline tangent
		FVector SplineTangent = Spline->GetWorldTangentAtDistanceAlongSpline(PositionOnSpline);
		

		OutMesh.VertexPositions.Add(SplineTangent.Rotation().RotateVector(v0) + SplineOffset);
		OutMesh.VertexPositions.Add(SplineTangent.Rotation().RotateVector(v1) + SplineOffset);
		OutMesh.VertexPositions.Add(SplineTangent.Rotation().RotateVector(v2) + SplineOffset);
		OutMesh.VertexPositions.Add(SplineTangent.Rotation().RotateVector(v3) + SplineOffset);

		//add other faces
		int Row = Segment * 4;
		int NextRow = (Segment + 1) * 4;

		//right
		Tri.Vertex0 = Row + 1;
		Tri.Vertex1 = Row + 3;
		Tri.Vertex2 = NextRow + 3;
		OutMesh.Triangles.Add(Tri);
		Tri.Vertex0 = Row + 1;
		Tri.Vertex1 = NextRow + 3;
		Tri.Vertex2 = NextRow + 1;
		OutMesh.Triangles.Add(Tri);

		//Top
		Tri.Vertex0 = Row + 0;
		Tri.Vertex1 = NextRow + 1;
		Tri.Vertex2 = NextRow + 0;
		OutMesh.Triangles.Add(Tri);
		Tri.Vertex1 = Row + 1;
		Tri.Vertex0 = Row + 0;
		Tri.Vertex2 = NextRow + 1;
		OutMesh.Triangles.Add(Tri);

		//left
		Tri.Vertex0 = NextRow + 0;
		Tri.Vertex1 = Row + 2;
		Tri.Vertex2 = Row + 0;
		OutMesh.Triangles.Add(Tri);
		Tri.Vertex0 = NextRow + 0;
		Tri.Vertex1 = NextRow + 2;
		Tri.Vertex2 = Row + 2;
		OutMesh.Triangles.Add(Tri);

		PositionOnSpline += SegmentLength;

		if (Segment == NumberOfSegments - 1)
		{
			//Add a final set of vertices
			SplineOffset = Spline->GetWorldLocationAtDistanceAlongSpline(PositionOnSpline);
			SplineOffset -= WorldLocaction;
			FVector SplineTangent = Spline->GetWorldTangentAtDistanceAlongSpline(PositionOnSpline);

			OutMesh.VertexPositions.Add(SplineTangent.Rotation().RotateVector(v0) + SplineOffset);
			OutMesh.VertexPositions.Add(SplineTangent.Rotation().RotateVector(v1) + SplineOffset);
			OutMesh.VertexPositions.Add(SplineTangent.Rotation().RotateVector(v2) + SplineOffset);
			OutMesh.VertexPositions.Add(SplineTangent.Rotation().RotateVector(v3) + SplineOffset);
		
			//add the back face
			Tri.Vertex0 = NextRow + 1;
			Tri.Vertex1 = NextRow + 3;
			Tri.Vertex2 = NextRow + 0;
			OutMesh.Triangles.Add(Tri);
			Tri.Vertex0 = NextRow + 0;
			Tri.Vertex1 = NextRow + 3;
			Tri.Vertex2 = NextRow + 2;
			OutMesh.Triangles.Add(Tri);
		}
	}

	//Add colors
	int NumberOfVertices = OutMesh.VertexPositions.Num();

	//TODO for testing only!
	uint8 Red = 254;
	uint8 Green = 1;
	uint8 Alpha = 1;
	int8 Delta = 4;
	int8 DeltaAlpha = 1;
	for (int32 c = 0; c < NumberOfVertices; c += 4)
	{
		uint16 Check = Green + Delta;
		if (Check > 255)
		{
			Delta = -4;
		}
		else
		{
			Check = Red + Delta;
			if (Check > 255)
			{
				Delta = 4;
			}
		}

		if (Alpha + DeltaAlpha > 25)
		{
			DeltaAlpha = -1;

		}
		else
		{
			int32 DeltaCheck = int32(Alpha) + int32(DeltaAlpha);
			if (DeltaCheck < 1)
			{
				DeltaAlpha = 1;
			}
		}

		Red -= Delta;
		Green += Delta;	//should wrap
		Alpha += DeltaAlpha;
		//Alpha = (Alpha > 250) ? 150 : Alpha;
		FColor Color(Red, 0, Green, Alpha);
		OutMesh.VertexColors.Add(Color);
		OutMesh.VertexColors.Add(Color);
		OutMesh.VertexColors.Add(Color);
		OutMesh.VertexColors.Add(Color);
	}

	return true;
}

void AProceduralSplineMesh::ChangeColor(FLinearColor InColor, float Intensity)
{
	static int32 NumberOfVertices = 4 * NumberOfSegments + 4;

	static int32 CurrentSet = 0;

	//this works pretty well
	//FColor AsColor(FMath::Lerp(uint8(0), uint8(255), InColor.R / Intensity),
	//	FMath::Lerp(uint8(0), uint8(255), InColor.G / Intensity),
	//	FMath::Lerp(uint8(0), uint8(255), InColor.B / Intensity),
	//	uint8(FMath::FloorToInt(Intensity)));

	FColor AsColor(InColor);
	AsColor.A = Intensity;

	FProceduralMeshData& MeshData = Mesh->GetMeshData();
	/*MeshData.VertexColors.Reset();


	for (int32 i = 0; i < NumberOfVertices; i++)
	{
		MeshData.VertexColors.Add(AsColor);
	}*/

	MeshData.VertexColors[CurrentSet] = AsColor;
	MeshData.VertexColors[CurrentSet + 1] = AsColor;
	MeshData.VertexColors[CurrentSet + 2] = AsColor;
	MeshData.VertexColors[CurrentSet + 3] = AsColor;

	Mesh->MarkRenderStateDirty();

	CurrentSet += 4;
	CurrentSet %= NumberOfVertices;
}
