#include <sista/sista.hpp>
#include <unordered_map>
#include <vector>
#include <random>


#define CANNON_FIRE_PROBABILITY 0.025
#define CANNON_FIRE_PERIOD 40
#define WORKER_PRODUCTION_PROBABILITY 1.0/150
#define WORKER_PRODUCTION_PERIOD 150
#define ZOMBIE_MOVING_PROBABILITY 0.2
#define ZOMBIE_SHOOTING_PROBABILITY 0.01
#define WALKER_MOVING_PROBABILITY 0.1

#define START_AMMONITION 10

#define DEBUG 1
#define INTRO 1
#define SCAN_FOR_NULLPTRS 1
#define VERSION "1.0.0-alpha"
#define DATE "2025-08-25"

#define WIN_API_MUSIC_DELAY 80
#define REPOPULATE 127 // The number of frame before the whole sista::Field is emptied and repopulated

void printIntro();

enum Type {
    PLAYER,
    WORKER,
    ARMED_WORKER,
    CANNON,
    BOMBER,
    BULLET,
    MINE,

    WALL,
    ZOMBIE,
    WALKER,
    ENEMYBULLET,
    QUEEN
};


enum Direction {UP, RIGHT, DOWN, LEFT};
extern std::unordered_map<Direction, sista::Coordinates> directionMap;
extern std::unordered_map<Direction, char> directionSymbol;
extern std::mt19937 rng;


class Entity : public sista::Pawn {
public:
    Type type;

    Entity();
    Entity(char, sista::Coordinates, sista::ANSISettings&, Type);
};


class Bullet : public Entity {
public:
    static sista::ANSISettings bulletStyle;
    static std::vector<std::shared_ptr<Bullet>> bullets;
    Direction direction;
    unsigned short speed = 1; // The bullet moves speed cells per frame
    bool collided = false; // If the bullet was destroyed in a collision with an opposite bullet

    Bullet();
    Bullet(sista::Coordinates, Direction);
    Bullet(sista::Coordinates, Direction, unsigned short);

    void move();

    static void removeBullet(std::shared_ptr<Bullet>);
    static void removeBullet(Bullet*); // Overload for raw pointer
};


class EnemyBullet : public Entity {
public:
    static sista::ANSISettings enemyBulletStyle;
    static std::vector<std::shared_ptr<EnemyBullet>> enemyBullets;
    Direction direction;
    unsigned short speed = 1; // The bullet moves speed cells per frame
    bool collided = false; // If the bullet was destroyed in a collision with an opposite bullet

    EnemyBullet();
    EnemyBullet(sista::Coordinates, Direction);
    EnemyBullet(sista::Coordinates, Direction, unsigned short);

    void move();

    static void removeEnemyBullet(std::shared_ptr<EnemyBullet>);
    static void removeEnemyBullet(EnemyBullet*); // Overload for raw pointer
};


class Player : public Entity {
public:
    static sista::ANSISettings playerStyle;
    static std::shared_ptr<Player> player;
    Type weapon = Type::BULLET; // The player can have different weapons (bullets, mines, etc.)
    int ammonitions; // The player has a certain amount of ammonition (when it reaches 0, the player can't shoot anymore)
    unsigned short speed = 1; // The player shoots bullets at speed cells per frame

    Player();
    Player(sista::Coordinates);

    void move(Direction);
    void shoot(Direction);
};


class Zombie : public Entity {
public:
    static sista::ANSISettings zombieStyle;
    static std::vector<std::shared_ptr<Zombie>> zombies;
    static std::bernoulli_distribution distribution; // The zombie moves a cell every zombieSpeed frames, on average
    static std::bernoulli_distribution shootDistribution; // The zombie shoots a bullet every zombieShootingRate frames, on average

    Zombie();
    Zombie(sista::Coordinates);

    void move(); // They should move mostly vertically
    // void move(Direction);
    void shoot(); // They should only shoot horizontally to the left
    // void shoot(Direction);

    static void removeZombie(std::shared_ptr<Zombie>);
    static void removeZombie(Zombie*); // Overload for raw pointer
};


class Queen : public Entity {
public:
    static sista::ANSISettings queenStyle;
    static std::shared_ptr<Queen> queen;
    int life; // The mother has a life score (when it reaches 0, the game is over) which regenerates over time

    Queen();
    Queen(sista::Coordinates);

    void move(); // Only moves vertically in a small range, I want it to always be near the center
    void createWall(); // Creates a 1x[3-5] wall of strenght 1 in front of the queen
};


class Wall : public Entity {
public:
    static sista::ANSISettings wallStyle;
    static std::vector<std::shared_ptr<Wall>> walls;
    short int strength; // The wall has a certain strength (when it reaches 0, the wall is destroyed)

    Wall();
    Wall(sista::Coordinates, short int);

    static void removeWall(std::shared_ptr<Wall>);
};


class Mine : public Entity {
public:
    static sista::ANSISettings mineStyle;
    static std::vector<std::shared_ptr<Mine>> mines;
    bool triggered = false;
    bool alive = true;

    Mine();
    Mine(sista::Coordinates);

    bool checkTrigger();
    void trigger();
    void explode();

    static void removeMine(std::shared_ptr<Mine>);
};


class Cannon : public Entity { // Cannons shoot bullets only against the zombies, they have a certain firing rate
public:
    static sista::ANSISettings cannonStyle;
    static std::vector<std::shared_ptr<Cannon>> cannons;
    static std::bernoulli_distribution distribution; // The cannon shoots a bullet every firingRate frames, on average

    Cannon();
    Cannon(sista::Coordinates, unsigned short);

    void fire();
    void recomputeDistribution(std::vector<std::vector<unsigned short>>&);

    static void removeCannon(std::shared_ptr<Cannon>);
    static void removeCannon(Cannon*); // Overload for raw pointer
};


class Worker : public Entity { // Workers produce ammonition for the player, they have a certain production rate
public:
    static sista::ANSISettings workerStyle;
    static std::vector<std::shared_ptr<Worker>> workers;
    std::bernoulli_distribution distribution; // The worker produces an ammonition every productionRate frames, on average

    Worker();
    Worker(sista::Coordinates);
    Worker(sista::Coordinates, unsigned short);

    void produce();

    static void removeWorker(std::shared_ptr<Worker>);
    static void removeWorker(Worker*);
};


class Bomber : public Entity { // Bombers go towards the enemies and explode when they meet a wall
public:
    static sista::ANSISettings bomberStyle;
    static std::vector<std::shared_ptr<Bomber>> bombers;

    Bomber();
    Bomber(sista::Coordinates);

    void move();
    void explode();

    static void removeBomber(std::shared_ptr<Bomber>);
    static void removeBomber(Bomber*); // Overload for raw pointer
};

class Walker : public Entity { // Walkers go towards the left side of the screen and can kill the player on touch, and they explode as bombers when they meet a worker
public:
    static sista::ANSISettings walkerStyle;
    static std::vector<std::shared_ptr<Walker>> walkers;
    static std::bernoulli_distribution distribution; // The walker moves a cell every walkerSpeed frames, on average

    Walker();
    Walker(sista::Coordinates);

    void move();
    void explode();

    static void removeWalker(std::shared_ptr<Walker>);
    static void removeWalker(Walker*); // Overload for raw pointer
};

class ArmedWorker : public Entity { // Workers produce ammonition for the player, they have a certain production rate
public:
    static sista::ANSISettings armedWorkerStyle;
    static std::vector<std::shared_ptr<ArmedWorker>> armedWorkers;
    std::bernoulli_distribution distribution; // The worker produces an ammonition every productionRate frames, on average

    ArmedWorker();
    ArmedWorker(sista::Coordinates);
    ArmedWorker(sista::Coordinates, unsigned short);

    void produce();
    void dodgeIfNeeded();

    static void removeArmedWorker(std::shared_ptr<ArmedWorker>);
    static void removeArmedWorker(ArmedWorker*); // Overload for raw pointer
};

void removeNullptrs(std::vector<std::shared_ptr<Entity>>&);
