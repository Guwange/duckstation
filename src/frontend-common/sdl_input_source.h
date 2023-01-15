// SPDX-FileCopyrightText: 2019-2022 Connor McLaughlin <stenzek@gmail.com>
// SPDX-License-Identifier: (GPL-3.0 OR CC-BY-NC-ND-4.0)

#pragma once
#include "SDL.h"
#include "input_source.h"
#include <array>
#include <functional>
#include <mutex>
#include <vector>

class SettingsInterface;

class SDLInputSource final : public InputSource
{
public:
  SDLInputSource();
  ~SDLInputSource();

  bool Initialize(SettingsInterface& si, std::unique_lock<std::mutex>& settings_lock) override;
  void UpdateSettings(SettingsInterface& si, std::unique_lock<std::mutex>& settings_lock) override;
  bool ReloadDevices() override;
  void Shutdown() override;

  void PollEvents() override;
  std::vector<std::pair<std::string, std::string>> EnumerateDevices() override;
  std::vector<InputBindingKey> EnumerateMotors() override;
  bool GetGenericBindingMapping(const std::string_view& device, GenericInputBindingMapping* mapping) override;
  void UpdateMotorState(InputBindingKey key, float intensity) override;
  void UpdateMotorState(InputBindingKey large_key, InputBindingKey small_key, float large_intensity,
                        float small_intensity) override;

  std::optional<InputBindingKey> ParseKeyString(const std::string_view& device,
                                                const std::string_view& binding) override;
  std::string ConvertKeyToString(InputBindingKey key) override;

  bool ProcessSDLEvent(const SDL_Event* event);

  SDL_Joystick* GetJoystickForDevice(const std::string_view& device);

private:
  struct ControllerData
  {
    SDL_Haptic* haptic;
    SDL_GameController* game_controller;
    SDL_Joystick* joystick;
    u16 rumble_intensity[2];
    int haptic_left_right_effect;
    int joystick_id;
    int player_id;
    bool use_game_controller_rumble;

    // Used to disable Joystick controls that are used in GameController inputs so we don't get double events
    std::vector<bool> joy_button_used_in_gc;
    std::vector<bool> joy_axis_used_in_gc;

    // Track last hat state so we can send "unpressed" events.
    std::vector<u8> last_hat_state;
  };

  using ControllerDataVector = std::vector<ControllerData>;

  bool InitializeSubsystem();
  void ShutdownSubsystem();
  void LoadSettings(SettingsInterface& si);
  void SetHints();

  ControllerDataVector::iterator GetControllerDataForJoystickId(int id);
  ControllerDataVector::iterator GetControllerDataForPlayerId(int id);
  int GetFreePlayerId() const;

  bool OpenDevice(int index, bool is_gamecontroller);
  bool CloseDevice(int joystick_index);
  bool HandleControllerAxisEvent(const SDL_ControllerAxisEvent* ev);
  bool HandleControllerButtonEvent(const SDL_ControllerButtonEvent* ev);
  bool HandleJoystickAxisEvent(const SDL_JoyAxisEvent* ev);
  bool HandleJoystickButtonEvent(const SDL_JoyButtonEvent* ev);
  bool HandleJoystickHatEvent(const SDL_JoyHatEvent* ev);
  void SendRumbleUpdate(ControllerData* cd);

  ControllerDataVector m_controllers;

  bool m_sdl_subsystem_initialized = false;
  bool m_controller_enhanced_mode = false;
  std::vector<std::pair<std::string, std::string>> m_sdl_hints;
};
