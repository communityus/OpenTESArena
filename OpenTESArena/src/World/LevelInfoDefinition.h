#ifndef LEVEL_INFO_DEFINITION_H
#define LEVEL_INFO_DEFINITION_H

#include <vector>

#include "LevelDefinition.h"
#include "LockDefinition.h"
#include "TriggerDefinition.h"
#include "VoxelDefinition.h"
#include "VoxelUtils.h"
#include "../Entities/EntityDefinition.h"

// Modern replacement for .INF files; defines the actual voxels, entities, etc. pointed to by a
// level definition. This is intended to separate the level's IDs from what they're pointing to
// so it's easier to change climates, etc..

class LevelInfoDefinition
{
private:
	// Definitions pointed to by a level definition.
	// @todo: eventually want a strictly engine-independent representation for all of these,
	// since currently voxel and entity definitions rely on runtime texture manager handles.
	// - Consider using TextureDefinition for each voxel texture/animation frame.
	std::vector<VoxelDefinition> voxelDefs;
	std::vector<EntityDefinition> entityDefs;
	std::vector<LockDefinition> lockDefs;
	std::vector<TriggerDefinition> triggerDefs;

	double ceilingScale; // Vertical size of walls; 1.0 by default.
public:
	LevelInfoDefinition();

	void init(double ceilingScale);

	const VoxelDefinition &getVoxelDef(LevelDefinition::VoxelDefID id) const;
	const EntityDefinition &getEntityDef(LevelDefinition::EntityDefID id) const;
	const LockDefinition &getLockDef(LevelDefinition::LockDefID id) const;
	const TriggerDefinition &getTriggerDef(LevelDefinition::TriggerDefID id) const;
	double getCeilingScale() const;

	LevelDefinition::VoxelDefID addVoxelDef(VoxelDefinition &&def);
	LevelDefinition::EntityDefID addEntityDef(EntityDefinition &&def);
	LevelDefinition::LockDefID addLockDef(LockDefinition &&def);
	LevelDefinition::TriggerDefID addTriggerDef(TriggerDefinition &&def);
};

#endif
