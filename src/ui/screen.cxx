#include "src/ui/screen.hxx"
#include <Magnum/ImGuiIntegration/Context.hpp>
#include <Magnum/Platform/Sdl2Application.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <ranges>
namespace ImGui
{
inline void
PushDisabled (bool enabled)
{
  if (enabled)
    {
      extern void PushItemFlag (int option, bool enabled);
      PushItemFlag (1 << 2 /*ImGuiItemFlags_Disabled*/, !false);
      PushStyleVar (ImGuiStyleVar_Alpha, ImGui::GetStyle ().Alpha * (false ? 1.0f : 0.5f));
    }
}

inline void
PopDisabled (bool enabled)
{
  if (enabled)
    {
      extern void PopItemFlag ();
      PopItemFlag ();
      PopStyleVar ();
    }
}

} // namespace ImGui

// namespace ImGui
// {
// void
// LoadingIndicatorCircle (const char *label, const float indicator_radius, const ImVec4 &main_color, const ImVec4 &backdrop_color, const int circle_count, const float speed)
// {
//   ImGuiWindow *window = GetCurrentWindow ();
//   if (window->SkipItems)
//     {
//       return;
//     }

//   ImGuiContext &g = *GImGui;
//   const ImGuiID id = window->GetID (label);

//   ImVec2 pos = window->DC.CursorPos;

//   const float circle_radius = indicator_radius / 8.0f;
//   // pos.y += (indicator_radius / 2) - (circle_radius * 2);
//   // pos.x -= 270;
//   const ImRect bb (pos, ImVec2 (pos.x + indicator_radius * 2.0f, pos.y + indicator_radius * 2.0f));
//   ItemSize (bb, 1);
//   if (!ItemAdd (bb, id))
//     {
//       return;
//     }
//   const float t = g.Time;
//   const auto degree_offset = 2.0f * IM_PI / circle_count;
//   for (int i = 0; i < circle_count; ++i)
//     {
//       const auto x = indicator_radius * std::sin (degree_offset * i);
//       const auto y = indicator_radius * std::cos (degree_offset * i);
//       const auto growth = std::max (0.0f, std::sin (t * speed - i * degree_offset));
//       ImVec4 color;
//       color.x = main_color.x * growth + backdrop_color.x * (1.0f - growth);
//       color.y = main_color.y * growth + backdrop_color.y * (1.0f - growth);
//       color.z = main_color.z * growth + backdrop_color.z * (1.0f - growth);
//       color.w = growth;
//       window->DrawList->AddCircleFilled (ImVec2 (pos.x + indicator_radius + x, pos.y + indicator_radius - y), circle_radius + growth * circle_radius, GetColorU32 (color));
//     }
// }
// }

void
chatScreen (ChatData &chatData, bool shouldLockScreen)
{
  ImGui::PushDisabled (shouldLockScreen);
  ImGui::Text ("Join Channel");
  ImGui::InputText ("##JoinChannel", &chatData.channelToJoin);
  chatData.joinChannelClicked = ImGui::Button ("Join Channel", ImVec2 (-1, 0));
  ImGui::PopDisabled (shouldLockScreen);
  auto channelNames = chatData.channelNames ();
  if (ImGui::BeginCombo ("##combo 1", chatData.selectChannelComboBoxName ().c_str ()))
    {
      if ((not chatData.selectedChannelName && not channelNames.empty ()) || (chatData.selectedChannelName && not channelNames.empty () && std::ranges::find (channelNames, chatData.selectedChannelName) == channelNames.end ()))
        {
          chatData.selectedChannelName = channelNames.front ();
        }
      for (auto &&channelName : channelNames)
        {
          const bool is_selected = (chatData.selectedChannelName == channelName);
          if (ImGui::Selectable (channelName.data (), is_selected)) chatData.selectedChannelName = channelName;
          if (is_selected) ImGui::SetItemDefaultFocus ();
        }
      ImGui::EndCombo ();
    }
  ImGui::BeginChild ("scrolling", ImVec2 (0, 500), false, ImGuiWindowFlags_HorizontalScrollbar);
  if (chatData.selectedChannelName && std::ranges::find (channelNames, chatData.selectedChannelName) != channelNames.end ())
    {
      for (auto const &text : chatData.channelMessages.at (chatData.selectedChannelName.value ()))
        {
          ImGui::TextUnformatted (text.data (), text.data () + text.size ());
        }
      if (ImGui::GetScrollY () >= ImGui::GetScrollMaxY ()) ImGui::SetScrollHereY (1.0f);
    }
  ImGui::EndChild ();
  ImGui::PushDisabled (shouldLockScreen);
  ImGui::Text ("Send to Channel");
  ImGui::InputText ("##SendToChannel", &chatData.messageToSendToChannel);
  chatData.sendMessageClicked = ImGui::Button ("Send to Channel", ImVec2 (-1, 0));
  ImGui::PopDisabled (shouldLockScreen);
}

void
createGameLobbyScreen (CreateGameLobby &createGameLobby, std::optional<WaitForServer> &waitForServer, std::string accountName, ChatData &chatData)
{
  // TODO allow joinin a game
  auto const shouldLockScreen = waitForServer.has_value ();
  chatScreen (chatData, shouldLockScreen);
  if (not createGameLobby.accountNamesInGameLobby.empty () && accountName == createGameLobby.accountNamesInGameLobby.at (0))
    {
      ImGui::Text ("set max user count: ");
      ImGui::PushDisabled (shouldLockScreen);
      ImGui::InputInt ("##MaxUserCount", &createGameLobby.maxUserInGameLobby);
      ImGui::PopDisabled (shouldLockScreen);
      ImGui::PushDisabled (shouldLockScreen);
      createGameLobby.sendMaxUserCountClicked = ImGui::Button ("set max user count", ImVec2 (-1, 0));
      ImGui::PopDisabled (shouldLockScreen);
    }
  else
    {
      ImGui::Text (std::string{ "max user count: " + std::to_string (createGameLobby.maxUserInGameLobby) }.c_str ());
    }
  ImGui::Text ("user in lobby:");
  for (auto &lobbyMemberAccountName : createGameLobby.accountNamesInGameLobby)
    {
      ImGui::Text (lobbyMemberAccountName.c_str ());
    }
  ImGui::PushDisabled (shouldLockScreen);
  createGameLobby.startGame = ImGui::Button ("Start Game", ImVec2 (-1, 0));
  createGameLobby.leaveGameLobby = ImGui::Button ("Leave Game Lobby", ImVec2 (-1, 0));
  ImGui::PopDisabled (shouldLockScreen);
}

void
messageBoxPopupScreen (MessageBoxPopup &messageBoxPopup, float windowWidth, float windowHeight, ImFont &biggerFont)
{
  ImGui::Dummy (ImVec2 (0.0f, (windowHeight - ((ImGui::GetFontSize () + ImGui::GetStyle ().ItemSpacing.y * 2))) / 3));
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
  ImGui::PushStyleVar (ImGuiStyleVar_ChildRounding, 5.0f);
  ImGui::Dummy (ImVec2 ((windowWidth - (8 * ImGui::GetStyle ().ItemSpacing.x)) / 2, 0.0f));
  ImGui::PushFont (&biggerFont);
  ImGui::PopFont ();
  ImGui::Dummy (ImVec2 (windowWidth / 4, 0.0f));
  ImGui::SameLine ();
  ImGui::BeginChild ("ChildR", ImVec2 (windowWidth / 2, (1 * (ImGui::GetFontSize () + ImGui::GetStyle ().ItemSpacing.y * 2)) + 40), true, window_flags);
  ImGui::Dummy (ImVec2 (0.0f, 10.0f));
  ImGui::Dummy (ImVec2 (10.0f, 0.0f));
  ImGui::SameLine ();
  ImGui::PushStyleVar (ImGuiStyleVar_ChildBorderSize, 0);
  ImGui::BeginChild ("ChildR_sub", ImVec2 ((windowWidth / 2) - 50, (1 * (ImGui::GetFontSize () + ImGui::GetStyle ().ItemSpacing.y * 2)) + 10), true, window_flags);
  ImGui::PopStyleVar ();
  ImGui::Text (messageBoxPopup.message.c_str ());
  ImGui::EndChild ();
  ImGui::EndChild ();
  ImGui::Dummy (ImVec2 (windowWidth / 4, 0.0f));
  for (auto &button : messageBoxPopup.buttons)
    {
      ImGui::SameLine ();
      ImGui::PushDisabled (button.disabled);
      button.pressed = ImGui::Button (button.name.c_str ());
      ImGui::PopDisabled (button.disabled);
    }
  ImGui::PopStyleVar ();
}

void
loginScreen (Login &data, std::optional<WaitForServer> &waitForServer, float windowWidth, float windowHeight, ImFont &biggerFont)
{
  auto const shouldLockScreen = waitForServer.has_value ();
  ImGui::Dummy (ImVec2 (0.0f, (windowHeight - (5 * (ImGui::GetFontSize () + ImGui::GetStyle ().ItemSpacing.y * 2))) / 3));
  ImGui::Dummy (ImVec2 ((windowWidth - ImGui::CalcTextSize ("Sign in to XYZ").x - (8 * ImGui::GetStyle ().ItemSpacing.x)) / 2, 0.0f));
  ImGui::SameLine ();
  ImGui::PushFont (&biggerFont);
  ImGui::Text ("Sign in to XYZ");
  ImGui::PopFont ();
  ImGui::Dummy (ImVec2 (windowWidth / 4, 0.0f));
  ImGui::SameLine ();
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar;
  ImGui::PushStyleVar (ImGuiStyleVar_ChildRounding, 5.0f);
  ImGui::BeginChild ("ChildR", ImVec2 (windowWidth / 2, (5 * (ImGui::GetFontSize () + ImGui::GetStyle ().ItemSpacing.y * 2)) + 50), true, window_flags);
  ImGui::PopStyleVar ();
  ImGui::Dummy (ImVec2 (0.0f, 10.0f));
  ImGui::Dummy (ImVec2 (10.0f, 0.0f));
  ImGui::SameLine ();
  ImGui::BeginChild ("ChildR_sub", ImVec2 ((windowWidth / 2) - 50, (5 * (ImGui::GetFontSize () + ImGui::GetStyle ().ItemSpacing.y * 2)) + 30), false, window_flags);
  ImGui::PushItemWidth (-1.0f);
  ImGui::Text ("Username");
  ImGui::PushDisabled (shouldLockScreen);
  ImGui::InputText ("##username", &data.accountName);
  ImGui::PopDisabled (shouldLockScreen);
  ImGui::Text ("Password");
  ImGui::PushDisabled (shouldLockScreen);
  ImGui::InputText ("##password", &data.password, ImGuiInputTextFlags_Password);
  ImGui::PopDisabled (shouldLockScreen);
  if (shouldLockScreen)
    {
      auto &button = waitForServer->buttons.front ();
      using namespace std::chrono_literals;
      if (waitForServer->elapsedTime () >= 5s)
        {
          button.pressed = ImGui::Button (button.name.c_str (), ImVec2 (-1, 0));
        }
      else
        {
          ImGui::PushDisabled (shouldLockScreen);
          ImGui::Button (button.name.c_str (), ImVec2 (-1, 0));
          ImGui::PopDisabled (shouldLockScreen);
        }
    }
  else
    {
      data.loginClicked = ImGui::Button ("Sign in", ImVec2 (-1, 0));
    }
  ImGui::PopItemWidth ();
  ImGui::EndChild ();
  ImGui::EndChild ();
  ImGui::Dummy (ImVec2 (windowWidth / 4, 0.0f));
  ImGui::SameLine ();
  ImGui::PushStyleVar (ImGuiStyleVar_ChildRounding, 5.0f);
  ImGui::BeginChild ("ChildR123", ImVec2 (windowWidth / 2, (1 * (ImGui::GetFontSize () + ImGui::GetStyle ().ItemSpacing.y * 2)) + 20), true, window_flags);
  ImGui::Dummy (ImVec2 (((windowWidth / 2) - (ImGui::CalcTextSize ("New to XYZ?").x + ImGui::CalcTextSize ("create an account").x + (ImGui::GetStyle ().ItemSpacing.x * 6))) / 2, 0.0f));
  ImGui::SameLine ();
  ImGui::Text ("New to XYZ?");
  ImGui::SameLine ();
  ImGui::PushDisabled (shouldLockScreen);
  data.createAccountClicked = ImGui::SmallButton ("Create an Account");
  ImGui::PopDisabled (shouldLockScreen);
  ImGui::PopStyleVar ();
  ImGui::EndChild ();
}

void
createAccountScreen (CreateAccount &data, std::optional<WaitForServer> &waitForServer, float windowWidth, float windowHeight, ImFont &biggerFont)
{
  auto const shouldLockScreen = waitForServer.has_value ();
  ImGui::Dummy (ImVec2 (0.0f, (windowHeight - (5 * (ImGui::GetFontSize () + ImGui::GetStyle ().ItemSpacing.y * 2))) / 3));
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar;
  ImGui::PushStyleVar (ImGuiStyleVar_ChildRounding, 5.0f);
  ImGui::Dummy (ImVec2 ((windowWidth - ImGui::CalcTextSize ("Create your Account").x - (8 * ImGui::GetStyle ().ItemSpacing.x)) / 2, 0.0f));
  ImGui::SameLine ();
  ImGui::PushFont (&biggerFont);
  ImGui::Text ("Create your Account");
  ImGui::PopFont ();
  ImGui::Dummy (ImVec2 (windowWidth / 4, 0.0f));
  ImGui::SameLine ();
  ImGui::BeginChild ("ChildR", ImVec2 (windowWidth / 2, (5 * (ImGui::GetFontSize () + ImGui::GetStyle ().ItemSpacing.y * 2)) + 50), true, window_flags);
  ImGui::Dummy (ImVec2 (0.0f, 10.0f));
  ImGui::Dummy (ImVec2 (10.0f, 0.0f));
  ImGui::SameLine ();
  ImGui::PushStyleVar (ImGuiStyleVar_ChildBorderSize, 0);
  ImGui::BeginChild ("ChildR_sub", ImVec2 ((windowWidth / 2) - 50, (5 * (ImGui::GetFontSize () + ImGui::GetStyle ().ItemSpacing.y * 2)) + 10), true, window_flags);
  ImGui::PopStyleVar ();
  ImGui::PushItemWidth (-1.0f);
  ImGui::Text ("Username");
  ImGui::PushDisabled (shouldLockScreen);
  ImGui::InputText ("##account-username", &data.accountName);
  ImGui::PopDisabled (shouldLockScreen);
  ImGui::Text ("Password");
  ImGui::PushDisabled (shouldLockScreen);
  ImGui::InputText ("##account-password", &data.password, ImGuiInputTextFlags_Password);
  ImGui::PopDisabled (shouldLockScreen);
  if (shouldLockScreen)
    {
      auto &button = waitForServer->buttons.front ();
      using namespace std::chrono_literals;
      if (waitForServer->elapsedTime () >= 5s)
        {
          button.pressed = ImGui::Button (button.name.c_str (), ImVec2 (-1, 0));
        }
      else
        {
          ImGui::PushDisabled (shouldLockScreen);
          ImGui::Button (button.name.c_str (), ImVec2 (-1, 0));
          ImGui::PopDisabled (shouldLockScreen);
        }
    }
  else
    {
      data.backToLoginClicked = ImGui::Button ("Back");
    }
  ImGui::SameLine ();
  ImGui::PushDisabled (shouldLockScreen);
  data.createAccountClicked = ImGui::Button ("Create Account and Sign in");
  ImGui::PopDisabled (shouldLockScreen);
  ImGui::PopItemWidth ();
  ImGui::EndChild ();
  ImGui::EndChild ();
  ImGui::PopStyleVar ();
}

void
lobbyScreen (Lobby &data, std::optional<WaitForServer> &waitForServer, ChatData &chatData)
{
  auto const shouldLockScreen = waitForServer.has_value ();
  chatScreen (chatData, shouldLockScreen);
  ImGui::PushDisabled (shouldLockScreen);
  ImGui::Text ("Create Game Lobby");
  ImGui::Text ("Game Lobby Name");
  ImGui::InputText ("##CreateGameLobbyName", &data.createGameLobbyName);
  ImGui::Text ("Game Lobby Password");
  ImGui::InputText ("##CreateGameLobbyPassword", &data.createGameLobbyPassword);
  data.createCreateGameLobbyClicked = ImGui::Button ("Create Game Lobby", ImVec2 (-1, 0));
  ImGui::Text ("Join Game Lobby");
  ImGui::Text ("Game Lobby Name");
  ImGui::InputText ("##JoinGameLobbyName", &data.joinGameLobbyName);
  ImGui::Text ("Game Lobby Password");
  ImGui::InputText ("##JoinGameLobbyPassword", &data.joinGameLobbyPassword);
  data.createJoinGameLobbyClicked = ImGui::Button ("Join Game Lobby", ImVec2 (-1, 0));
  data.logoutButtonClicked = ImGui::Button ("Logout", ImVec2 (-1, 0));
  ImGui::PopDisabled (shouldLockScreen);
}
