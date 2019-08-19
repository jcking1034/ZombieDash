#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
class StudentWorld;

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

/* GraphObject Contains:
 GraphObject(int imageID, double startX, double startY, int startDirection, int depth)
 getX()
 getY()
 moveTo(doublex, double y)
 getDirection()
 setDirection(Direction d);
*/

class Actor: public GraphObject {
public:
    Actor(int imageID, int level_x, int level_y, StudentWorld* thing, int startDirection = 0, int depth = 0, bool is_alive = 1, bool block_movement = 0, bool damaged_by_flame = 0, bool damaged_by_vomit = 0, bool block_flames = 0, int multiplier_x = SPRITE_WIDTH, int multiplier_y = SPRITE_HEIGHT);
    virtual void doSomething() = 0;
    
    bool collision(int other_x, int other_y);
    bool overlap(int other_x, int other_y);
    
    StudentWorld* getWorld() { return my_world_ptr; }
    
    void add_tick() { tick_count++; }
    bool is_two_tick() { return tick_count == 2; }
    bool is_even_tick() { return tick_count % 2 == 0; }
    bool is_thirty_tick() { return tick_count == 30; }
    
    bool is_alive() { return alive; }
    void set_alive(bool life) { alive = life; }
    
    virtual void kill() {};
    virtual void infect() {};
    
    bool can_block_movement() { return m_block_movement; }
    bool damaged_by_flame() { return m_damaged_by_flame; }
    bool damaged_by_vomit() { return m_damaged_by_vomit; }
    bool block_flames() { return m_block_flames; }
private:
    StudentWorld* my_world_ptr;
    int tick_count;
    bool alive;
    
    bool m_block_movement;    //  wall, citizen, zombie, Penelope
    bool m_damaged_by_flame;
    bool m_damaged_by_vomit;
    bool m_block_flames;
};

// PERSON
class Person: public Actor {
public:
    Person(int imageID, int level_x, int level_y, StudentWorld * thing, bool damaged_by_vomit, int multiplier_x = SPRITE_WIDTH, int multiplier_y = SPRITE_HEIGHT);
    void doSomething();
    virtual void do_specific() = 0;
private:
};

class Human: public Person {
public:
    Human(int imageID, int level_x, int level_y, StudentWorld * thing);
    
    virtual void infect() { infection_status = true; }
    bool infected() { return infection_status; }
    void clear_infect() {
        infection_status = false;
        infection_count = 0;
    }
    int get_infection_count() { return infection_count; }
    
    virtual void do_specific();
    virtual void human_died_specific() = 0;
    virtual void human_act_specific() = 0;
private:
    bool infection_status;
    int infection_count;
};

class Penelope: public Human {
public:
    Penelope(int level_x, int level_y, StudentWorld * thing);
    virtual void human_died_specific();
    virtual void human_act_specific();
    
    virtual void kill();
    int get_landmines() { return landmines; }
    int get_flamethrower() { return flamethrower; }
    int get_vaccines() { return vaccines; }
    
    void add_landmines(int x) { landmines += x; }
    void add_flamethrower(int x) { flamethrower += x; }
    void add_vaccines(int x) { vaccines += x; }
private:
    int landmines;
    int flamethrower;
    int vaccines;
};

class Citizen: public Human {
public:
    Citizen(int level_x, int level_y, StudentWorld * thing);
    virtual void human_died_specific();
    virtual void human_act_specific();
    virtual void kill();
    virtual void infect();
private:
    
};

class Zombie: public Person {
public:
    Zombie(int level_x, int level_y, StudentWorld * thing, int multiplier_x = SPRITE_WIDTH, int multiplier_y = SPRITE_HEIGHT);
    virtual void do_specific();
    virtual void zombie_act_specific() = 0;
    
    bool try_new_vomit(int x, int y);
    
    int get_movement_plan_distance() { return movement_plan_distance; }
    void set_movement_plan_distance(int x) { movement_plan_distance = x; }
private:
    int movement_plan_distance;
};

class Dumb_Zombie: public Zombie {
public:
    Dumb_Zombie(int level_x, int level_y, StudentWorld * thing, int multiplier_x = SPRITE_WIDTH, int multiplier_y = SPRITE_HEIGHT);
    virtual void zombie_act_specific();
    virtual void kill();
private:
};

class Smart_Zombie: public Zombie {
public:
    Smart_Zombie(int level_x, int level_y, StudentWorld * thing, int multiplier_x = SPRITE_WIDTH, int multiplier_y = SPRITE_HEIGHT);
    virtual void zombie_act_specific();
    virtual void kill();
private:
};

// WALL
class Wall: public Actor {
public:
    Wall(int level_x, int level_y, StudentWorld * thing);
    virtual void doSomething() {};
private:
    
};

// Activating_Object
class Activating_Object: public Actor {
public:
    Activating_Object(int imageID, int level_x, int level_y, StudentWorld * thing, int direction, int depth, bool damaged_by_flame, bool block_flame, int multiplier_x = SPRITE_WIDTH, int multiplier_y = SPRITE_HEIGHT);
    void doSomething();
    virtual void specific_do_something() = 0;
private:
    int tick_count;
};

class Exit: public Activating_Object {
public:
    Exit(int level_x, int level_y, StudentWorld * thing);
    virtual void specific_do_something();
private:
};

class Pit: public Activating_Object {
public:
    Pit(int level_x, int level_y, StudentWorld * thing, int multiplier_x = SPRITE_WIDTH, int multiplier_y = SPRITE_HEIGHT);
    virtual void specific_do_something();
private:
};

class Flame: public Activating_Object {
public:
    Flame(int level_x, int level_y, StudentWorld * thing, int direction, int multiplier_x = SPRITE_WIDTH, int multiplier_y = SPRITE_HEIGHT);
    virtual void specific_do_something();
private:
};

class Vomit: public Activating_Object {
public:
    Vomit(int level_x, int level_y, StudentWorld * thing, int direction, int multiplier_x = SPRITE_WIDTH, int multiplier_y = SPRITE_HEIGHT);
    virtual void specific_do_something();
private:
    
};

class Landmine: public Activating_Object {
public:
    Landmine(int level_x, int level_y, StudentWorld * thing, int multiplier_x = SPRITE_WIDTH, int multiplier_y = SPRITE_HEIGHT);
    virtual void specific_do_something();
    virtual void kill();
private:
    bool active;
};

// GOODIE
class Goodie: public Actor {
public:
    Goodie(int imageID, int level_x, int level_y, StudentWorld * thing, int multiplier_x = SPRITE_WIDTH, int multiplier_y = SPRITE_HEIGHT);
    void doSomething();
    virtual void grant_specific_goodie_reward() = 0;
private:
};

class Vaccine_Goodie: public Goodie {
public:
    Vaccine_Goodie(int level_x, int level_y, StudentWorld * thing, int multiplier_x = SPRITE_WIDTH, int multiplier_y = SPRITE_HEIGHT);
    virtual void grant_specific_goodie_reward();
private:
};

class Gas_Can_Goodie: public Goodie {
public:
    Gas_Can_Goodie(int level_x, int level_y, StudentWorld * thing);
    virtual void grant_specific_goodie_reward();
private:
    
};

class Landmine_Goodie: public Goodie {
public:
    Landmine_Goodie(int level_x, int level_y, StudentWorld * thing);
    virtual void grant_specific_goodie_reward();
private:
};

 #endif // ACTOR_H_

