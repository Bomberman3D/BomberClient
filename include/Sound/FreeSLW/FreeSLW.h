// Copyright (c) 2012 Lukas Heise (http://www.lukasheise.com)
//
// This file is part of FreeSLW.
//
// FreeSLW is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// FreeSLW is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#ifdef FREESLW_EXPORTS
	#define FSLWAPI				__declspec(dllexport)
	#define FSLWAPIENTRY		__cdecl
#else
	#ifdef FREESLW_STATIC
		#define FSLWAPI
		#define FSLWAPIENTRY
	#else
		#define FSLWAPI			__declspec(dllimport)
		#define FSLWAPIENTRY	__cdecl
	#endif
#endif

#define FSLW_VERSION 100

#define IS_SOUND_3D(SOUND)	((SOUND)->GetFormat() == TA_MONO8 || (SOUND)->GetFormat() == TA_MONO16)
#define IS_SOUND_2D(SOUND)	((SOUND)->GetFormat() == TA_STEREO8 || (SOUND)->GetFormat() == TA_STEREO16)

namespace freeslw
{
	enum audioSubsystem_e
	{
		AS_OPENAL = 0
	};

	enum audioEnvironment_e
	{
		AE_GENERIC = 0,
		AE_PADDEDCELL,
		AE_ROOM,
		AE_BATHROOM,
		AE_LIVINGROOM,
		AE_STONEROOM,
		AE_AUDITORIUM,
		AE_CONCERTHALL,
		AE_CAVE,
		AE_ARENA,
		AE_HANGAR,
		AE_CARPETEDHALLWAY,
		AE_HALLWAY,
		AE_STONECORRIDOR,
		AE_ALLEY,
		AE_FOREST,
		AE_CITY,
		AE_MOUNTAINS,
		AE_QUARRY,
		AE_PLAIN,
		AE_PARKINGLOT,
		AE_SEWERPIPE,
		AE_UNDERWATER,
		AE_DRUGGED,
		AE_DIZZY,
		AE_PSYCHOTIC,

		AE_NONE
	};

	enum soundFormat_e
	{
		TA_MONO8 = 0,
		TA_MONO16,
		TA_STEREO8,
		TA_STEREO16
	};

	enum distanceModel_e
	{
		DM_INVERSE_DISTANCE = 0,
		DM_INVERSE_DISTANCE_CLAMPED,
		DM_LINEAR_DISTANCE,
		DM_LINEAR_DISTANCE_CLAMPED,
		DM_EXPONENT_DISTANCE,
		DM_EXPONENT_DISTANCE_CLAMPED
	};

	struct audioStatistics_t
	{
		size_t soundsPlaying;
		size_t soundsStreaming;
		size_t soundCount;
		size_t soundMemory; // may not be accurate, size in unit of kb (kilobytes)
	};

	class Listener
	{
	public:
		Listener() {}
		virtual ~Listener() {}

		virtual void SetPosition(float x, float y, float z) = 0;
		virtual void SetPosition(const float* xyz) = 0;
		virtual void GetPosition(float& x, float& y, float& z) const = 0;

		virtual void SetOrientation(float ax, float ay, float az, float ux, float uy, float uz) = 0;
		virtual void SetOrientation(const float* a, const float* u) = 0;
		virtual void GetOrientation(float& ax, float& ay, float& az, float& ux, float& uy, float& uz) const = 0;

		virtual void SetVelocity(float x, float y, float z) = 0;
		virtual void SetVelocity(const float* xyz) = 0;
		virtual void GetVelocity(float& x, float& y, float& z) const = 0;

		virtual void SetDistanceModel(distanceModel_e m) = 0;
		virtual distanceModel_e GetDistanceModel() const = 0;

		virtual void SetDopplerParameters(float factor, float velocity) = 0;
		virtual void GetDopplerParameters(float& factor, float& velocity) const = 0;

		virtual void SetEnvironment(audioEnvironment_e env) = 0;
		virtual audioEnvironment_e GetEnvironment() const = 0;
	};

	class Sound
	{
	public:
		Sound() {}
		virtual ~Sound() {}

		virtual const char* GetName() const = 0;
		virtual int GetFrequency() const = 0;
		virtual soundFormat_e GetFormat() const = 0;

		virtual bool IsStream() const = 0;
		
		virtual void SetGroup(unsigned char id) = 0;
		virtual unsigned char GetGroup() const = 0;

		virtual void Play() = 0;
		virtual void Resume() = 0;
		virtual void Rewind() = 0;
		virtual void Stop() = 0;
		virtual void Pause() = 0;
		virtual bool IsPlaying() = 0;
		virtual bool IsPaused() = 0;

		virtual void SetLooping(bool doLoop) = 0;
		virtual bool IsLooping() const = 0;
		
		virtual void SetPosition(float x, float y, float z) = 0;
		virtual void SetPosition(const float* xyz) = 0;
		virtual void GetPosition(float& x, float& y, float& z) const = 0;

		virtual void SetVelocity(float x, float y, float z) = 0;
		virtual void SetVelocity(const float* xyz) = 0;
		virtual void GetVelocity(float& x, float& y, float& z) const = 0;

		virtual void SetPitch(float p) = 0;
		virtual float GetPitch() const = 0;

		virtual void SetGain(float g) = 0;
		virtual float GetGain() const = 0;

		virtual void SetRolloff(float r) = 0;
		virtual float GetRolloff() const = 0;

		virtual void SetDistance(float refDistance, float maxDistance) = 0;
		virtual void GetDistance(float& refDistance, float& maxDistance) const = 0;

		virtual float GetPlayDuration() const = 0;								// total play time (length of sound), the actual playtime is: GetPlayDuration() / GetPitch()
		virtual float GetPlayTime() = 0;										// time in seconds this sound has played
		virtual void SetPlayTime(float seconds) = 0;							// set the current position in time to play

		virtual void FadeIn(float duration) = 0;
		virtual void FadeOut(float duration) = 0;
	};

	class AudioInterface
	{
	public:
		AudioInterface() {}
		virtual ~AudioInterface() {}

		virtual audioSubsystem_e GetSubsystem() const = 0;

		virtual void Update() = 0;

		virtual Listener* GetListener() = 0;
		virtual audioStatistics_t GetStatistics() = 0;
		virtual const char* GetSupportedStaticSoundFormats() const = 0;
		virtual const char* GetSupportedStreamSoundFormats() const = 0;

		virtual void SetVolume(float v) = 0;
		virtual float GetVolume() const = 0;

		virtual void SetMetersPerUnit(float s) = 0;
		virtual float GetMetersPerUnit() const = 0;

		virtual void SetGroup(unsigned char id, bool active) = 0;
		virtual bool IsGroupActive(unsigned char id) const = 0;

		virtual void StopAllSounds() = 0;

		virtual Sound* DuplicateSound(Sound* s) = 0;
		virtual Sound* LoadSound(const char* file, unsigned char group = 0, unsigned char priority = 0) = 0;
		virtual Sound* LoadSoundFromZip(const char* file, const char* package, unsigned char group = 0, unsigned char priority = 0) = 0;
		virtual Sound* LoadSoundFromData(const char* name, const char* type, const char* data, size_t size, unsigned char group = 0, unsigned char priority = 0) = 0;
		virtual Sound* StreamSound(const char* file, unsigned char group = 0, unsigned char priority = 0) = 0;
		virtual Sound* CreateSound(const char* name, const void* data, size_t size, soundFormat_e format, int frequency, unsigned char group = 0, unsigned char priority = 0) = 0;
		virtual void ReleaseSound(Sound* snd) = 0;
	};

	FSLWAPI int FSLWAPIENTRY GetVersion();
	FSLWAPI void FSLWAPIENTRY SetErrorCallback(void (*_error_callback)(const char* message));
	FSLWAPI AudioInterface* FSLWAPIENTRY GetInterface(audioSubsystem_e subsystem);
	FSLWAPI void FSLWAPIENTRY ReleaseInterface();
}