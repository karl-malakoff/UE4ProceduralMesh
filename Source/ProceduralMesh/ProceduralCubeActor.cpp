// UE4 Procedural Mesh Generation from the Epic Wiki (https://wiki.unrealengine.com/Procedural_Mesh_Generation)
//

#include "ProceduralMesh.h"
#include "ProceduralCubeActor.h"

AProceduralCubeActor::AProceduralCubeActor()
{
	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralCube"));

	// Apply a simple material directly using the VertexColor as its BaseColor input
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("Material'/Game/Materials/BaseColor.BaseColor'"));
	// TODO Apply a real material with textures, using UVs
//	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("Material'/Game/Materials/M_Concrete_Poured.M_Concrete_Poured'"));
	mesh->SetMaterial(0, Material.Object);

	// Generate a cube
	FProceduralMeshData data;
	GenerateCube(100.f, data);
	mesh->SetMeshData(data);

	RootComponent = mesh;
}

// Generate a full cube
void AProceduralCubeActor::GenerateCube(const float& InSize, FProceduralMeshData& OutData)
{
	// The 8 vertices of the cube
	OutData.VertexPositions.Add( FVector(	0.f,	0.f,	0.f));	//0
	OutData.VertexPositions.Add( FVector(   0.f,    0.f, InSize));  //1
	OutData.VertexPositions.Add( FVector(InSize,    0.f, InSize));  //2
	OutData.VertexPositions.Add( FVector(InSize,    0.f,    0.f));  //3
	OutData.VertexPositions.Add( FVector(InSize, InSize,    0.f));  //4
	OutData.VertexPositions.Add( FVector(InSize, InSize, InSize));  //5
	OutData.VertexPositions.Add( FVector(   0.f, InSize, InSize));  //6
	OutData.VertexPositions.Add( FVector(	0.f, InSize,	0.f));  //7

	//add color
	OutData.VertexColors.Append(&FColor::Red, 8);

	FProceduralMeshVertexUV uv0(0.f, 0.f);
	FProceduralMeshVertexUV uv1(0.f, 1.f);
	FProceduralMeshVertexUV uv2(1.f, 1.f);
	FProceduralMeshVertexUV uv3(1.f, 0.f);

	FProceduralMeshTriangle t1;
	FProceduralMeshTriangle t2;

	//set UVs for tiangles
	t1.UV0 = uv0;
	t1.UV1 = uv1;
	t1.UV2 = uv2;
	t2.UV0 = uv0;
	t2.UV1 = uv2; 
	t2.UV2 = uv3;

	// front face
	t1.Vertex0 = 0;
	t1.Vertex1 = 1;
	t1.Vertex2 = 2;
	t2.Vertex0 = 0;
	t2.Vertex1 = 2;
	t2.Vertex2 = 3;
	OutData.Triangles.Add(t1);
	OutData.Triangles.Add(t2);

	// back face
	t1.Vertex0 = 4;
	t1.Vertex1 = 5;
	t1.Vertex2 = 6;
	t2.Vertex0 = 4;
	t2.Vertex1 = 6;
	t2.Vertex2 = 7;
	OutData.Triangles.Add(t1);
	OutData.Triangles.Add(t2);

	// left face
	t1.Vertex0 = 7;
	t1.Vertex1 = 6;
	t1.Vertex2 = 1;
	t2.Vertex0 = 7;
	t2.Vertex1 = 1;
	t2.Vertex2 = 0;
	OutData.Triangles.Add(t1);
	OutData.Triangles.Add(t2);

	// right face
	t1.Vertex0 = 3;
	t1.Vertex1 = 2;
	t1.Vertex2 = 5;
	t2.Vertex0 = 3;
	t2.Vertex1 = 5;
	t2.Vertex2 = 4;
	OutData.Triangles.Add(t1);
	OutData.Triangles.Add(t2);

	// top face
	t1.Vertex0 = 1;
	t1.Vertex1 = 6;
	t1.Vertex2 = 5;
	t2.Vertex0 = 1;
	t2.Vertex1 = 5;
	t2.Vertex2 = 2;
	OutData.Triangles.Add(t1);
	OutData.Triangles.Add(t2);

	// bottom face
	t1.Vertex0 = 3;
	t1.Vertex1 = 4;
	t1.Vertex2 = 7;
	t2.Vertex0 = 3;
	t2.Vertex1 = 7;
	t2.Vertex2 = 0;
	OutData.Triangles.Add(t1);
	OutData.Triangles.Add(t2);
}