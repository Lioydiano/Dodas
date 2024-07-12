#include "dodas.hpp"
#include <thread>
#include <chrono>


int main() {
    sista::SwappableField field(50, 30);
    std::mt19937 rng(std::random_device{}());
}


std::unordered_map<Direction, sista::Coordinates> directionMap = {
    {Direction::UP, {-1, 0}},
    {Direction::RIGHT, {0, 1}},
    {Direction::DOWN, {1, 0}},
    {Direction::LEFT, {0, -1}}
};
std::unordered_map<Direction, char> directionSymbol = {
    {Direction::UP, '^'},
    {Direction::RIGHT, '>'},
    {Direction::DOWN, 'v'},
    {Direction::LEFT, '<'}
};

Entity::Entity(char symbol, sista::Coordinates coordinates, ANSI::Settings& settings, Type type) : sista::Pawn(symbol, coordinates, settings), type(type) {}
Entity::Entity() : sista::Pawn(' ', (sista::Coordinates){0, 0}, Wall::wallStyle, Type::PLAYER) {}

ANSI::Settings Bullet::bulletStyle = {
    ANSI::ForegroundColor::F_MAGENTA,
    ANSI::BackgroundColor::B_BLACK,
    ANSI::Attribute::BRIGHT
};
Bullet::Bullet(sista::Coordinates coordinates, Direction direction) : Entity(directionSymbol[direction], coordinates, bulletStyle, Type::BULLET), direction(direction), slowness(slowness) {}
Bullet::Bullet() : Entity(' ', {0, 0}, bulletStyle, Type::BULLET), direction(Direction::UP), slowness(1), countdown(0) {}

ANSI::Settings EnemyBullet::enemyBulletStyle = {
    ANSI::ForegroundColor::F_GREEN,
    ANSI::BackgroundColor::B_BLACK,
    ANSI::Attribute::BRIGHT
};
EnemyBullet::EnemyBullet(sista::Coordinates coordinates, Direction direction, unsigned short speed) : Entity(directionSymbol[direction], coordinates, enemyBulletStyle, Type::BULLET), direction(direction), slowness(slowness) {}
EnemyBullet::EnemyBullet() : Entity(' ', {0, 0}, enemyBulletStyle, Type::ENEMYBULLET), direction(Direction::UP), slowness(1), countdown(0) {}

ANSI::Settings Player::playerStyle = {
    ANSI::ForegroundColor::F_RED,
    ANSI::BackgroundColor::B_BLACK,
    ANSI::Attribute::BRIGHT
};
Player::Player(sista::Coordinates coordinates) : Entity('$', coordinates, playerStyle, Type::PLAYER) {}
Player::Player() : Entity('$', {0, 0}, playerStyle, Type::PLAYER) {}

ANSI::Settings Zombie::zombieStyle = {
    ANSI::ForegroundColor::F_BLACK,
    ANSI::BackgroundColor::B_GREEN,
    ANSI::Attribute::FAINT
};
Zombie::Zombie(sista::Coordinates coordinates) : Entity('Z', coordinates, zombieStyle, Type::ZOMBIE) {}
Zombie::Zombie() : Entity('Z', {0, 0}, zombieStyle, Type::ZOMBIE) {}

ANSI::Settings Mother::motherStyle = {
    ANSI::ForegroundColor::F_BLACK,
    ANSI::BackgroundColor::B_RED,
    ANSI::Attribute::BRIGHT
};
Mother::Mother(sista::Coordinates coordinates) : Entity('9', coordinates, motherStyle, Type::MOTHER), life(9) {}
Mother::Mother() : Entity('9', {0, 0}, motherStyle, Type::MOTHER), life(9) {}

ANSI::Settings Wall::wallStyle = {
    ANSI::ForegroundColor::F_YELLOW,
    ANSI::BackgroundColor::B_BLACK,
    ANSI::Attribute::BRIGHT
};
Wall::Wall(sista::Coordinates coordinates, short int strength) : Entity('=', coordinates, wallStyle, Type::WALL), strength(strength) {}
Wall::Wall() : Entity('#', {0, 0}, wallStyle, Type::WALL) {}