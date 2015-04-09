// UE4 Procedural Mesh Generation from the Epic Wiki (https://wiki.unrealengine.com/Procedural_Mesh_Generation)
//
// A lathed object is a 3D model whose vertex geometry is produced by rotating a set of points around a fixed axis.
// (from Wikipedia http://en.wikipedia.org/wiki/Lathe_(graphics))

#include "ProceduralMesh.h"
#include "ProceduralLatheActor.h"

AProceduralLatheActor::AProceduralLatheActor()
{
	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralLathe"));

	// Apply a material
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Game/Materials/BaseColor.BaseColor"));
	mesh->SetMaterial(0, Material.Object);

	// Contains the points describing the polyline we are going to rotate
	TArray<FVector> points;

	points.Add(FVector(190, 50, 0));
	points.Add(FVector(140, 60, 0));
	points.Add(FVector(110, 70, 0));
	points.Add(FVector(100, 80, 0));
	points.Add(FVector(70, 70, 0));
	points.Add(FVector(50, 60, 0));
	points.Add(FVector(30, 50, 0));
	points.Add(FVector(20, 40, 0));
	points.Add(FVector(10, 30, 0));
	points.Add(FVector( 0, 40, 0));

	// Generate a Lathe from rotating the given points
	FProceduralMeshData data;
	GenerateLathe(points, 128, data);
	mesh->SetMeshData(data);

	RootComponent = mesh;
}

// Generate a lathe by rotating the given polyline
void AProceduralLatheActor::GenerateLathe(const TArray<FVector>& InPoints, const int InSegments, FProceduralMeshData& OutData)
{
	UE_LOG(LogClass, Log, TEXT("AProceduralLatheActor::Lathe POINTS %d"), InPoints.Num());

	TArray<FVector> verts;

	// precompute some trig
	float angle = FMath::DegreesToRadians(360.0f / InSegments);
	float sinA = FMath::Sin(angle);
	float cosA = FMath::Cos(angle);

	/*
	This implementation is rotation around the X Axis, other formulas below

	Z Axis Rotation
	x' = x*cos q - y*sin q
	y' = x*sin q + y*cos q
	z' = z

	X Axis Rotation
	y' = y*cos q - z*sin q
	z' = y*sin q + z*cos q
	x' = x

	Y Axis Rotation
	z' = z*cos q - x*sin q
	x' = z*sin q + x*cos q
	y' = y
	*/


	int32 NumOfVertices = InPoints.Num() * InSegments + 2; //puls 2 fpr first and last point
	OutData.VertexColors.Append(&FColor::Blue, NumOfVertices);

	// Working point array, in which we keep the rotated line we draw with
	TArray<FVector> wp;
	for(int i = 0; i < InPoints.Num(); i++)
	{
		wp.Add(InPoints[i]);
	}

	// Add a first and last point on the axis to complete the OutTriangles
	FVector p0(wp[0].X, 0, 0);
	FVector pLast(wp[wp.Num() - 1].X, 0, 0);

	OutData.VertexPositions.Add(p0);

	FProceduralMeshTriangle tri;
	// for each segment draw the OutTriangles clockwise for normals pointing out or counterclockwise for the opposite (this here does CW)
	// for each segment create the vertices and colors
	for(int segment = 0; segment<InSegments; segment++)
	{
		int32 SegmentRow = segment	 * InPoints.Num();
		int32 SegmentNextRow = (segment < InSegments - 1) ?	(segment + 1) * InPoints.Num() : 0;
		for(int i = 0; i<InPoints.Num() - 1; i++)
		{
			//wraps around at end!
			int32 p1 = SegmentRow + i + 1;
			int32 p2 = SegmentRow + i + 2;
			int32 p1r = SegmentNextRow + i + 1;
			int32 p2r = SegmentNextRow + i + 2;
			FVector p1v = wp[i];
			FVector p2v = wp[i + 1];
			FVector p1rv(p1v.X, p1v.Y*cosA - p1v.Z*sinA, p1v.Y*sinA + p1v.Z*cosA);

			//don't add vertex if i = 0
			if(i == 0)
			{
				tri.Vertex0 = p1;
				tri.Vertex1 = 0;
				tri.Vertex2 = p1r;
				OutData.Triangles.Add(tri);
			}

			tri.Vertex0 = p1;
			tri.Vertex1 = p1r;
			tri.Vertex2 = p2;
			OutData.Triangles.Add(tri);

			tri.Vertex0 = p2;
			tri.Vertex1 = p1r;
			tri.Vertex2 = p2r;
			OutData.Triangles.Add(tri);

			OutData.VertexPositions.Add(wp[i]);
			wp[i] = p1rv;

			//this has to go here to mainatin add order for vertex positions
			if(i == InPoints.Num() - 2)
			{
				FVector p2rv(p2v.X, p2v.Y*cosA - p2v.Z*sinA, p2v.Y*sinA + p2v.Z*cosA);
				tri.Vertex0 = p2;
				tri.Vertex1 = p2r;
				tri.Vertex2 = NumOfVertices - 1;
				OutData.Triangles.Add(tri);
				OutData.VertexPositions.Add(wp[i + 1]);
				wp[i + 1] = p2rv;
			}
		}
	}

	OutData.VertexPositions.Add(pLast);
}