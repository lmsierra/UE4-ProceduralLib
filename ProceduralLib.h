// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProceduralMeshComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ProceduralLib.generated.h"

UCLASS()
class ELEMENT0_API UProceduralLib : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
    Creates a procedural mesh component copying the data from another procedural mesh component
    param OriginalComponent                  - Component that is going to be copied.
    param OuyDestinationComponent            - Component that copies the original mesh.
    param (Optional) ShouldGenerateCollision - Indicates if collision should be generated.
    param (Optional) NumSection              - First position where the new sections are created. 
    param (Optional) ShouldCopyMaterials     - True if it is copying materials. False otherwise.
    param (Optional) MaterialPosition        - Position where the first material should be copied. 
    return                                   - Return the number of sections created.
    */
    UFUNCTION(BlueprintCallable, Category = "ProceduralMeshLibrary")
        static int32 CopyProceduralMeshFromProceduralMeshComponent(UProceduralMeshComponent* OriginalComponent, UProceduralMeshComponent* OutDestinationComponent, const bool ShouldGenerateCollision = true, const int32 NumSection = 0, const bool ShouldCopyMaterials = true, const int32 MaterialPosition = 0);

    /**
    Merges an array of procedural mesh components into a single procedural mesh component
    param OriginalComponent                  - Component that is going to be copied.
    param OuyDestinationComponent            - Component that copies the original mesh.
    param (Optional) ShouldGenerateCollision - Indicates if collision should be generated.
    param (Optional) NumSection              - First position where the new sections are created.
    param (Optional) ShouldCopyMaterials     - True if it is copying materials. False otherwise.
    param (Optional) MaterialPosition        - Position where the first material should be copied.
    return                                   - Return the number of sections created.
    */
    UFUNCTION(BlueprintCallable, Category = "ProceduralMeshLibrary")
        static int32 MergeProceduralMeshComponentArrayIntoSingleProceduralMeshComponent(TArray<UProceduralMeshComponent*> Origin, UProceduralMeshComponent* OutDestination, const bool ShouldGenerateCollision = true, const int32 NumSection = 0, const bool ShouldCopyMaterials = true, const int32 MaterialPosition = 0);


    /**
    Copy materials from a procedural mesh component into another procedural mesh component
    param Origin                    - Component that is going to be copied.
    param OutDestination            - Component that receives the materials.
    param (Optional) StartPosition  - Position where the first material should be copied.
    return                          - Return the number of materials copied.
    */
    UFUNCTION(BlueprintCallable, Category = "ProceduralMeshLibrary")
        static int32 CopyMaterialsFromProceduralMeshComponentIntoProceduralMeshComponent(const UProceduralMeshComponent* Origin, UProceduralMeshComponent* OutDestination, const int32 StartPosition = 0);
    
    
    /**
     Creates a procedural mesh component from a static mesh component.
     param OriginalComponent       - Component that is going to be copied.
     param OuyDestinationComponent - Component that copies the originalMesh.
     param LODIndex                - Index of the level of detail to copy.
     param ShouldGenerateCollision - Indicates if collision should be generated.
     */
    UFUNCTION(BlueprintCallable, Category = "ProceduralMeshLibrary")
        static void CopyProceduralMeshFromStaticMeshComponent(UStaticMeshComponent* OriginalComponent, UProceduralMeshComponent* OuyDestinationComponent, int32 LODIndex, bool ShouldGenerateCollision);
    
private:
    
    static void GetSectionFromStaticMesh(UStaticMesh* InMesh, int32 LODIndex, int32 SectionIndex, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals, TArray<FVector2D>& UVs, TArray<FProcMeshTangent>& Tangents);

    static int32 GetNewIndexForOldVertIndex(int32 MeshVertIndex, TMap<int32, int32>& MeshToSectionVertMap, const FPositionVertexBuffer* PosBuffer, const FStaticMeshVertexBuffer* VertBuffer, TArray<FVector>& Vertices, TArray<FVector>& Normals, TArray<FVector2D>& UVs, TArray<FProcMeshTangent>& Tangents);

};
