#include "nsweepergame.hpp"

#include <algorithm>
#include <random>

NSweeperGame::NSweeperGame(int width, int height, int numberOfMines) : boardWidth(std::max(width, 10)),
                                                                       boardHeight(std::max(height, 10)),
                                                                       totalNumberOfMines(std::min(std::max(numberOfMines, 10), (width * height) - 1)),
                                                                       gameLost(false),
                                                                       foundMines(0)
{
  auto rng = std::default_random_engine(time(0));
  availableFlags = totalNumberOfMines;
  gameBoard.resize(boardWidth * boardHeight);
  std::vector<int> indices;
  indices.resize(boardWidth * boardHeight);
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      int idx = boardWidth * y + x;
      gameBoard.at(idx) = CellType::Hidden;
      indices.at(idx) = idx;
    }
  }
  std::shuffle(std::begin(indices), std::end(indices), rng);

  for (int i = 0; i < totalNumberOfMines; i++)
  {
    gameBoard.at(indices.at(i)) = CellType::Mine;
  }
}

NSweeperGame::CellType NSweeperGame::getGameCellInternal(int x, int y) const
{
  if (x < 0 || y < 0 || x >= boardWidth || y >= boardHeight)
  {
    return CellType::Invalid;
  }

  int idx = boardWidth * y + x;
  return gameBoard.at(idx);
}

NSweeperGame::CellType NSweeperGame::getGameCell(int x, int y) const
{
  auto cell = getGameCellInternal(x, y);

  if (cell == CellType::Invalid || (!gameLost && cell == CellType::Mine))
  {
    cell = CellType::Hidden;
  }

  if (cell == CellType::FlagMine)
  {
    cell = CellType::Flag;
  }

  return cell;
}

bool NSweeperGame::onClick(int x, int y)
{
  if (x < 0 || y < 0 || x >= boardWidth || y >= boardHeight || gameLost)
  {
    return false;
  }

  int idx = boardWidth * y + x;

  if (gameBoard.at(idx) == CellType::FlagMine || gameBoard.at(idx) == CellType::Flag)
  {
    interactFlag(x, y);
  }

  if (gameBoard.at(idx) == CellType::Mine || gameBoard.at(idx) == CellType::FlagMine)
  {
    gameLost = true;
    return false;
  }

  if (gameBoard.at(idx) == CellType::Hidden)
  {
    updateCellType(x, y, true);
  }

  return true;
}

bool NSweeperGame::interactFlag(int x, int y)
{
  if (x < 0 || y < 0 || x >= boardWidth || y >= boardHeight || availableFlags <= 0)
  {
    return false;
  }
  bool interacted = false;
  auto currentCell = getGameCellInternal(x, y);
  int idx = boardWidth * y + x;

  if (availableFlags > 0)
  {
    switch (currentCell)
    {
    case CellType::Hidden:
    {
      gameBoard.at(idx) = CellType::Flag;
      interacted = true;
      availableFlags--;
      break;
    }

    case CellType::Mine:
    {
      gameBoard.at(idx) = CellType::FlagMine;
      interacted = true;
      availableFlags--;
      foundMines++;
      break;
    }

    default:
    {
      break;
    }
    }
  }

  switch (currentCell)
  {
  // removing flag
  case CellType::Flag:
  {
    availableFlags++;
    interacted = true;
    gameBoard.at(idx) = CellType::Hidden;
    break;
  }
  case CellType::FlagMine:
  {
    gameBoard.at(idx) = CellType::Mine;
    interacted = true;
    availableFlags++;
    foundMines--;
    break;
  }
  default:
  {
    break;
  }
  }

  return interacted;
}

void NSweeperGame::updateCellType(int x, int y, bool firstClick)
{
  int numberOfMines = 0;
  int idx = boardWidth * y + x;

  for (int xOffset = -1; xOffset <= 1; xOffset++)
  {
    for (int yOffset = -1; yOffset <= 1; yOffset++)
    {
      if (getGameCellInternal(x + xOffset, y + yOffset) == CellType::Mine)
      {
        numberOfMines++;
      }
    }
  }

  if (firstClick || numberOfMines <= 1)
  {
    gameBoard.at(idx) = (CellType)('0' + numberOfMines);
    if (numberOfMines == 0)
    {
      if (getGameCellInternal(x - 1, y) == CellType::Hidden)
      {
        updateCellType(x - 1, y, false);
      }
      if (getGameCellInternal(x + 1, y) == CellType::Hidden)
      {
        updateCellType(x + 1, y, false);
      }
      if (getGameCellInternal(x, y - 1) == CellType::Hidden)
      {
        updateCellType(x, y - 1, false);
      }
      if (getGameCellInternal(x, y + 1) == CellType::Hidden)
      {
        updateCellType(x, y + 1, false);
      }
    }
  }
}

int NSweeperGame::getBoardWidth() const { return boardWidth; }
int NSweeperGame::getBoardHeight() const { return boardHeight; }
int NSweeperGame::getTotalNumberOfMines() const { return totalNumberOfMines; }
int NSweeperGame::getAvailableFlags() const { return availableFlags; }
bool NSweeperGame::isGameLost() const { return gameLost; }
bool NSweeperGame::isGameWon() const { return foundMines == totalNumberOfMines; }
bool NSweeperGame::isFlagCorrectlyPlaced(int x, int y) const
{
  if (!isGameLost() && !isGameWon())
  {
    return false;
  }
  return getGameCellInternal(x, y) == CellType::FlagMine;
}