/**
 * @file dx.cpp
 *
 * Implementation of functions setting up the graphics pipeline.
 */
#include "engine/dx.h"

#include <cstdint>

#ifdef USE_SDL3
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#else
#include <SDL.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "controls/control_mode.hpp"
#include "controls/plrctrls.h"
#include "engine/render/primitive_render.hpp"
#include "headless_mode.hpp"
#include "init.hpp"
#include "options.h"
#include "utils/display.h"
#include "utils/log.hpp"
#include "utils/sdl_wrap.h"
#include "utils/static_vector.hpp"

#ifndef USE_SDL1
#include "controls/touch/renderers.h"
#endif

#ifdef __3DS__
#include <3ds.h>
#endif

namespace devilution {

int refreshDelay;
SDL_Renderer *renderer;
#ifndef USE_SDL1
SDLTextureUniquePtr texture;
#ifdef PSP
SDLTextureUniquePtr PspRightTexture;

namespace {
struct PspFrameProfile {
	uint64_t draw = 0;
	uint64_t blit = 0;
	uint64_t clear = 0;
	uint64_t upload = 0;
	uint64_t copy = 0;
	uint64_t present = 0;
	uint64_t frame = 0;
	uint64_t lastFrameEnd = 0;
	uint64_t pendingDraw = 0;
	uint64_t pendingBlit = 0;
	int samples = 0;
	int intervals = 0;
	int width = 0;
};
PspFrameProfile PspProfile;
} // namespace

void PspProfileRecordDraw(uint64_t ticks)
{
	PspProfile.pendingDraw += ticks;
}

void PspProfileRecordBlit(uint64_t ticks)
{
	PspProfile.pendingBlit += ticks;
}
#endif
#endif

/** Currently active palette */
SDLPaletteUniquePtr Palette;

/** 24-bit renderer texture surface */
SDLSurfaceUniquePtr RendererTextureSurface;

/** 8-bit surface that we render to */
SDL_Surface *PalSurface;
namespace {
SDLSurfaceUniquePtr PinnedPalSurface;

#ifndef USE_SDL1
constexpr size_t MaxDirtyRects = 16;
StaticVector<SDL_Rect, MaxDirtyRects> DirtyRects;
bool ForceFullUpdate;
const void *LastPresentedPixels;
#endif
} // namespace

/** Whether we render directly to the screen surface, i.e. `PalSurface == GetOutputSurface()` */
bool RenderDirectlyToOutputSurface;

namespace {

bool CanRenderDirectlyToOutputSurface()
{
#ifdef USE_SDL1
#ifdef SDL1_FORCE_DIRECT_RENDER
	return true;
#else
	auto *outputSurface = GetOutputSurface();
	return ((outputSurface->flags & SDL_DOUBLEBUF) == SDL_DOUBLEBUF
	    && outputSurface->w == gnScreenWidth && outputSurface->h == gnScreenHeight
	    && outputSurface->format->BitsPerPixel == 8);
#endif
#else // !USE_SDL1
	if (renderer != nullptr) return false;
	SDL_Surface *outputSurface = GetOutputSurface();
	// Assumes double-buffering is available.
	return outputSurface->w == static_cast<int>(gnScreenWidth)
	    && outputSurface->h == static_cast<int>(gnScreenHeight)
	    && SDLC_SURFACE_BITSPERPIXEL(outputSurface) == 8;
#endif
}

/**
 * @brief Limit FPS to avoid high CPU load, use when v-sync isn't available
 */
void LimitFrameRate()
{
	if (*GetOptions().Graphics.frameRateControl != FrameRateControl::CPUSleep)
		return;
	static uint32_t frameDeadline;
	const uint32_t tc = SDL_GetTicks() * 1000;
	uint32_t v = 0;
	if (frameDeadline > tc) {
		v = tc % refreshDelay;
		SDL_Delay((v / 1000) + 1); // ceil
	}
	frameDeadline = tc + v + refreshDelay;
}

} // namespace

void dx_init()
{
#ifndef USE_SDL1
	SDL_RaiseWindow(ghMainWnd);
	SDL_ShowWindow(ghMainWnd);
#endif

	Palette = SDLWrap::AllocPalette();
	palette_init();
	CreateBackBuffer();
}

Surface GlobalBackBuffer()
{
	return Surface(PalSurface, SDL_Rect { 0, 0, gnScreenWidth, gnScreenHeight });
}

void dx_cleanup()
{
#ifndef USE_SDL1
	if (ghMainWnd != nullptr)
		SDL_HideWindow(ghMainWnd);
#endif

	PalSurface = nullptr;
	PinnedPalSurface = nullptr;
	Palette = nullptr;
	RendererTextureSurface = nullptr;
#ifndef USE_SDL1
	texture = nullptr;
#ifdef PSP
	PspRightTexture = nullptr;
#endif
	FreeVirtualGamepadTextures();
	if (*GetOptions().Graphics.upscale)
		SDL_DestroyRenderer(renderer);
#endif
	SDL_DestroyWindow(ghMainWnd);
}

void CreateBackBuffer()
{
	if (CanRenderDirectlyToOutputSurface()) {
		Log("{}", "Will render directly to the SDL output surface");
		PalSurface = GetOutputSurface();
		RenderDirectlyToOutputSurface = true;
	} else {
#ifdef PSP
		// Release the old 8-bit buffer before allocating a wider one when
		// changing the PSP resolution from the menu.
		PinnedPalSurface = nullptr;
		PalSurface = nullptr;
#endif
		PinnedPalSurface = SDLWrap::CreateRGBSurfaceWithFormat(
		    /*flags=*/0,
		    /*width=*/gnScreenWidth,
		    /*height=*/gnScreenHeight,
		    /*depth=*/8,
		    SDL_PIXELFORMAT_INDEX8);
		PalSurface = PinnedPalSurface.get();
	}

#if defined(USE_SDL3)
	if (!SDL_SetSurfacePalette(PalSurface, Palette.get())) ErrSdl();
#elif !defined(USE_SDL1)
	// In SDL2, `PalSurface` points to the global `palette`.
	if (SDL_SetSurfacePalette(PalSurface, Palette.get()) < 0)
		ErrSdl();
#else
	// In SDL1, `PalSurface` owns its palette and we must update it every
	// time the global `palette` is changed. No need to do anything here as
	// the global `palette` doesn't have any colors set yet.
#endif
}

void BltFast(SDL_Rect *srcRect, SDL_Rect *dstRect)
{
	if (RenderDirectlyToOutputSurface) {
#ifndef USE_SDL1
		// A null rect means the caller changed the entire surface.
		if (dstRect == nullptr || DirtyRects.size() == MaxDirtyRects)
			ForceFullUpdate = true;
		else
			DirtyRects.push_back(*dstRect);
#endif
		return;
	}
#if defined(PSP) && !defined(USE_SDL1)
	const uint64_t pspBlitStart = SDL_GetPerformanceCounter();
#endif
	Blit(PalSurface, srcRect, dstRect);
#if defined(PSP) && !defined(USE_SDL1)
	PspProfileRecordBlit(SDL_GetPerformanceCounter() - pspBlitStart);
#endif
}

void Blit(SDL_Surface *src, SDL_Rect *srcRect, SDL_Rect *dstRect)
{
	if (HeadlessMode)
		return;

	SDL_Surface *dst = GetOutputSurface();
#if defined(USE_SDL3)
	if (!SDL_BlitSurface(src, srcRect, dst, dstRect)) ErrSdl();
#elif !defined(USE_SDL1)
	if (SDL_BlitSurface(src, srcRect, dst, dstRect) < 0)
		ErrSdl();
#else
	if (!OutputRequiresScaling()) {
		if (SDL_BlitSurface(src, srcRect, dst, dstRect) < 0)
			ErrSdl();
		return;
	}

	SDL_Rect scaledDstRect;
	if (dstRect != NULL) {
		scaledDstRect = *dstRect;
		ScaleOutputRect(&scaledDstRect);
		dstRect = &scaledDstRect;
	}

	// Same pixel format: We can call BlitScaled directly.
	if (SDLBackport_PixelFormatFormatEq(src->format, dst->format)) {
		if (SDL_BlitScaled(src, srcRect, dst, dstRect) < 0)
			ErrSdl();
		return;
	}

	// If the surface has a color key, we must stretch first and can then call BlitSurface.
	if (SDL_HasColorKey(src)) {
		SDLSurfaceUniquePtr stretched = SDLWrap::CreateRGBSurface(SDL_SWSURFACE, dstRect->w, dstRect->h, src->format->BitsPerPixel,
		    src->format->Rmask, src->format->Gmask, src->format->BitsPerPixel, src->format->Amask);
		SDL_SetColorKey(stretched.get(), SDL_SRCCOLORKEY, src->format->colorkey);
		if (src->format->palette != NULL)
			SDL_SetPalette(stretched.get(), SDL_LOGPAL, src->format->palette->colors, 0, src->format->palette->ncolors);
		SDL_Rect stretched_rect = { 0, 0, dstRect->w, dstRect->h };
		if (SDL_SoftStretch(src, srcRect, stretched.get(), &stretched_rect) < 0
		    || SDL_BlitSurface(stretched.get(), &stretched_rect, dst, dstRect) < 0) {
			ErrSdl();
		}
		return;
	}

	// A surface with a non-output pixel format but without a color key needs scaling.
	// We can convert the format and then call BlitScaled.
	SDLSurfaceUniquePtr converted = SDLWrap::ConvertSurface(src, dst->format, 0);
	if (SDL_BlitScaled(converted.get(), srcRect, dst, dstRect) < 0)
		ErrSdl();
#endif
}

#ifndef USE_SDL1
namespace {

void UpdateOutputSurface()
{

	// Only do partial blitting if the output buffer is the same as last frame.
	const SDL_Surface *outputSurface = GetOutputSurface();
	const void *pixels = outputSurface != nullptr ? outputSurface->pixels : nullptr;
	const void *lastPixels = LastPresentedPixels;
	LastPresentedPixels = pixels;

	if (!ForceFullUpdate && !DirtyRects.empty() && pixels == lastPixels) {
		const int numRects = static_cast<int>(DirtyRects.size());
#ifdef USE_SDL3
		const bool updated = SDL_UpdateWindowSurfaceRects(ghMainWnd, DirtyRects.data(), numRects);
#else
		const bool updated = SDL_UpdateWindowSurfaceRects(ghMainWnd, DirtyRects.data(), numRects) >= 0;
#endif
		DirtyRects.clear();
		if (!updated) ErrSdl();
		return;
	}
	DirtyRects.clear();
	ForceFullUpdate = false;

#ifdef USE_SDL3
	if (!SDL_UpdateWindowSurface(ghMainWnd)) ErrSdl();
#else
	if (SDL_UpdateWindowSurface(ghMainWnd) <= -1) ErrSdl();
#endif
}

} // namespace
#endif

void RenderPresent()
{
	if (HeadlessMode)
		return;

	SDL_Surface *surface = GetOutputSurface();

	if (!gbActive) {
#ifdef __EMSCRIPTEN__
		// Just yield to browser when inactive instead of blocking
		emscripten_sleep(1);
#else
		LimitFrameRate();
#endif
		return;
	}

#ifndef USE_SDL1
	if (renderer != nullptr) {
#ifdef USE_SDL3
		if (!SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255)) ErrSdl();
		if (!SDL_RenderClear(renderer)) ErrSdl();
		if (!SDL_UpdateTexture(texture.get(), nullptr, surface->pixels, surface->pitch)) ErrSdl();
		if (!SDL_RenderTexture(renderer, texture.get(), nullptr, nullptr)) ErrSdl();
#else
#ifdef PSP
		const uint64_t pspClearStart = SDL_GetPerformanceCounter();
#endif
		if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) <= -1) ErrSdl();
		if (SDL_RenderClear(renderer) <= -1) ErrSdl();
#ifdef PSP
		const uint64_t pspUploadStart = SDL_GetPerformanceCounter();
		const auto *pixels = static_cast<const std::uint8_t *>(surface->pixels);
		const int firstTextureWidth = std::min<int>(gnScreenWidth, PspFirstTextureWidth);
		const auto *rightPixels = pixels + firstTextureWidth * surface->format->BytesPerPixel;

		if (SDL_UpdateTexture(texture.get(), nullptr, pixels, surface->pitch) <= -1)
			ErrSdl();

		uint64_t pspCopyStart;
		if (PspRightTexture != nullptr) {
			if (SDL_UpdateTexture(PspRightTexture.get(), nullptr, rightPixels, surface->pitch) <= -1)
				ErrSdl();

			pspCopyStart = SDL_GetPerformanceCounter();
			// PSPDEV's SDL2 PSP renderer ignores the logical viewport offset when
			// drawing textures. Apply it here so the image is positioned correctly.
			SDL_Rect viewport;
			SDL_RenderGetViewport(renderer, &viewport);

			const SDL_Rect leftRect = { viewport.x, viewport.y, firstTextureWidth, gnScreenHeight };
			const SDL_Rect rightRect = { viewport.x + firstTextureWidth, viewport.y, gnScreenWidth - firstTextureWidth, gnScreenHeight };

			if (SDL_RenderCopy(renderer, texture.get(), nullptr, &leftRect) <= -1
			    || SDL_RenderCopy(renderer, PspRightTexture.get(), nullptr, &rightRect) <= -1)
				ErrSdl();
		} else {
			pspCopyStart = SDL_GetPerformanceCounter();
			if (SDL_RenderCopy(renderer, texture.get(), nullptr, nullptr) <= -1)
				ErrSdl();
		}
		const uint64_t pspPresentStart = SDL_GetPerformanceCounter();
#else
		if (SDL_UpdateTexture(texture.get(), nullptr, surface->pixels, surface->pitch) <= -1) ErrSdl();
		if (SDL_RenderCopy(renderer, texture.get(), nullptr, nullptr) <= -1) ErrSdl();
#endif
#endif

		if (ControlMode == ControlTypes::VirtualGamepad) {
			RenderVirtualGamepad(renderer);
		}
		SDL_RenderPresent(renderer);
#ifdef PSP
		const uint64_t pspFrameEnd = SDL_GetPerformanceCounter();
		if (PspProfile.pendingDraw != 0) {
			if (PspProfile.width != gnScreenWidth) {
				const uint64_t pendingDraw = PspProfile.pendingDraw;
				const uint64_t pendingBlit = PspProfile.pendingBlit;
				PspProfile = {};
				PspProfile.pendingDraw = pendingDraw;
				PspProfile.pendingBlit = pendingBlit;
			}
			PspProfile.width = gnScreenWidth;
			PspProfile.draw += PspProfile.pendingDraw;
			PspProfile.blit += PspProfile.pendingBlit;
			PspProfile.clear += pspUploadStart - pspClearStart;
			PspProfile.upload += pspCopyStart - pspUploadStart;
			PspProfile.copy += pspPresentStart - pspCopyStart;
			PspProfile.present += pspFrameEnd - pspPresentStart;
			if (PspProfile.lastFrameEnd != 0) {
				PspProfile.frame += pspFrameEnd - PspProfile.lastFrameEnd;
				++PspProfile.intervals;
			}
			PspProfile.lastFrameEnd = pspFrameEnd;
			++PspProfile.samples;
			if (PspProfile.samples == 120) {
				const double frequency = static_cast<double>(SDL_GetPerformanceFrequency());
				const auto milliseconds = [frequency](uint64_t ticks, int count) {
					return count == 0 ? 0.0 : 1000.0 * static_cast<double>(ticks) / frequency / count;
				};
				Log("PSP PERF {}x{}: frame={:.1f} draw={:.1f} blit={:.1f} clear={:.1f} upload={:.1f} copy={:.1f} present={:.1f} ms ({} frames)",
				    gnScreenWidth, gnScreenHeight, milliseconds(PspProfile.frame, PspProfile.intervals),
				    milliseconds(PspProfile.draw, PspProfile.samples), milliseconds(PspProfile.blit, PspProfile.samples),
				    milliseconds(PspProfile.clear, PspProfile.samples), milliseconds(PspProfile.upload, PspProfile.samples),
				    milliseconds(PspProfile.copy, PspProfile.samples), milliseconds(PspProfile.present, PspProfile.samples), PspProfile.samples);
				PspProfile = {};
				PspProfile.width = gnScreenWidth;
				// Exclude the SD-card log write from the next frame interval.
			}
		} else {
			PspProfile = {};
		}
		PspProfile.pendingDraw = 0;
		PspProfile.pendingBlit = 0;
#endif

#ifdef __EMSCRIPTEN__
		// TODO: Refactor to use emscripten_set_main_loop or requestAnimationFrame instead.
		// For now, yield to browser to allow rendering via ASYNCIFY sleep.
		emscripten_sleep(1);
#endif

		if (*GetOptions().Graphics.frameRateControl != FrameRateControl::VerticalSync) {
			LimitFrameRate();
		}
	} else {
		if (ControlMode == ControlTypes::VirtualGamepad) {
			RenderVirtualGamepad(surface);
		}

		UpdateOutputSurface();

		if (RenderDirectlyToOutputSurface)
			PalSurface = GetOutputSurface();
		LimitFrameRate();
	}
#else
	if (SDL_Flip(surface) <= -1) {
		ErrSdl();
	}
	if (RenderDirectlyToOutputSurface)
		PalSurface = GetOutputSurface();
	LimitFrameRate();
#endif
}

} // namespace devilution
