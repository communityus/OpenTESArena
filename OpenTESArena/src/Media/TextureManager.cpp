#include <algorithm>

#include "SDL.h"

#include "PaletteFile.h"
#include "PaletteName.h"
#include "PaletteUtils.h"
#include "TextureManager.h"
#include "../Assets/CFAFile.h"
#include "../Assets/CIFFile.h"
#include "../Assets/COLFile.h"
#include "../Assets/Compression.h"
#include "../Assets/DFAFile.h"
#include "../Assets/FLCFile.h"
#include "../Assets/IMGFile.h"
#include "../Assets/RCIFile.h"
#include "../Assets/SETFile.h"
#include "../Math/Vector2.h"
#include "../Rendering/Renderer.h"

#include "components/debug/Debug.h"
#include "components/utilities/String.h"
#include "components/utilities/StringView.h"
#include "components/vfs/manager.hpp"

TextureManager::~TextureManager()
{
	
}

void TextureManager::loadCOLPalette(const std::string &colName)
{
	COLFile colFile;
	if (!colFile.init(colName.c_str()))
	{
		DebugCrash("Could not init .COL file \"" + colName + "\".");
	}

	this->palettes.emplace(std::make_pair(colName, colFile.getPalette()));
}

void TextureManager::loadIMGPalette(const std::string &imgName)
{
	Palette palette;
	if (!IMGFile::extractPalette(imgName.c_str(), palette))
	{
		DebugCrash("Could not extract palette from \"" + imgName + "\".");
	}

	this->palettes.emplace(std::make_pair(imgName, palette));
}

void TextureManager::loadPalette(const std::string &paletteName)
{
	// Don't load the same palette more than once.
	DebugAssert(this->palettes.find(paletteName) == this->palettes.end());

	// Get file extension of the palette name.
	const std::string_view extension = StringView::getExtension(paletteName);
	const bool isCOL = extension == "COL";
	const bool isIMG = extension == "IMG";
	const bool isMNU = extension == "MNU";

	if (isCOL)
	{
		this->loadCOLPalette(paletteName);
	}
	else if (isIMG || isMNU)
	{
		this->loadIMGPalette(paletteName);
	}
	else
	{
		DebugCrash("Unrecognized palette \"" + paletteName + "\".");
	}

	// Make sure everything above works as intended.
	DebugAssert(this->palettes.find(paletteName) != this->palettes.end());
}

Surface TextureManager::make32BitFromPaletted(int width, int height,
	const uint8_t *srcPixels, const Palette &palette)
{
	Surface surface = Surface::createWithFormat(width, height,
		Renderer::DEFAULT_BPP, Renderer::DEFAULT_PIXELFORMAT);
	uint32_t *dstPixels = static_cast<uint32_t*>(surface.get()->pixels);

	// Generate a 32-bit color from each palette index in the source image and
	// write them to the destination image.
	const int pixelCount = width * height;
	std::transform(srcPixels, srcPixels + pixelCount, dstPixels,
		[&palette](uint8_t pixel)
	{
		return palette[pixel].toARGB();
	});

	return surface;
}

Buffer2D<uint8_t> TextureManager::make8BitSurface(const std::string_view &filename,
	Palette *outPalette)
{
	// Check what kind of file extension the filename has.
	const std::string_view extension = StringView::getExtension(filename);
	const bool isIMG = extension == "IMG";
	const bool isMNU = extension == "MNU";

	Buffer2D<uint8_t> buffer;

	if (isIMG || isMNU)
	{
		IMGFile img;
		if (!img.init(filename.data()))
		{
			DebugCrash("Could not init .IMG file \"" + std::string(filename) + "\".");
		}

		buffer.init(img.getWidth(), img.getHeight());

		const uint8_t *srcPixels = img.getPixels();
		const int pixelCount = img.getWidth() * img.getHeight();
		std::copy(srcPixels, srcPixels + pixelCount, buffer.get());

		// Optionally write out the contained palette (if any).
		const Palette *imgPalette = img.getPalette();
		if (imgPalette != nullptr && outPalette != nullptr)
		{
			std::copy(imgPalette->begin(), imgPalette->end(), outPalette->begin());
		}
	}
	else
	{
		DebugCrash("Unrecognized surface format \"" + std::string(filename) + "\".");
	}

	return buffer;
}

Buffer2D<uint8_t> TextureManager::make8BitSurface(const std::string_view &filename)
{
	return TextureManager::make8BitSurface(filename, nullptr);
}

Buffer<Buffer2D<uint8_t>> TextureManager::make8BitSurfaces(const std::string_view &filename,
	Buffer<Palette> *outPalettes)
{
	const std::string_view extension = StringView::getExtension(filename);
	const bool isCFA = extension == "CFA";
	const bool isCIF = extension == "CIF";
	const bool isCEL = extension == "CEL";
	const bool isDFA = extension == "DFA";
	const bool isFLC = extension == "FLC";
	const bool isRCI = extension == "RCI";
	const bool isSET = extension == "SET";

	Buffer<Buffer2D<uint8_t>> buffers;

	if (isCFA)
	{
		CFAFile cfa;
		if (!cfa.init(filename.data()))
		{
			DebugCrash("Could not init .CFA file \"" + std::string(filename) + "\".");
		}

		buffers.init(cfa.getImageCount());

		// Create a surface for each image in the .CFA.
		for (int i = 0; i < cfa.getImageCount(); i++)
		{
			Buffer2D<uint8_t> &buffer = buffers.get(i);
			buffer.init(cfa.getWidth(), cfa.getHeight());

			const uint8_t *srcPixels = cfa.getPixels(i);
			const int pixelCount = cfa.getWidth() * cfa.getHeight();
			std::copy(srcPixels, srcPixels + pixelCount, buffer.get());
		}
	}
	else if (isCIF)
	{
		CIFFile cif;
		if (!cif.init(filename.data()))
		{
			DebugCrash("Could not init .CIF file \"" + std::string(filename) + "\".");
		}

		buffers.init(cif.getImageCount());

		// Create a surface for each image in the .CIF.
		for (int i = 0; i < cif.getImageCount(); i++)
		{
			const int cifWidth = cif.getWidth(i);
			const int cifHeight = cif.getHeight(i);

			Buffer2D<uint8_t> &buffer = buffers.get(i);
			buffer.init(cifWidth, cifHeight);

			const uint8_t *srcPixels = cif.getPixels(i);
			const int pixelCount = cifWidth * cifHeight;
			std::copy(srcPixels, srcPixels + pixelCount, buffer.get());
		}
	}
	else if (isDFA)
	{
		DFAFile dfa;
		if (!dfa.init(filename.data()))
		{
			DebugCrash("Could not init .DFA file \"" + std::string(filename) + "\".");
		}

		buffers.init(dfa.getImageCount());

		// Create a surface for each image in the .DFA.
		for (int i = 0; i < dfa.getImageCount(); i++)
		{
			Buffer2D<uint8_t> &buffer = buffers.get(i);
			buffer.init(dfa.getWidth(), dfa.getHeight());

			const uint8_t *srcPixels = dfa.getPixels(i);
			const int pixelCount = dfa.getWidth() * dfa.getHeight();
			std::copy(srcPixels, srcPixels + pixelCount, buffer.get());
		}
	}
	else if (isFLC || isCEL)
	{
		FLCFile flc;
		if (!flc.init(filename.data()))
		{
			DebugCrash("Could not init .FLC file \"" + std::string(filename) + "\".");
		}

		buffers.init(flc.getFrameCount());

		// Optionally prepare the output palette buffer.
		if (outPalettes != nullptr)
		{
			outPalettes->init(flc.getFrameCount());
		}

		// Create a surface for each frame in the .FLC.
		for (int i = 0; i < flc.getFrameCount(); i++)
		{
			Buffer2D<uint8_t> &buffer = buffers.get(i);
			buffer.init(flc.getWidth(), flc.getHeight());

			const uint8_t *srcPixels = flc.getPixels(i);
			const int pixelCount = flc.getWidth() * flc.getHeight();
			std::copy(srcPixels, srcPixels + pixelCount, buffer.get());

			// Optionally write out the frame's palette.
			if (outPalettes != nullptr)
			{
				const Palette &framePalette = flc.getFramePalette(i);
				outPalettes->set(i, framePalette);
			}
		}
	}
	else if (isRCI)
	{
		RCIFile rci;
		if (!rci.init(filename.data()))
		{
			DebugCrash("Could not init .RCI file \"" + std::string(filename) + "\".");
		}

		buffers.init(rci.getImageCount());

		// Create a surface for each image in the .RCI.
		for (int i = 0; i < rci.getImageCount(); i++)
		{
			Buffer2D<uint8_t> &buffer = buffers.get(i);
			buffer.init(RCIFile::WIDTH, RCIFile::HEIGHT);

			const uint8_t *srcPixels = rci.getPixels(i);
			const int pixelCount = RCIFile::WIDTH * RCIFile::HEIGHT;
			std::copy(srcPixels, srcPixels + pixelCount, buffer.get());
		}
	}
	else if (isSET)
	{
		SETFile set;
		if (!set.init(filename.data()))
		{
			DebugCrash("Could not init .SET file \"" + std::string(filename) + "\".");
		}

		buffers.init(set.getImageCount());

		// Create a surface for each image in the .SET.
		for (int i = 0; i < set.getImageCount(); i++)
		{
			Buffer2D<uint8_t> &buffer = buffers.get(i);
			buffer.init(SETFile::CHUNK_WIDTH, SETFile::CHUNK_HEIGHT);

			const uint8_t *srcPixels = set.getPixels(i);
			const int pixelCount = SETFile::CHUNK_WIDTH * SETFile::CHUNK_HEIGHT;
			std::copy(srcPixels, srcPixels + pixelCount, buffer.get());
		}
	}
	else
	{
		DebugCrash("Unrecognized surface list \"" + std::string(filename) + "\".");
	}

	return buffers;
}

Buffer<Buffer2D<uint8_t>> TextureManager::make8BitSurfaces(const std::string_view &filename)
{
	return TextureManager::make8BitSurfaces(filename, nullptr);
}

const Surface &TextureManager::getSurface(const std::string &filename,
	const std::string &paletteName)
{
	// Use this name when interfacing with the surfaces map.
	const std::string fullName = filename + paletteName;

	// See if the image file has already been loaded with the palette.
	auto surfaceIter = this->surfaces.find(fullName);
	if (surfaceIter != this->surfaces.end())
	{
		// The requested surface exists.
		return surfaceIter->second;
	}

	// Attempt to use the image's built-in palette if requested.
	const bool useBuiltInPalette = PaletteUtils::isBuiltIn(paletteName);

	// See if the palette hasn't already been loaded.
	const bool paletteIsLoaded = this->palettes.find(paletteName) != this->palettes.end();
	const bool imagePaletteIsLoaded = this->palettes.find(filename) != this->palettes.end();
	if ((!useBuiltInPalette && !paletteIsLoaded) ||
		(useBuiltInPalette && !imagePaletteIsLoaded))
	{
		// Use the image's filename if using the built-in palette. Otherwise, use the given
		// palette name.
		this->loadPalette(useBuiltInPalette ? filename : paletteName);
	}

	// The image hasn't been loaded with the palette yet, so make a new entry.
	// Check what kind of file extension the filename has.
	const std::string_view extension = StringView::getExtension(filename);
	const bool isCOL = extension == "COL";
	const bool isIMG = extension == "IMG";
	const bool isMNU = extension == "MNU";

	Surface surface;

	if (isCOL)
	{
		// A palette was requested as the primary image. Convert it to a surface.
		COLFile colFile;
		if (!colFile.init(filename.c_str()))
		{
			DebugCrash("Could not init .COL file \"" + filename + "\".");
		}

		const Palette &colPalette = colFile.getPalette();

		DebugAssert(colPalette.size() == PALETTE_SIZE);
		surface = Surface::createWithFormat(16, 16, Renderer::DEFAULT_BPP,
			Renderer::DEFAULT_PIXELFORMAT);
		
		uint32_t *pixels = static_cast<uint32_t*>(surface.get()->pixels);
		for (size_t i = 0; i < colPalette.size(); i++)
		{
			pixels[i] = colPalette[i].toARGB();
		}
	}
	else if (isIMG || isMNU)
	{
		IMGFile img;
		if (!img.init(filename.c_str()))
		{
			DebugCrash("Could not init .IMG file \"" + filename + "\".");
		}

		// Decide if the .IMG will use its own palette or not.
		const Palette &palette = useBuiltInPalette ?
			*img.getPalette() : this->palettes.at(paletteName);
		
		// Create a surface from the .IMG.
		surface = Surface::createWithFormat(img.getWidth(), img.getHeight(),
			Renderer::DEFAULT_BPP, Renderer::DEFAULT_PIXELFORMAT);

		// Generate 32-bit colors from each palette index in the .IMG pixels and
		// write them to the destination image.
		const uint8_t *srcPixels = img.getPixels();
		uint32_t *dstPixels = static_cast<uint32_t*>(surface.get()->pixels);
		std::transform(srcPixels, srcPixels + (img.getWidth() * img.getHeight()), dstPixels,
			[&palette](uint8_t srcPixel)
		{
			return palette[srcPixel].toARGB();
		});
	}
	else
	{
		DebugCrash("Unrecognized surface format \"" + filename + "\".");
	}

	// Add the new surface and return it.
	auto iter = this->surfaces.emplace(std::make_pair(fullName, std::move(surface))).first;
	return iter->second;
}

const Surface &TextureManager::getSurface(const std::string &filename)
{
	return this->getSurface(filename, this->activePalette);
}

const Texture &TextureManager::getTexture(const std::string &filename,
	const std::string &paletteName, Renderer &renderer)
{
	// Use this name when interfacing with the textures map.
	const std::string fullName = filename + paletteName;

	// See if the image file has already been loaded with the palette.
	auto textureIter = this->textures.find(fullName);
	if (textureIter != this->textures.end())
	{
		// The requested texture exists.
		return textureIter->second;
	}
	// Attempt to use the image's built-in palette if requested.
	const bool useBuiltInPalette = PaletteUtils::isBuiltIn(paletteName);

	// See if the palette hasn't already been loaded.
	const bool paletteIsLoaded = this->palettes.find(paletteName) != this->palettes.end();
	const bool imagePaletteIsLoaded = this->palettes.find(filename) != this->palettes.end();
	if ((!useBuiltInPalette && !paletteIsLoaded) || 
		(useBuiltInPalette && !imagePaletteIsLoaded))
	{
		// Use the filename (i.e., TAMRIEL.IMG) if using the built-in palette.
		// Otherwise, use the given palette name (i.e., PAL.COL).
		this->loadPalette(useBuiltInPalette ? filename : paletteName);
	}

	// The image hasn't been loaded with the palette yet, so make a new entry.
	// Check what kind of file extension the filename has.
	const std::string_view extension = StringView::getExtension(filename);
	const bool isIMG = extension == "IMG";
	const bool isMNU = extension == "MNU";

	Texture texture;

	if (isIMG || isMNU)
	{
		Surface surface = [this, &filename, &paletteName, useBuiltInPalette]()
		{
			IMGFile img;
			if (!img.init(filename.c_str()))
			{
				DebugCrash("Could not init .IMG file \"" + filename + "\".");
			}

			// Decide if the .IMG will use its own palette or not.
			const Palette &palette = useBuiltInPalette ?
				*img.getPalette() : this->palettes.at(paletteName);

			// Create a surface from the .IMG.
			Surface surface = Surface::createWithFormat(img.getWidth(), img.getHeight(),
				Renderer::DEFAULT_BPP, Renderer::DEFAULT_PIXELFORMAT);

			// Generate 32-bit colors from each palette index in the .IMG pixels and
			// write them to the destination image.
			const uint8_t *srcPixels = img.getPixels();
			uint32_t *dstPixels = static_cast<uint32_t*>(surface.get()->pixels);
			std::transform(srcPixels, srcPixels + (img.getWidth() * img.getHeight()), dstPixels,
				[&palette](uint8_t srcPixel)
			{
				return palette[srcPixel].toARGB();
			});

			return surface;
		}();

		// Create a texture from the surface.
		texture = renderer.createTextureFromSurface(surface);

		// Set alpha transparency on.
		SDL_SetTextureBlendMode(texture.get(), SDL_BLENDMODE_BLEND);
	}
	else
	{
		DebugCrash("Unrecognized texture format \"" + filename + "\".");
	}

	// Add the new texture and return it.
	auto iter = this->textures.emplace(std::make_pair(fullName, std::move(texture))).first;
	DebugAssert(texture.get() == nullptr);
	return iter->second;
}

const Texture &TextureManager::getTexture(const std::string &filename, Renderer &renderer)
{
	return this->getTexture(filename, this->activePalette, renderer);
}

const std::vector<Surface> &TextureManager::getSurfaces(
	const std::string &filename, const std::string &paletteName)
{
	// This method deals with animations and movies, so it will check filenames 
	// for ".CFA", ".CIF", ".DFA", ".FLC", ".SET", etc..

	// Use this name when interfacing with the surface sets map.
	const std::string fullName = filename + paletteName;

	// See if the file has already been loaded with the palette.
	auto setIter = this->surfaceSets.find(fullName);
	if (setIter != this->surfaceSets.end())
	{
		// The requested texture set exists.
		return setIter->second;
	}

	// Do not use a built-in palette for surface sets.
	DebugAssertMsg(!PaletteUtils::isBuiltIn(paletteName),
		"Image sets (i.e., .SET files) do not have built-in palettes.");

	// See if the palette hasn't already been loaded.
	if (this->palettes.find(paletteName) == this->palettes.end())
	{
		this->loadPalette(paletteName);
	}

	// The file hasn't been loaded with the palette yet, so make a new entry.
	const auto iter = this->surfaceSets.emplace(
		std::make_pair(fullName, std::vector<Surface>())).first;

	std::vector<Surface> &surfaceSet = iter->second;
	const Palette &palette = this->palettes.at(paletteName);

	const std::string_view extension = StringView::getExtension(filename);
	const bool isCFA = extension == "CFA";
	const bool isCIF = extension == "CIF";
	const bool isCEL = extension == "CEL";
	const bool isDFA = extension == "DFA";
	const bool isFLC = extension == "FLC";
	const bool isRCI = extension == "RCI";
	const bool isSET = extension == "SET";

	if (isCFA)
	{
		CFAFile cfaFile;
		if (!cfaFile.init(filename.c_str()))
		{
			DebugCrash("Could not init .CFA file \"" + filename + "\".");
		}

		// Create a surface for each image in the .CFA.
		for (int i = 0; i < cfaFile.getImageCount(); i++)
		{
			Surface surface = TextureManager::make32BitFromPaletted(
				cfaFile.getWidth(), cfaFile.getHeight(), cfaFile.getPixels(i), palette);
			surfaceSet.push_back(std::move(surface));
		}
	}
	else if (isCIF)
	{
		CIFFile cifFile;
		if (!cifFile.init(filename.c_str()))
		{
			DebugCrash("Could not init .CIF file \"" + filename + "\".");
		}

		// Create a surface for each image in the .CIF.
		for (int i = 0; i < cifFile.getImageCount(); i++)
		{
			Surface surface = TextureManager::make32BitFromPaletted(
				cifFile.getWidth(i), cifFile.getHeight(i), cifFile.getPixels(i), palette);
			surfaceSet.push_back(std::move(surface));
		}
	}
	else if (isDFA)
	{
		DFAFile dfaFile;
		if (!dfaFile.init(filename.c_str()))
		{
			DebugCrash("Could not init .DFA file \"" + filename + "\".");
		}

		// Create a surface for each image in the .DFA.
		for (int i = 0; i < dfaFile.getImageCount(); i++)
		{
			Surface surface = TextureManager::make32BitFromPaletted(
				dfaFile.getWidth(), dfaFile.getHeight(), dfaFile.getPixels(i), palette);
			surfaceSet.push_back(std::move(surface));
		}
	}
	else if (isFLC || isCEL)
	{
		FLCFile flcFile;
		if (!flcFile.init(filename.c_str()))
		{
			DebugCrash("Could not init .FLC/.CEL file \"" + filename + "\".");
		}

		// Create a surface for each frame in the .FLC.
		for (int i = 0; i < flcFile.getFrameCount(); i++)
		{
			Surface surface = TextureManager::make32BitFromPaletted(
				flcFile.getWidth(), flcFile.getHeight(), flcFile.getPixels(i),
				flcFile.getFramePalette(i));
			surfaceSet.push_back(std::move(surface));
		}
	}
	else if (isRCI)
	{
		RCIFile rciFile;
		if (!rciFile.init(filename.c_str()))
		{
			DebugCrash("Could not init .RCI file \"" + filename + "\".");
		}

		// Create a surface for each image in the .RCI.
		for (int i = 0; i < rciFile.getImageCount(); i++)
		{
			Surface surface = TextureManager::make32BitFromPaletted(
				RCIFile::WIDTH, RCIFile::HEIGHT, rciFile.getPixels(i), palette);
			surfaceSet.push_back(std::move(surface));
		}
	}
	else if (isSET)
	{
		SETFile setFile;
		if (!setFile.init(filename.c_str()))
		{
			DebugCrash("Could not init .SET file \"" + filename + "\".");
		}

		// Create a surface for each image in the .SET.
		for (int i = 0; i < setFile.getImageCount(); i++)
		{
			Surface surface = TextureManager::make32BitFromPaletted(
				SETFile::CHUNK_WIDTH, SETFile::CHUNK_HEIGHT, setFile.getPixels(i), palette);
			surfaceSet.push_back(std::move(surface));
		}
	}
	else
	{
		DebugCrash("Unrecognized surface list \"" + filename + "\".");
	}

	return surfaceSet;
}

const std::vector<Surface> &TextureManager::getSurfaces(const std::string &filename)
{
	return this->getSurfaces(filename, this->activePalette);
}

const std::vector<Texture> &TextureManager::getTextures(
	const std::string &filename, const std::string &paletteName, Renderer &renderer)
{
	// This method deals with animations and movies, so it will check filenames 
	// for ".CFA", ".CIF", ".DFA", ".FLC", ".SET", etc..

	// Use this name when interfacing with the texture sets map.
	const std::string fullName = filename + paletteName;

	// See if the file has already been loaded with the palette.
	auto setIter = this->textureSets.find(fullName);
	if (setIter != this->textureSets.end())
	{
		// The requested texture set exists.
		return setIter->second;
	}

	// Do not use a built-in palette for texture sets.
	DebugAssertMsg(!PaletteUtils::isBuiltIn(paletteName),
		"Image sets (i.e., .SET files) do not have built-in palettes.");

	// See if the palette hasn't already been loaded.
	if (this->palettes.find(paletteName) == this->palettes.end())
	{
		this->loadPalette(paletteName);
	}

	// The file hasn't been loaded with the palette yet, so make a new entry.
	const auto iter = this->textureSets.emplace(
		std::make_pair(fullName, std::vector<Texture>())).first;

	std::vector<Texture> &textureSet = iter->second;
	const Palette &palette = this->palettes.at(paletteName);

	const std::string_view extension = StringView::getExtension(filename);
	const bool isCFA = extension == "CFA";
	const bool isCIF = extension == "CIF";
	const bool isCEL = extension == "CEL";
	const bool isDFA = extension == "DFA";
	const bool isFLC = extension == "FLC";
	const bool isRCI = extension == "RCI";
	const bool isSET = extension == "SET";

	if (isCFA)
	{
		CFAFile cfaFile;
		if (!cfaFile.init(filename.c_str()))
		{
			DebugCrash("Could not init .CFA file \"" + filename + "\".");
		}

		// Create a texture for each image in the .CFA.
		for (int i = 0; i < cfaFile.getImageCount(); i++)
		{
			Surface surface = TextureManager::make32BitFromPaletted(
				cfaFile.getWidth(), cfaFile.getHeight(), cfaFile.getPixels(i), palette);
			Texture texture = renderer.createTextureFromSurface(surface);
			textureSet.push_back(std::move(texture));
		}
	}
	else if (isCIF)
	{
		CIFFile cifFile;
		if (!cifFile.init(filename.c_str()))
		{
			DebugCrash("Could not init .CIF file \"" + filename + "\".");
		}

		// Create a texture for each image in the .CIF.
		for (int i = 0; i < cifFile.getImageCount(); i++)
		{
			Surface surface = TextureManager::make32BitFromPaletted(
				cifFile.getWidth(i), cifFile.getHeight(i), cifFile.getPixels(i), palette);
			Texture texture = renderer.createTextureFromSurface(surface);
			textureSet.push_back(std::move(texture));
		}
	}
	else if (isDFA)
	{
		DFAFile dfaFile;
		if (!dfaFile.init(filename.c_str()))
		{
			DebugCrash("Could not init .DFA file \"" + filename + "\".");
		}

		// Create a texture for each image in the .DFA.
		for (int i = 0; i < dfaFile.getImageCount(); i++)
		{
			Surface surface = TextureManager::make32BitFromPaletted(
				dfaFile.getWidth(), dfaFile.getHeight(), dfaFile.getPixels(i), palette);
			Texture texture = renderer.createTextureFromSurface(surface);
			textureSet.push_back(std::move(texture));
		}
	}
	else if (isFLC || isCEL)
	{
		FLCFile flcFile;
		if (!flcFile.init(filename.c_str()))
		{
			DebugCrash("Could not init .FLC/.CEL file \"" + filename + "\".");
		}

		// Create a texture for each frame in the .FLC.
		for (int i = 0; i < flcFile.getFrameCount(); i++)
		{
			Surface surface = TextureManager::make32BitFromPaletted(
				flcFile.getWidth(), flcFile.getHeight(), flcFile.getPixels(i),
				flcFile.getFramePalette(i));
			Texture texture = renderer.createTextureFromSurface(surface);
			textureSet.push_back(std::move(texture));
		}
	}
	else if (isRCI)
	{
		RCIFile rciFile;
		if (!rciFile.init(filename.c_str()))
		{
			DebugCrash("Could not init .RCI file \"" + filename + "\".");
		}

		// Create a texture for each image in the .RCI.
		for (int i = 0; i < rciFile.getImageCount(); i++)
		{
			Surface surface = TextureManager::make32BitFromPaletted(
				RCIFile::WIDTH, RCIFile::HEIGHT, rciFile.getPixels(i), palette);
			Texture texture = renderer.createTextureFromSurface(surface);
			textureSet.push_back(std::move(texture));
		}
	}
	else if (isSET)
	{
		SETFile setFile;
		if (!setFile.init(filename.c_str()))
		{
			DebugCrash("Could not init .SET file \"" + filename + "\".");
		}

		// Create a texture for each image in the .SET.
		for (int i = 0; i < setFile.getImageCount(); i++)
		{
			Surface surface = TextureManager::make32BitFromPaletted(
				SETFile::CHUNK_WIDTH, SETFile::CHUNK_HEIGHT, setFile.getPixels(i), palette);
			Texture texture = renderer.createTextureFromSurface(surface);
			textureSet.push_back(std::move(texture));
		}
	}
	else
	{
		DebugCrash("Unrecognized texture list \"" + filename + "\".");
	}

	// Set alpha transparency on for each texture.
	for (auto &texture : textureSet)
	{
		SDL_SetTextureBlendMode(texture.get(), SDL_BLENDMODE_BLEND);
	}

	return textureSet;
}

const std::vector<Texture> &TextureManager::getTextures(const std::string &filename,
	Renderer &renderer)
{
	return this->getTextures(filename, this->activePalette, renderer);
}

void TextureManager::init()
{
	DebugLog("Initializing.");

	// Load default palette.
	this->setPalette(PaletteFile::fromName(PaletteName::Default));
}

void TextureManager::setPalette(const std::string &paletteName)
{
	// Check if the palette hasn't already been loaded.
	if (this->palettes.find(paletteName) == this->palettes.end())
	{
		this->loadPalette(paletteName);
	}

	this->activePalette = paletteName;
}
