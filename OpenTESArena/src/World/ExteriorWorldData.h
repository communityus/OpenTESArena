#ifndef EXTERIOR_WORLD_DATA_H
#define EXTERIOR_WORLD_DATA_H

#include <cstdint>
#include <vector>

#include "ExteriorLevelData.h"
#include "InteriorWorldData.h"
#include "LevelData.h"
#include "WorldData.h"
#include "../Assets/INFFile.h"
#include "../Math/Vector2.h"

class BinaryAssetLibrary;
class ExeData;
class LocationDefinition;
class MIFFile;
class ProvinceDefinition;
class TextAssetLibrary;

enum class ClimateType;
enum class WeatherType;

class ExteriorWorldData : public WorldData
{
private:
	struct InteriorState
	{
		InteriorWorldData worldData;
		Int2 returnVoxel; // Where the player returns to outside.

		InteriorState(InteriorWorldData &&worldData, const Int2 &returnVoxel);
	};

	ExteriorLevelData levelData;
	std::unique_ptr<InteriorState> interior; // Non-null when the player is in an interior.
	bool isCity; // True if city, false if wilderness.

	ExteriorWorldData(ExteriorLevelData &&levelData, bool isCity);
public:
	ExteriorWorldData(ExteriorWorldData&&) = default;
	virtual ~ExteriorWorldData();

	// Loads an exterior city skeleton and its random .MIF chunks.
	static ExteriorWorldData loadCity(const LocationDefinition &locationDef,
		const ProvinceDefinition &provinceDef, const MIFFile &mif, WeatherType weatherType,
		int currentDay, int starCount, const BinaryAssetLibrary &binaryAssetLibrary,
		const TextAssetLibrary &textAssetLibrary, TextureManager &textureManager);

	// Loads wilderness for a given city on the world map.
	static ExteriorWorldData loadWilderness(const LocationDefinition &locationDef,
		const ProvinceDefinition &provinceDef, WeatherType weatherType, int currentDay,
		int starCount, const BinaryAssetLibrary &binaryAssetLibrary,
		TextureManager &textureManager);

	// Returns the current active interior (if any).
	InteriorWorldData *getInterior() const;

	virtual WorldType getBaseWorldType() const override;
	virtual WorldType getActiveWorldType() const override;

	virtual LevelData &getActiveLevel() override;
	virtual const LevelData &getActiveLevel() const override;

	// Sets the exterior world's active interior, and also saves the player's exterior voxel
	// position. Causes an error if there's already an interior.
	void enterInterior(InteriorWorldData &&interior, const Int2 &returnVoxel);

	// Leaves the current interior, sets the exterior active, and returns the saved voxel coordinate
	// for where the player is put in the exterior. Causes an error if no interior is active.
	Int2 leaveInterior();
};

#endif
