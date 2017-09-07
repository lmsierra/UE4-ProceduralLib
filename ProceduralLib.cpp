// Fill out your copyright notice in the Description page of Project Settings.

#include "Element0.h"
#include "ProceduralLib.h"
#include "PhysicsEngine/BodySetup.h"

int32 UProceduralLib::CopyProceduralMeshFromProceduralMeshComponent(UProceduralMeshComponent* OriginalComponent, UProceduralMeshComponent* OutDestinationComponent, const bool ShouldGenerateCollision, const int32 NumSection, const bool ShouldCopyMaterials, const int32 MaterialPosition)
{
    if (OutDestinationComponent == nullptr || OriginalComponent == nullptr) return -1;
    
    uint32 counter = 0;
    for (int32 i = 0; i < OriginalComponent->GetNumSections(); i++)
    {
        TArray<FVector>          vertices;
        TArray<FVector>          normals;
        TArray<FVector2D>        UVs;
        TArray<FColor>           vertexColors;
        TArray<FProcMeshTangent> tangents;

        FProcMeshSection* section            = OriginalComponent->GetProcMeshSection(i);
        TArray<int32> indexBuffer            = section->ProcIndexBuffer;
        TArray<FProcMeshVertex> vertexBuffer = section->ProcVertexBuffer;

        for (FProcMeshVertex vertex : vertexBuffer)
        {
            vertices.Add(vertex.Position);
            normals.Add(vertex.Normal);
            UVs.Add(vertex.UV0);
            vertexColors.Add(vertex.Color);
            tangents.Add(vertex.Tangent);
        }

        OutDestinationComponent->CreateMeshSection(NumSection + counter, vertices, indexBuffer, normals, UVs, vertexColors, tangents, ShouldGenerateCollision);
        ++counter;
    }

    if (ShouldCopyMaterials)
    {
        CopyMaterialsFromProceduralMeshComponentIntoProceduralMeshComponent(OriginalComponent, OutDestinationComponent, MaterialPosition);
    }

    return counter;
}


int32 UProceduralLib::MergeProceduralMeshComponentArrayIntoSingleProceduralMeshComponent(TArray<UProceduralMeshComponent*> Origin, UProceduralMeshComponent* OutDestination, const bool ShouldGenerateCollision, const int32 NumSection, const bool ShouldCopyMaterials, const int32 MaterialPosition)
{
    int32 meshCounter = 0;
    for (UProceduralMeshComponent* c : Origin)
    {
        meshCounter += CopyProceduralMeshFromProceduralMeshComponent(c, OutDestination, true, NumSection + meshCounter, false);
    }

    if (ShouldCopyMaterials)
    {
        uint32 materialCounter = 0;
        int32 result;
        for (UProceduralMeshComponent* c : Origin)
        {
            result = CopyMaterialsFromProceduralMeshComponentIntoProceduralMeshComponent(c, OutDestination, MaterialPosition + materialCounter);
            if (result >= 0)
            {
                materialCounter += result;
            }
            else
            {
                break;
            }
        }
    }

    return meshCounter;
}


int32 UProceduralLib::CopyMaterialsFromProceduralMeshComponentIntoProceduralMeshComponent(const UProceduralMeshComponent* Origin, UProceduralMeshComponent* OutDestination, const int32 StartPosition)
{
    int32 counter = 0;
    for (int32 i = 0; i < Origin->GetNumMaterials(); i++)
    {
        if (StartPosition + counter >= OutDestination->GetNumMaterials()) 
            break;

        OutDestination->SetMaterial(StartPosition + counter, Origin->GetMaterial(i));
        ++counter;
    }

    return counter;
}


void UProceduralLib::CopyProceduralMeshFromStaticMeshComponent(UStaticMeshComponent* OriginalComponent, UProceduralMeshComponent* OutDestinationComponent, int32 LODIndex, bool ShouldGenerateCollision)
{
    if( OriginalComponent != nullptr &&
       OriginalComponent->GetStaticMesh() != nullptr &&
       OutDestinationComponent != nullptr )
    {
        UStaticMesh* StaticMesh = OriginalComponent->GetStaticMesh();
        
        //// MESH DATA
        
        int32 NumSections = StaticMesh->GetNumSections(LODIndex);
        for (int32 SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
        {
            // Buffers for copying geom data
            TArray<FVector> Vertices;
            TArray<int32> Triangles;
            TArray<FVector> Normals;
            TArray<FVector2D> UVs;
            TArray<FProcMeshTangent> Tangents;
            
            // Get geom data from static mesh
            GetSectionFromStaticMesh(StaticMesh, LODIndex, SectionIndex, Vertices, Triangles, Normals, UVs, Tangents);
            
            // Create section using data
            TArray<FLinearColor> DummyColors;
            OutDestinationComponent->CreateMeshSection_LinearColor(SectionIndex, Vertices, Triangles, Normals, UVs, DummyColors, Tangents, ShouldGenerateCollision);
        }
        
        //// SIMPLE COLLISION
        
        // Clear any existing collision hulls
        OutDestinationComponent->ClearCollisionConvexMeshes();
        
        if (StaticMesh->BodySetup != nullptr)
        {
            // Iterate over all convex hulls on static mesh..
            const int32 NumConvex = StaticMesh->BodySetup->AggGeom.ConvexElems.Num();
            for (int ConvexIndex = 0; ConvexIndex < NumConvex; ConvexIndex++)
            {
                // Copy convex verts to ProcMesh
                FKConvexElem& MeshConvex = StaticMesh->BodySetup->AggGeom.ConvexElems[ConvexIndex];
                OutDestinationComponent->AddCollisionConvexMesh(MeshConvex.VertexData);
            }
        }
        
        //// MATERIALS
        
        for (int32 MatIndex = 0; MatIndex < OriginalComponent->GetNumMaterials(); MatIndex++)
        {
            OutDestinationComponent->SetMaterial(MatIndex, OriginalComponent->GetMaterial(MatIndex));
        }
    }
}



void UProceduralLib::GetSectionFromStaticMesh(UStaticMesh* InMesh, int32 LODIndex, int32 SectionIndex, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals, TArray<FVector2D>& UVs, TArray<FProcMeshTangent>& Tangents)
{
    if(	InMesh != nullptr )
    {
        if (!InMesh->bAllowCPUAccess)
        {
            return;
        }
        else if (InMesh->RenderData != nullptr && InMesh->RenderData->LODResources.IsValidIndex(LODIndex))
        {
            const FStaticMeshLODResources& LOD = InMesh->RenderData->LODResources[LODIndex];
            if (LOD.Sections.IsValidIndex(SectionIndex))
            {
                // Empty output buffers
                Vertices.Reset();
                Triangles.Reset();
                Normals.Reset();
                UVs.Reset();
                Tangents.Reset();
                
                // Map from vert buffer for whole mesh to vert buffer for section of interest
                TMap<int32, int32> MeshToSectionVertMap;
                
                const FStaticMeshSection& Section = LOD.Sections[SectionIndex];
                const uint32 OnePastLastIndex = Section.FirstIndex + Section.NumTriangles * 3;
                FIndexArrayView Indices = LOD.IndexBuffer.GetArrayView();
                
                // Iterate over section index buffer, copying verts as needed
                for (uint32 i = Section.FirstIndex; i < OnePastLastIndex; i++)
                {
                    uint32 MeshVertIndex = Indices[i];
                    
                    // See if we have this vert already in our section vert buffer, and copy vert in if not
                    int32 SectionVertIndex = GetNewIndexForOldVertIndex(MeshVertIndex, MeshToSectionVertMap, &LOD.PositionVertexBuffer, &LOD.VertexBuffer, Vertices, Normals, UVs, Tangents);
                    
                    // Add to index buffer
                    Triangles.Add(SectionVertIndex);
                }
            }
        }
    }
}

int32 UProceduralLib::GetNewIndexForOldVertIndex(int32 MeshVertIndex, TMap<int32, int32>& MeshToSectionVertMap, const FPositionVertexBuffer* PosBuffer, const FStaticMeshVertexBuffer* VertBuffer, TArray<FVector>& Vertices, TArray<FVector>& Normals, TArray<FVector2D>& UVs, TArray<FProcMeshTangent>& Tangents)
{
    int32* NewIndexPtr = MeshToSectionVertMap.Find(MeshVertIndex);
    if (NewIndexPtr != nullptr)
    {
        return *NewIndexPtr;
    }
    else
    {
        // Copy position
        int32 SectionVertIndex = Vertices.Add(PosBuffer->VertexPosition(MeshVertIndex));
        
        // Copy normal
        Normals.Add(VertBuffer->VertexTangentZ(MeshVertIndex));
        check(Normals.Num() == Vertices.Num());
        
        // Copy UVs
        UVs.Add(VertBuffer->GetVertexUV(MeshVertIndex, 0));
        check(UVs.Num() == Vertices.Num());
        
        // Copy tangents
        FVector4 TangentX = VertBuffer->VertexTangentX(MeshVertIndex);
        FProcMeshTangent NewTangent(TangentX, TangentX.W < 0.f);
        Tangents.Add(NewTangent);
        check(Tangents.Num() == Vertices.Num());
        
        MeshToSectionVertMap.Add(MeshVertIndex, SectionVertIndex);
        
        return SectionVertIndex;
    }
}

