#ifndef FAST_TRAVEL_SUB_PANEL_H
#define FAST_TRAVEL_SUB_PANEL_H

#include <string>
#include <vector>

#include "Panel.h"
#include "ProvinceMapPanel.h"

// This sub-panel is the glue between the province map's travel button and the game world.

class Random;
class Renderer;
class Texture;

class FastTravelSubPanel : public Panel
{
private:
	static const double FRAME_TIME; // Each animation frame's time in seconds.

	ProvinceMapPanel::TravelData travelData; // To give to the game world's arrival pop-up.
	double currentSeconds, totalSeconds, targetSeconds;
	size_t frameIndex;

	// Gets the filename used for the world map image (intended for getting its palette).
	const std::string &getBackgroundFilename() const;

	// Gets the animation for display.
	const std::vector<Texture> &getAnimation() const;

	// Updates the game clock based on the travel data.
	void tickTravelTime(Random &random) const;

	// Called when the target animation time has been reached. Decides whether to go
	// straight to the game world panel or to a staff dungeon splash image panel.
	void switchToNextPanel();
public:
	FastTravelSubPanel(Game &game, const ProvinceMapPanel::TravelData &travelData);
	virtual ~FastTravelSubPanel() = default;

	static const double MIN_SECONDS;

	virtual std::pair<SDL_Texture*, CursorAlignment> getCurrentCursor() const override;
	virtual void tick(double dt) override;
	virtual void render(Renderer &renderer) override;
};

#endif
