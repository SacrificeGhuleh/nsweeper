#ifndef NSWEEPERGAME_HPP
#define NSWEEPERGAME_HPP

#include <algorithm>
#include <vector>

class NSweeperGame
{
public:
  explicit NSweeperGame(int width = 10, int height = 10, int numberOfMines = 10);

  enum class CellType : char
  {
    Mines_0 = '0',
    Mines_1 = '1',
    Mines_2 = '2',
    Mines_3 = '3',
    Mines_4 = '4',
    Mines_5 = '5',
    Mines_6 = '6',
    Mines_7 = '7',
    Mines_8 = '8',
    Mine = '*',
    Flag = '!',
    FlagMine = '#',
    Hidden = 'H',
    Invalid = (char)0xFF
  };

  int getBoardWidth() const;
  int getBoardHeight() const;
  int getTotalNumberOfMines() const;
  int getAvailableFlags() const;

  CellType getGameCell(int x, int y) const;
  bool onClick(int x, int y);
  bool interactFlag(int x, int y);
  bool isGameLost() const;
  bool isGameWon() const;
  bool isFlagCorrectlyPlaced(int x, int y) const;

private:
  const int boardWidth;
  const int boardHeight;
  const int totalNumberOfMines;
  int availableFlags;
  int foundMines;
  bool gameLost;

  std::vector<CellType> gameBoard;

  void updateCellType(int x, int y, bool firstClick);
  CellType getGameCellInternal(int x, int y) const;
};

#endif