#include <iostream>
#include <vector>
using namespace std;

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define width 30
#define height 16
#define numMines 99

class Minesweeper : public olc::PixelGameEngine
{
public:
	Minesweeper()
	{
		sAppName = "Minesweeper";
	}

public:
	struct Cell
	{
		int x;
		int y;
		bool mine;
		bool revealed;
		int minesCount;
		int flag = 0; // 0: " ", 1: "#", 2: "?"

		Cell() // Default Constructor
		{

		}

		Cell(int _x, int _y, bool _mine)
		{
			x = _x;
			y = _y;
			mine = _mine;
			revealed = false;
			minesCount = 0;
		}

		void CountMines(vector<vector<Cell>>& grid)
		{
			if (this->mine)
			{
				this->minesCount = -1;
				return;
			}

			int result = 0;
			for (int _x = -1; _x <= 1; _x++)
			{
				for (int _y = -1; _y <= 1; _y++)
				{
					// Make sure we only count existing neighbors
					if (this->x + _x > -1 && this->x + _x < width && this->y + _y > -1 && this->y + _y < height)
					{
						Cell* neighbor = &grid[this->x + _x][this->y + _y];
						if (neighbor->mine)
						{
							result++;
						}
					}
				}
			}

			this->minesCount = result;
		}

		void Reveal(vector<vector<Cell>>& grid)
		{
			this->revealed = true;
			if (this->minesCount == 0)
			{
				this->RevealAll(grid);
			}
		}

		void RevealAll(vector<vector<Cell>>& grid)
		{
			for (int _x = -1; _x <= 1; _x++)
			{
				for (int _y = -1; _y <= 1; _y++)
				{
					// Make sure we only count existing neighbors
					if (this->x + _x > -1 && this->x + _x < width && this->y + _y > -1 && this->y + _y < height)
					{
						Cell* neighbour = &grid[this->x + _x][this->y + _y];
						if (!neighbour->mine && !neighbour->revealed)
						{
							neighbour->Reveal(grid);
						}
					}
				}
			}
		}
	};

	// Restart the map
	void InitGrid(vector<vector<Cell>>& grid)
	{
		firstClick = true;
		// Probability of a mine spawning
		float p = (float)numMines / (float)(width * height);
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				grid[x][y] = Cell(x, y, ((float)rand() / (float)RAND_MAX) < p);
			}
		}
	}

	// Check if the game is won
	bool IsWon(vector<vector<Cell>>& grid)
	{
		// The game is won is the only unrevealed cells are the mines
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				Cell* curr = &grid[x][y];
				if (!curr->revealed && !curr->mine)
				{
					return false;
				}
			}
		}

		return true;
	}

	// Relating to cells
	int cellSize = 14;
	int borderSize = 1;

	bool firstClick = true;
	bool gameover = false;

	int numPossibleMines = numMines;

	vector<vector<Cell>> grid;

public:
	bool OnUserCreate() override
	{
		srand(time(NULL));
		grid.resize(width, vector<Cell>(height));

		InitGrid(grid);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Get cursor position
		int selectedCellX = GetMouseX() / cellSize;
		int selectedCellY = GetMouseY() / cellSize;

		if (GetMouse(0).bReleased) // Reveal a cell
		{
			// Only generate grid upon the first click, which should never be a mine
			if (selectedCellX >= 0 && selectedCellX < width && selectedCellY >= 0 && selectedCellY < height)
			{
				Cell* clickedCell = &grid[selectedCellX][selectedCellY];

				// First click should never be on a mine
				if (firstClick)
				{
					clickedCell->mine = false;
					firstClick = false;
				}

				if (clickedCell->mine)
				{
					// Uh-oh...
					gameover = true;
				}
				else if (!gameover)
				{
					clickedCell->Reveal(grid);
				}
			}
		}

		// Flag a cell for possible mine
		if (GetMouse(1).bReleased)
		{
			// Check if click is in boundary
			if (selectedCellX >= 0 && selectedCellX < width && selectedCellY >= 0 && selectedCellY < height)
			{
				Cell* clickedCell = &grid[selectedCellX][selectedCellY];
				clickedCell->flag = (clickedCell->flag + 1) % 3;

				// Change the number of possible mines left
				if (clickedCell->flag == 1)
				{
					numPossibleMines--;
				}
				else if (clickedCell->flag == 2)
				{
					numPossibleMines++;
				}
			}
		}

		// Restart the game
		if (GetKey(olc::Key::R).bReleased)
		{
			gameover = false;
			InitGrid(grid);
		}

		// Relating to the scoreboard
		/*int scoreBoardW = 15;
		int scoreBoardH = 4;*/

		// Clear Screen
		Clear(olc::DARK_GREY);

		// Draw Grid
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				// Loop through neighboring cells
				Cell* curr = &grid[x][y];
				curr->CountMines(grid);

				// Colors to represent number of mines around a cell
				olc::Pixel col;
				switch (curr->minesCount)
				{
				case 1:
					col = olc::BLUE;
					break;
				case 2:
					col = olc::DARK_GREEN;
					break;
				case 3:
					col = olc::RED;
					break;
				case 4:
					col = olc::VERY_DARK_BLUE;
					break;
				case 5:
					col = olc::DARK_RED;
					break;
				case 6:
					col = olc::DARK_CYAN;
					break;
				case 7:
					col = olc::BLACK;
					break;
				case 8:
					col = olc::GREY;
					break;
				}

				// Store offsets for display
				int cellS = cellSize - borderSize;

				int cellOffsetX = x * cellSize + borderSize;
				int cellOffsetY = y * cellSize + borderSize;
				
				float strOffsetX = (x + 0.25) * cellSize + borderSize;
				float strOffsetY = (y + 0.25) * cellSize + borderSize / 2;

				bool win = IsWon(grid);

				if (curr->revealed)
				{
					FillRect(cellOffsetX, cellOffsetY, cellS, cellS, olc::WHITE);

					if (curr->minesCount != 0)
					{
						DrawString(strOffsetX, strOffsetY, to_string(curr->minesCount), col);
					}
				}
				else
				{
					FillRect(cellOffsetX, cellOffsetY, cellS, cellS, olc::GREY);
					// Once they are revealed, the flags are redundant
					switch (curr->flag)
					{
					case 1:
						// Flagged as mine
						DrawString(strOffsetX, strOffsetY, "#", olc::BLACK);
						break;
					case 2:
						// Drafting
						DrawString(strOffsetX, strOffsetY, "?", olc::BLACK);
						break;
					}
				}

				// Display number of (possible) mines left 
				DrawString({ 5, 230 }, "Mines left: " + to_string(numPossibleMines), olc::WHITE);

				if (gameover && !win)
				{
					numPossibleMines = numMines;
					DrawString({ 5, 250 }, "Oh noes! :(", olc::WHITE);
					DrawString({ 5, 270 }, "Restart? (Press R)", olc::WHITE);

					if (curr->mine)
					{
						FillRect(cellOffsetX, cellOffsetY, cellS, cellS, olc::RED);
						DrawString(strOffsetX, strOffsetY, "X", olc::BLACK);
					}			
				}

				if (win)
				{
					if (curr->mine)
					{
						FillRect(cellOffsetX, cellOffsetY, cellS, cellS, olc::GREEN);
						DrawString(strOffsetX, strOffsetY, "", olc::BLACK);
					}

					DrawString({ 5, 250 }, "Congratulations, you won!", olc::WHITE);
					DrawString({ 5, 270 }, "Restart? (Press R)", olc::WHITE);
				}
			}
		}

		return true;
	}
};

int main()
{
	Minesweeper game;
	if (game.Construct(512, 480, 2, 2))
		game.Start();
	return 0;
}