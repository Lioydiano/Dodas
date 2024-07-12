#include "include/sista/sista.hpp"
#include <unordered_map>
#include <vector>
#include <random>


enum Type {
    PLAYER,
    BULLET,
    MINE,
    WALL,
    ZOMBIE,
    ENEMYBULLET,
    MOTHER
};


enum Direction {UP, RIGHT, DOWN, LEFT};
extern std::unordered_map<Direction, sista::Coordinates> directionMap;
extern std::unordered_map<Direction, char> directionSymbol;


class Entity : public sista::Pawn {
public:
    Type type;

    Entity();
    Entity(char, sista::Coordinates, ANSI::Settings&, Type);
};


class Bullet : public Entity {
public:
    static ANSI::Settings bulletStyle;
    static std::vector<Bullet*> bullets;
    Direction direction;
    unsigned short slowness; // The bullet moves every slowness frames
    short int countdown; // Countdown for next move

    Bullet();
    Bullet(sista::Coordinates, Direction);

    void move();
};


class EnemyBullet : public Entity {
public:
    static ANSI::Settings enemyBulletStyle;
    static std::vector<EnemyBullet*> enemyBullets;
    Direction direction;
    unsigned short slowness; // The bullet moves every slowness frames
    short int countdown; // Countdown for next move

    EnemyBullet();
    EnemyBullet(sista::Coordinates, Direction, unsigned short);

    void move();
};


class Player : public Entity {
public:
    static ANSI::Settings playerStyle;
    static Player* player;

    Player();
    Player(sista::Coordinates);

    void move();
    void shoot();
};


class Zombie : public Entity {
public:
    static ANSI::Settings zombieStyle;
    static std::vector<Zombie*> zombies;

    Zombie();
    Zombie(sista::Coordinates);

    void move(); // They should move mostly vertically
    void move(Direction);
    void shoot(); // They should only shoot horizontally to the left
    void shoot(Direction);
};


class Mother : public Entity {
public:
    static ANSI::Settings motherStyle;
    static Mother* mother;
    int life; // The mother has a life score (when it reaches 0, the game is over) which regenerates over time

    Mother();
    Mother(sista::Coordinates);

    void move(); // Only moves vertically in a small range, I want it to always be in the center
    void shoot(); // Only shoots horizontally to the left
    void spawnZombie(); // Spawns a zombie at a random position (but I would like the zombies to be born close to the mother)
};


class Wall : public Entity {
public:
    static ANSI::Settings wallStyle;
    static std::vector<Wall*> walls;
    short int strength; // The wall has a certain strength (when it reaches 0, the wall is destroyed)

    Wall();
    Wall(sista::Coordinates, short int);
};