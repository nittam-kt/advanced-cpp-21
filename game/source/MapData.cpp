#include "MapData.h"

#include <fstream>


namespace
{
    constexpr const char* map_[] = {
        "###########",
        "#.#.......#",
        "#.###.##.##",
        "#.....#...#",
        "#.###P###.#",
        "#..#......#",
        "##.###.##.#",
        "#.........#",
        "###########"
    };
}

char MapData::getData(int x, int y) const
{
	return map_[y][x];
}

size_t MapData::getHeight() const
{
	return sizeof(map_) / sizeof(map_[0]);
}

size_t MapData::getWidth() const
{
	return sizeof(map_[0]);
}

bool MapData::load(std::string_view filename)
{
    std::ifstream file(filename.data());
    if (!file) {
        return false; // ファイルが開けなかった
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (!line.empty())
        {
            // TODO:lineから情報を読み取って保存する
        }
    }
    return true;
}
