#pragma once

#include "platform/DynamicLibrary.h"
#include "platform/DynamicLibraryMemory.h"
#include "libretroplug/MessageBus.h"
#include "roms/Lsdj.h"
#include <mutex>
#include <atomic>
#include <vector>

enum class GameboyModel {
	Auto,
	DmgB,
	//SgbNtsc,
	//SgbPal,
	//Sgb2,
	CgbC,
	CgbE,
	Agb
};

struct SameboyPlugSymbols {
	void*(*sameboy_init)(void* user_data, const char* path, int model, bool fast_boot);
	void(*sameboy_free)(void* state);
	void(*sameboy_reset)(void* state, int model, bool fast_boot);

	void(*sameboy_update)(void* state, size_t requiredAudioFrames);
	void(*sameboy_update_multiple)(void** states, size_t stateCount, size_t requiredAudioFrames);

	void(*sameboy_set_sample_rate)(void* state, double sample_rate);
	void(*sameboy_set_setting)(void* state, const char* name, int value);
	void(*sameboy_disable_rendering)(void* state, bool disabled);

	void(*sameboy_set_midi_bytes)(void* state, int offset, const char* bytes, size_t count);
	void(*sameboy_set_button)(void* state, int buttonId, bool down);
	void(*sameboy_set_link_targets)(void* state, void** linkTargets, size_t count);

	size_t(*sameboy_battery_size)(void* state);
	void(*sameboy_load_battery)(void* state, const char* source, size_t size);
	size_t(*sameboy_save_battery)(void* state, const char* target, size_t size);

	size_t(*sameboy_save_state_size)(void* state);
	void(*sameboy_load_state)(void* state, const char* source, size_t size);
	void(*sameboy_save_state)(void* state, char* target, size_t size);

	size_t(*sameboy_fetch_audio)(void* state, int16_t* audio);
	size_t(*sameboy_fetch_video)(void* state, uint32_t* video);

	const char*(*sameboy_get_rom_name)(void* state);
};

class SameBoyPlug;
using SameBoyPlugPtr = std::shared_ptr<SameBoyPlug>;

class SameBoyPlug {
private:
	DynamicLibraryMemory _library;
	//DynamicLibrary _library;
	SameboyPlugSymbols _symbols = { nullptr };

	void* _instance = nullptr;

	std::wstring _romPath;
	std::wstring _savePath;
	std::string _romName;

	MessageBus _bus;

	std::mutex _lock;
	std::atomic<bool> _midiSync = false;
	std::atomic<bool> _gameLink = false;
	std::atomic<int> _resetSamples = 0;

	Lsdj _lsdj;
	GameboyModel _model = GameboyModel::Auto;

	double _sampleRate = 48000;

public:
	SameBoyPlug();
	~SameBoyPlug() { shutdown(); }

	Lsdj& lsdj() { return _lsdj; }

	GameboyModel model() const { return _model; }

	bool midiSync() { return _midiSync.load(); }

	void setMidiSync(bool enabled) { _midiSync = enabled; }

	bool gameLink() const { return _gameLink.load(); }

	void setGameLink(bool enabled) { _gameLink = enabled; }

	void init(const std::wstring& romPath, GameboyModel model, bool fastBoot);

	void reset(GameboyModel model, bool fast);

	bool active() const { return _instance != nullptr; }

	const std::string& romName() const { return _romName; }

	const std::wstring& romPath() const { return _romPath; }

	std::mutex& lock() { return _lock; }

	void setSampleRate(double sampleRate);

	void sendMidiByte(int offset, char byte) { sendMidiBytes(offset, &byte, 1); }

	void sendMidiBytes(int offset, const char* bytes, size_t count);

	size_t saveStateSize();

	bool saveBattery(std::wstring path);

	bool saveBattery(std::vector<char>& data);

	bool loadBattery(const std::wstring& path, bool reset);

	bool loadBattery(const std::vector<char>& path, bool reset);

	void saveState(char* target, size_t size);

	void loadState(const char* source, size_t size);

	void setSetting(const std::string& name, int value);

	void setLinkTargets(std::vector<SameBoyPlugPtr> linkTargets);

	void setButtonState(const ButtonEvent& ev) { _bus.buttons.writeValue(ev); }

	MessageBus* messageBus() { return &_bus; }

	void update(size_t audioFrames);

	void updateMultiple(SameBoyPlug** plugs, size_t plugCount, size_t audioFrames);

	void shutdown();

	void* instance() { return _instance; }

	void disableRendering(bool disable);

	void setSavePath(const std::wstring& path) { _savePath = path; }

	const std::wstring& savePath() const { return _savePath; }

private:
	void updateButtons();

	void updateAV(int audioFrames);
};


