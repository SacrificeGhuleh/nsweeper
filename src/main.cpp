#include <vector>
#include <cstdlib>
#include <cstdint>
#include <memory>
#include <string>
#include <iostream>
#include <sstream>

#include "imtui/imtui.h"
#include "imtui/imtui-impl-ncurses.h"

#include "nsweepergame.hpp"

static void handleGame(std::unique_ptr<NSweeperGame> &activeGame);
static void showMainMenuBar();
static void showDebugWindow();

static bool windowShallClose = false;
std::string err = "";
std::vector<std::string> debugTexts;

struct GameSettings
{
  int width = 10;
  int height = 10;
  int numberOfMines = 10;
};

GameSettings gameSettings;

int main()
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::GetIO().IniFilename = NULL;

  auto screen = ImTui_ImplNcurses_Init(true);
  ImTui_ImplText_Init();

  std::unique_ptr<NSweeperGame> activeGame = std::make_unique<NSweeperGame>(gameSettings.width, gameSettings.height, gameSettings.numberOfMines);

  try
  {
    while (!windowShallClose)
    {
      ImTui_ImplNcurses_NewFrame();
      ImTui_ImplText_NewFrame();

      ImGui::NewFrame();

      showMainMenuBar();

      // ImGui::SetNextWindowPos(ImVec2(0, 1), ImGuiCond_Once);
      ImGui::SetNextWindowSize(ImVec2(20, 10), ImGuiCond_Once);
      ImVec2 center = ImGui::GetMainViewport()->GetCenter();
      ImGui::SetNextWindowPos(center, ImGuiCond_Once, ImVec2(0.5f, 0.5f));
      handleGame(activeGame);

      // ImGui::SetNextWindowPos(ImVec2(35, 1), ImGuiCond_Once);
      // ImGui::SetNextWindowSize(ImVec2(70, 20), ImGuiCond_Once);
      // showDebugWindow();

      ImGui::Render();

      ImTui_ImplText_RenderDrawData(ImGui::GetDrawData(), screen);
      ImTui_ImplNcurses_DrawScreen();
    }
  }
  catch (const std::exception &e)
  {
    err = e.what();
  }

  ImTui_ImplText_Shutdown();
  ImTui_ImplNcurses_Shutdown();
  if (!err.empty())
  {
    std::cerr << err << '\n';
  }
  return 0;
}

static void showDebugWindow()
{
  ImGui::Begin("debug");

  ImGui::Text("Mouse Pos : x = %g, y = %g", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
  ImGui::Text("Time per frame %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  ImGui::Text("Float:");

  ImGui::Text("%s", "");

  for (const auto &dbgText : debugTexts)
  {
    ImGui::Text("%s", dbgText.c_str());
  }
  debugTexts.clear();

  if (ImGui::Button("Exit program", {ImGui::GetContentRegionAvail().x, 2}))
  {
    windowShallClose = true;
  }

  ImGui::End();
}

static void showMainMenuBar()
{
  if (ImGui::BeginMainMenuBar())
  {
    if (ImGui::BeginMenu("File"))
    {
      if (ImGui::MenuItem("New game", "CTRL+N"))
      {
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Quit", "CTRL+C"))
      {
        windowShallClose = true;
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
}

static void renderAlignedSquare(ImDrawList *drawList, const ImVec2 &pos, ImU32 col, bool even = true)
{
  ImVec2 pos1(((int)pos.x / 2) * 2, pos.y);
  ImVec2 pos2(pos1.x + 1, pos.y);

  if (even)
  {
    pos1.x += 1;
    pos2.x += 1;
  }

  drawList->AddLine(pos1, pos1, col);
  drawList->AddLine(pos2, pos2, col);
}

static void renderHighlightedStr(ImDrawList *drawList, const ImVec2 &pos, const char *str, bool even = true, ImU32 col = IM_COL32(0xAA, 0xAA, 0xAA, 0xFF))
{
  ImVec2 alignedPos(((int)pos.x / 2) * 2, pos.y);
  if (even)
  {
    alignedPos.x += 1;
  }
  renderAlignedSquare(drawList, alignedPos, col, even);
  drawList->AddText(alignedPos, IM_COL32(0xFF, 0xFF, 0xFF, 0xFF), str);
}

static void handleGame(std::unique_ptr<NSweeperGame> &activeGame)
{
  ImGui::Begin("Minesweeper", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);

  auto *drawList = ImGui::GetWindowDrawList();
  auto screenPos = ImGui::GetWindowPos();
  auto screenSize = ImVec2((activeGame->getBoardWidth() * 2), activeGame->getBoardHeight());
  auto cursorGlobalPos = ImGui::GetIO().MousePos;
  auto cursorLocalPos = ImVec2(cursorGlobalPos.x - screenPos.x, cursorGlobalPos.y - screenPos.y - 1);
  auto tilesIndex = ImVec2((int)((cursorLocalPos.x) / 2), (int)(cursorLocalPos.y));
  bool hovering = cursorLocalPos.x >= 0 && cursorLocalPos.x < screenSize.x && cursorLocalPos.y >= 0 && cursorLocalPos.y < screenSize.y;
  bool odd = (int)screenPos.x & 1;
  ImGui::InvisibleButton("canvas", screenSize);

  ImU32 hiddenCol = IM_COL32(0x55, 0x55, 0x55, 0xFF);
  ImU32 discoveredCol = IM_COL32(0x11, 0x11, 0x11, 0xFF);
  ImU32 mineCol = IM_COL32(0xFF, 0x10, 0x10, 0xFF);
  ImU32 flagCol = IM_COL32(0x00, 0x55, 0x55, 0xFF);
  ImU32 hoverCol = IM_COL32(0xCC, 0xCC, 0xCC, 0xFF);

  if (hovering && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
  {
    activeGame->onClick(tilesIndex.x, tilesIndex.y);
  }

  if (hovering && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
  {
    activeGame->interactFlag(tilesIndex.x, tilesIndex.y);
  }

  char buffer[2] = {0, 0};
  for (int y = 0; y < activeGame->getBoardHeight(); y++)
  {
    for (int x = 0; x < activeGame->getBoardWidth(); x++)
    {
      auto cell = activeGame->getGameCell(x, y);

      if (cell == NSweeperGame::CellType::Mines_0 || cell == NSweeperGame::CellType::Hidden || cell == NSweeperGame::CellType::Invalid)
      {
        buffer[0] = ' ';
      }
      else
      {
        buffer[0] = (char)cell;
      }

      ImU32 col;
      if (hovering && (x == tilesIndex.x && y == tilesIndex.y))
      {
        col = hoverCol;
      }
      else
      {
        switch (cell)
        {
        case NSweeperGame::CellType::Flag:
        {
          if (activeGame->isFlagCorrectlyPlaced(x, y))
          {
            col = flagCol;
          }
          else
          {
            col = hiddenCol;
          }
          break;
        }
        case NSweeperGame::CellType::Hidden:
        {
          col = hiddenCol;
          break;
        }
        case NSweeperGame::CellType::Mine:
        {
          col = mineCol;
          break;
        }
        default:
        {
          col = discoveredCol;
        }
        }
      }

      ImVec2 actualCoords = ImVec2(screenPos.x + (2 * x), screenPos.y + y + 1);
      renderHighlightedStr(drawList, actualCoords, buffer, odd, col);
    }
  }
  if (ImGui::CollapsingHeader("Settings"))
  {
    ImGui::DragScalar("Width", ImGuiDataType_U8, &gameSettings.width, 0.2, NULL, NULL, "%u");
    ImGui::DragScalar("Height", ImGuiDataType_U8, &gameSettings.height, 0.2, NULL, NULL, "%u");
    ImGui::DragScalar("Mines", ImGuiDataType_U8, &gameSettings.numberOfMines, 0.2, NULL, NULL, "%u");
  }
  ImGui::Text("Remaining mines: %d", activeGame->getAvailableFlags());
  if (activeGame->isGameLost())
  {
    ImGui::Text("Kaboom. U wot m8?");
  }
  if (activeGame->isGameWon())
  {
    ImGui::Text("U WON!!! CHEATER!!!");
  }

  if (ImGui::Button("Reset", {screenSize.x, 1}))
  {
    activeGame = std::make_unique<NSweeperGame>(gameSettings.width, gameSettings.height, gameSettings.numberOfMines);
  }
  if (ImGui::Button("I'm scared, quit", {screenSize.x, 1}))
  {
    windowShallClose = true;
  }
  ImGui::End();
}
